#include "stdafx.h"
#include "HttpResponse.h"

#include "UriCoder.h"

CHttpResponse* CHttpResponse::BuildDirectoryList(const CString& path)
{
    const CString title = PathFindFileName(path);

    const auto response = new CHttpResponse();
    response->Status = 200;
    response->Headers.SetAt("Content-Type", "text/html");
    CStringA body = "<meta charset=\"UTF-8\"><h1>";
    body += title;
    body += "</h1><hr><ul>";

    TCHAR szPath[MAX_PATH] = {};
    (void)PathCchCombine(szPath, MAX_PATH, path, _T("*"));

    CFileFind finder;
    BOOL bWorking = finder.FindFile(szPath);
    while (bWorking)
    {
        bWorking = finder.FindNextFile();
        if (finder.IsDots()) continue;
        

        body += "<li><a href=\"";
        body += CUriCoder::Encode(finder.GetFileName());
        
        if (finder.IsDirectory())
            body += '/';
        
        body += "\">";
        body += finder.GetFileName();
        
        if (finder.IsDirectory())
            body += '/';

        body += "</a></li>";
    }

    body += "</ul>";

    response->Body.SetSize(body.GetLength());
    memcpy_s(response->Body.GetData(), response->Body.GetSize(), body.GetString(), body.GetLength());

    return response;
}

CHttpResponse* CHttpResponse::Build404()
{
    const auto response = new CHttpResponse();
    response->Status = 404;
    response->Headers.SetAt("Content-Type", "text/html");
    const CStringA body = "<meta charset=\"UTF-8\"><h1>404 File Not Found</h1>";
    response->Body.SetSize(body.GetLength());
    memcpy_s(response->Body.GetData(), response->Body.GetSize(), body.GetString(), body.GetLength());
    return response;
}

CByteArray* CHttpResponse::BuildResponse()
{
    BuildHeaders();

    const auto response = new CByteArray();

    CStringA head;
    head.AppendFormat("HTTP/1.1 %d OK\r\n", Status);
    POSITION pos = Headers.GetStartPosition();
    while (pos != nullptr)
    {
        CStringA key;
        CStringA val;
        Headers.GetNextAssoc(pos, key, val);

        head.AppendFormat("%s: %s\r\n", key.GetString(), val.GetString());
    }
    head.Append("\r\n");

    response->SetSize(head.GetLength());
    memcpy_s(response->GetData(), response->GetSize(), head.GetString(), head.GetLength());

    response->Append(Body);
    return response;
}

void CHttpResponse::BuildHeaders()
{
    CStringA length;
    length.Format("%lld", Body.GetSize());
    Headers.SetAt("Content-Length", length);
}
