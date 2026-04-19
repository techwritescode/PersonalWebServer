#pragma once

enum class EHttpRequestState
{
    ACTIVE,
    CLOSING
};

class CHttpRequest
{
public:
    explicit CHttpRequest(SOCKET socket);
    void Reset();
    
    void IncOp();
    LONG DecOp();
    
    void ResetTtl();

    CStringA Buffer;
    
    SOCKET Socket;
    CStringA Method;
    CStringA Path;
    CMap<CStringA, LPCSTR, CStringA, LPCSTR> Headers;
    
    CTime Ttl;
    EHttpRequestState State;
    LONG PendingOps;

    bool ResponseSent;
};
