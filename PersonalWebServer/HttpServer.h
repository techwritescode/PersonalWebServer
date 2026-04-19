#pragma once
#include "HttpRequest.h"

struct SAcceptContext;
struct SReadContext;
struct SWriteContext;
struct SIoContext;

class CHttpServer
{
public:
    explicit CHttpServer();
    void Start();
    void Stop();
    
    CString Path;

private:
    void HandleRequest(CHttpRequest* req);
    static DWORD ServerThread(LPVOID _this);
    
    void PostAccept();
    void PostRead(CHttpRequest* req);
    void PostSend(CHttpRequest* req, CByteArray* res);
    
    void OnAccept(const SAcceptContext* ctx);
    void OnRead(const SReadContext* ctx, CHttpRequest* req, DWORD bytesRead);
    void OnSend(const SWriteContext* ctx, CHttpRequest* req);
    
    void LogMessage(const CString& message);
    
    void CleanupRequests();
    void CleanupRequest(CHttpRequest* req);

    CArray<CHttpRequest*> _openSockets;
    SOCKET _server;
    HANDLE _iocp;
    UINT _port;
};
