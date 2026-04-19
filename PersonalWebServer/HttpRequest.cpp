#include "stdafx.h"
#include "HttpRequest.h"

CHttpRequest::CHttpRequest(const SOCKET socket): Socket(socket)
{
    ResetTtl();
}

void CHttpRequest::Reset()
{
    Buffer = "";
    Method = "";
    Path = "";
    Headers.RemoveAll();
    ResponseSent = false;
}

void CHttpRequest::IncOp()
{
    InterlockedIncrement(&PendingOps);
}

LONG CHttpRequest::DecOp()
{
    InterlockedDecrement(&PendingOps);
    
    return PendingOps;
}

void CHttpRequest::ResetTtl()
{
    Ttl = CTime::GetTickCount() + CTimeSpan(5); // 5 second timeout
}
