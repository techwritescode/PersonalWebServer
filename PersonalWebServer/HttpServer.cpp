#include "stdafx.h"
#include "HttpServer.h"
#include "App.h"
#include "HttpParser.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "MimeTypes.h"
#include "UriCoder.h"
#include "Url.h"

enum class EIoType: UCHAR
{
    ACCEPT,
    RECV,
    SEND
};

struct SIoContext
{
    OVERLAPPED Ol{};
    EIoType Type;
};

struct SReadContext
{
    SIoContext Io;
    WSABUF Buffer;
    char Data[4096];
};

struct SWriteContext
{
    SIoContext Io;
    CByteArray* ByteArray;
    WSABUF Buffer;
};

struct SAcceptContext
{
    SIoContext Io;
    SOCKET AcceptSocket;
    char Buffer[(sizeof(sockaddr_in) + 16) * 2];
};

static CHttpResponse* SendFile(const CString& path)
{
    const CString ext = PathFindExtension(path);
    UINT flags = CFile::modeRead;

    BOOL isBinary;
    const CString mime = MimeTypes.Lookup(ext, &isBinary);

    if (isBinary)
    {
        flags |= CFile::typeBinary;
    }

    const auto res = new CHttpResponse();

    CFile file(path, flags);
    const auto size = file.GetLength();
    res->Body.SetSize(static_cast<INT_PTR>(size));
    file.Read(res->Body.GetData(), static_cast<UINT>(size));
    file.Close();

    if (mime != "")
    {
        res->Headers.SetAt("Content-Type", CStringA(mime));
    }
    return res;
}

CHttpServer::CHttpServer() : _server(0), _iocp(nullptr), _port(0)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void CHttpServer::HandleRequest(CHttpRequest* req)
{
    CUrl url(req->Path);
    const CString resolved = CUriCoder::Canonicalize(CString(url.GetPath()));

    CString fileName;
    {
        TCHAR path[MAX_PATH] = {};
        CStringArray parts;

        int pos = 0;
        CString token = resolved.Tokenize(L"/", pos);
        while (token != "")
        {
            parts.Add(token);
            (void)PathCchCombine(path, MAX_PATH, path, token);
            token = resolved.Tokenize(L"/", pos);
        }
        fileName = path;
    }
    CString resolvedFile;
    (void)PathCchCombine(resolvedFile.GetBuffer(MAX_PATH), MAX_PATH, Path, fileName);
    resolvedFile.ReleaseBuffer();

    CHttpResponse* response;

    if (PathFileExists(resolvedFile))
    {
        if (PathIsDirectory(resolvedFile))
        {
            CString indexFile;
            PathCchCombine(indexFile.GetBuffer(MAX_PATH), MAX_PATH, resolvedFile, L"index.html");

            if (PathFileExists(indexFile))
            {
                response = SendFile(indexFile);
            }
            else
            {
                response = CHttpResponse::BuildDirectoryList(resolvedFile);
            }
        }
        else
        {
            response = SendFile(resolvedFile);
        }
    }
    else
    {
        response = CHttpResponse::Build404();
    }

    CByteArray* buffer = response->BuildResponse();
    delete response;

    PostSend(req, buffer);
}

