#include "stdafx.h"
#include "App.h"
#include "MainWnd.h"
#include "afxvisualmanagerwindows.h"
#include "HttpServer.h"

BOOL CApp::InitInstance()
{
    AfxOleInit();
    AfxInitRichEdit5();

    SetRegistryKey(L"techwritescode\\PersonalWebServer");
    m_bSaveState = FALSE;
    
    m_pMainWnd = new CMainWnd;
    m_pMainWnd->ShowWindow(m_nCmdShow);
    m_pMainWnd->UpdateWindow();
    
    Server = new CHttpServer();
    
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
    
    return TRUE;
}
