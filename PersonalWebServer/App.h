#pragma once
#include "HttpServer.h"

class CHttpServer;

class CApp : public CWinAppEx
{
public:
    BOOL InitInstance() override;

    CCriticalSection LogCs;
    CList<CString> Log;
    
    CHttpServer* Server;
};