DWORD CHttpServer::ServerThread(const LPVOID _this)
{
    const auto server = static_cast<CHttpServer*>(_this);
    server->_port = AfxGetApp()->GetProfileInt(_T("Settings"), _T("Port"), 8080);

    LPFN_ACCEPTEX pfnAcceptEx;
    GUID acceptExGuid = WSAID_ACCEPTEX;

    DWORD bytes = 0;

    server->_server = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    ASSERT(server->_server != INVALID_SOCKET);

    sockaddr_in service = {};
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = ADDR_ANY;
    service.sin_port = htons(static_cast<USHORT>(server->_port));

    ASSERT(bind(server->_server, reinterpret_cast<SOCKADDR*>(&service), sizeof(service)) == 0);
    ASSERT(listen(server->_server, SOMAXCONN) == 0);

    WSAIoctl(server->_server, SIO_GET_EXTENSION_FUNCTION_POINTER, &acceptExGuid, sizeof(acceptExGuid), &pfnAcceptEx,
             sizeof(pfnAcceptEx), &bytes, nullptr, nullptr);


    server->_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(server->_server), server->_iocp, 0, 0);

    CString startupMsg;
    startupMsg.Format(_T("Listening on port http://localhost:%d/"), server->_port);
    server->LogMessage(startupMsg);

    for (int i = 0; i < 16; i++)
    {
        server->PostAccept();
    }

    while (true)
    {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey;
        OVERLAPPED* ol;

        BOOL ok = GetQueuedCompletionStatus(server->_iocp, &bytesTransferred, &completionKey, &ol, 1000);

        if (completionKey == 0xDEADBEEF)
        {
            break;
        }

        auto* ctx = CONTAINING_RECORD(ol, SIoContext, Ol);

        if (ok)
        {
            if (ctx->Type == EIoType::ACCEPT)
            {
                const auto acceptCtx = reinterpret_cast<SAcceptContext*>(ctx);
                server->OnAccept(acceptCtx);
                delete acceptCtx;
            }
            else if (ctx->Type == EIoType::RECV)
            {
                const auto readCtx = reinterpret_cast<SReadContext*>(ctx);
                const auto req = reinterpret_cast<CHttpRequest*>(completionKey);
                server->OnRead(readCtx, req, bytesTransferred);
                delete readCtx;
            }
            else if (ctx->Type == EIoType::SEND)
            {
                const auto writeCtx = reinterpret_cast<SWriteContext*>(ctx);
                const auto req = reinterpret_cast<CHttpRequest*>(completionKey);
                server->OnSend(writeCtx, req);
                delete writeCtx;
            }
        }
        else
        {
            if (ol == nullptr)
            {
                server->CleanupRequests();
            }
            else
            {
                const auto req = reinterpret_cast<CHttpRequest*>(completionKey);
                req->DecOp();

                switch (ctx->Type)
                {
                case EIoType::ACCEPT:
                    delete reinterpret_cast<SAcceptContext*>(ctx);
                    server->PostAccept();
                    break;
                case EIoType::RECV:
                    delete reinterpret_cast<SReadContext*>(ctx);
                    break;
                case EIoType::SEND:
                    const auto writeCtx = reinterpret_cast<SWriteContext*>(ctx);
                    delete writeCtx->Buffer.buf;
                    delete writeCtx;
                    break;
                }
            }
        }
    }


    for (INT_PTR i = 0; i < server->_openSockets.GetSize(); i++)
    {
        server->CleanupRequest(server->_openSockets[i]);
    }
    server->_openSockets.RemoveAll();

    closesocket(server->_server);
    CloseHandle(server->_iocp);

    TRACE("Server Stopped\r\n");

    return 0;
}

void CHttpServer::Start()
{
    LogMessage("Starting server...");
    CreateThread(nullptr, 0, ServerThread, this, 0, nullptr);
}

void CHttpServer::Stop()
{
    LogMessage("Stopping server...");
    PostQueuedCompletionStatus(_iocp, sizeof(int), 0xDEADBEEF, nullptr);
}

void CHttpServer::PostAccept()
{
    const auto ctx = new SAcceptContext();
    ctx->Io.Type = EIoType::ACCEPT;
    ctx->AcceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

    DWORD bytesReceived = 0;

    const BOOL ok = AcceptEx(_server, ctx->AcceptSocket, ctx->Buffer, 0, sizeof(sockaddr_in) + 16,
                             sizeof(sockaddr_in) + 16,
                             &bytesReceived, &ctx->Io.Ol);

    if (!ok && WSAGetLastError() != ERROR_IO_PENDING)
    {
        TRACE("WSA ACCEPT Error: %x\r\n", WSAGetLastError());
        closesocket(ctx->AcceptSocket);
        delete ctx;
    }
}

void CHttpServer::PostRead(CHttpRequest* req)
{
    req->IncOp();
    const auto ctx = new SReadContext();
    ctx->Io.Type = EIoType::RECV;
    ctx->Buffer.buf = ctx->Data;
    ctx->Buffer.len = sizeof(ctx->Data);

    DWORD flags = 0;

    if (const int rc = WSARecv(req->Socket, &ctx->Buffer, 1, nullptr, &flags, &ctx->Io.Ol, nullptr); rc == SOCKET_ERROR
        &&
        WSAGetLastError() != ERROR_IO_PENDING)
    {
        TRACE("WSA READ Error: %x\r\n", WSAGetLastError());
        req->DecOp();
        CleanupRequest(req);
        delete ctx;
    }
}

void CHttpServer::PostSend(CHttpRequest* req, CByteArray* res)
{
    req->IncOp();
    const auto ctx = new SWriteContext();
    ctx->Io.Type = EIoType::SEND;
    ctx->ByteArray = res;
    ctx->Buffer.buf = reinterpret_cast<CHAR*>(ctx->ByteArray->GetData());
    ctx->Buffer.len = static_cast<ULONG>(ctx->ByteArray->GetCount());

    if (const int rc = WSASend(req->Socket, &ctx->Buffer, 1, nullptr, 0, &ctx->Io.Ol, nullptr); rc == SOCKET_ERROR &&
        WSAGetLastError() != ERROR_IO_PENDING)
    {
        TRACE("WSA WRITE Error: %x\r\n", WSAGetLastError());
        req->DecOp();
        CleanupRequest(req);
        delete res;
        delete ctx;
    }
}

void CHttpServer::OnAccept(const SAcceptContext* ctx)
{
    TRACE("Accepting Connection\r\n");
    auto req = new CHttpRequest(ctx->AcceptSocket);
    _openSockets.Add(req);

    setsockopt(ctx->AcceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char*>(&_server),
               sizeof(_server));
    BOOL opt = TRUE;
    setsockopt(ctx->AcceptSocket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&opt), sizeof(opt));

    CreateIoCompletionPort(reinterpret_cast<HANDLE>(ctx->AcceptSocket), _iocp, reinterpret_cast<ULONG_PTR>(req), 0);
    PostRead(req);
    PostAccept();
}

void CHttpServer::OnRead(const SReadContext* ctx, CHttpRequest* req, const DWORD bytesRead)
{
    req->DecOp();
    if (bytesRead == 0)
    {
        return;
    }

    req->ResetTtl();

    req->Buffer += CStringA(ctx->Buffer.buf, static_cast<int>(bytesRead));
    CHttpParser::ParseRequest(req->Buffer, req);
    HandleRequest(req);

    CString logMessage;
    const CString time = CTime::GetCurrentTime().Format("%F %T");
    logMessage.Format(_T("[%s] %hs %s"), time.GetString(), req->Method.GetString(),
                      CUriCoder::Decode(req->Path.GetString()).GetString());
    LogMessage(logMessage);

    req->Reset();
}

void CHttpServer::OnSend(const SWriteContext* ctx, CHttpRequest* req)
{
    req->DecOp();
    req->ResetTtl();

    delete ctx->ByteArray;
    req->ResponseSent = true;

    if (req->State == EHttpRequestState::ACTIVE)
    {
        PostRead(req);
    }
}

void CHttpServer::LogMessage(const CString& message)
{
    const auto app = dynamic_cast<CApp*>(AfxGetApp());
    {
        CSingleLock lock(&app->LogCs, TRUE);
        app->Log.AddHead(message);
    }
    app->m_pMainWnd->PostMessage(WM_LOG_MESSAGE, 0, 0);
}

void CHttpServer::CleanupRequests()
{
    const CTime now = CTime::GetCurrentTime();

    for (auto i = _openSockets.GetCount() - 1; i >= 0; i--)
    {
        if (const auto req = _openSockets[i]; req->State == EHttpRequestState::ACTIVE)
        {
            if (req->Ttl < now)
            {
                TRACE("Socket Timeout: %d, Starting Shutdown\r\n", req->Socket);
                CleanupRequest(_openSockets[i]);
            }
        }
        else if (req->State == EHttpRequestState::CLOSING && req->PendingOps == 0)
        {
            TRACE("Stale Socket Remove: %d %d\r\n", req->Socket, req->State);
            delete req;
            _openSockets.RemoveAt(i);
        }
        else
        {
            TRACE("ZOMBIE %d %d\r\n", req->Socket, req->State);
            ASSERT(FALSE); // This should never get hit
        }
    }
}

void CHttpServer::CleanupRequest(CHttpRequest* req)
{
    TRACE("Socket: %d, Pending Ops: %d, State: %d\r\n", req->Socket, req->PendingOps, req->State);

    if (req->State == EHttpRequestState::ACTIVE)
    {
        req->State = EHttpRequestState::CLOSING;
        CancelIoEx(reinterpret_cast<HANDLE>(req->Socket), nullptr);
        shutdown(req->Socket, SD_BOTH);
        closesocket(req->Socket);
    }
}
