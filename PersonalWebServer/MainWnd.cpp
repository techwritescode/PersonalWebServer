#include "stdafx.h"
#include "MainWnd.h"

#include "AboutDlg.h"
#include "App.h"
#include "SettingsDlg.h"

BEGIN_MESSAGE_MAP(CMainWnd, CFrameWndEx)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_COMMAND(ID_START_SERVER, OnStartServer)
    ON_COMMAND(ID_STOP_SERVER, OnStopServer)
    ON_COMMAND(ID_CLEAR_LOG, OnClearLog)
    ON_UPDATE_COMMAND_UI_RANGE(ID_START_SERVER, ID_STOP_SERVER, OnUpdateStartServer)
    ON_MESSAGE(WM_LOG_MESSAGE, OnLogMessage)

    ON_COMMAND(ID_FILE_SAVE, OnSave)
    ON_COMMAND(ID_APP_EXIT, OnExit)
    ON_COMMAND(ID_EDIT_COPY, OnCopy)
    ON_COMMAND(ID_EDIT_SELECT_ALL, OnSelectAll)
    ON_COMMAND(ID_MENU_EDIT_SETTINGS, OnSettings)
    ON_COMMAND(ID_MENU_HELP_ABOUT, OnAbout)
END_MESSAGE_MAP()

struct SToolbarItem
{
    UINT Action;
    UINT Icon;
};

static void LoadToolbar(CMFCToolBar& toolBar, const CArray<SToolbarItem>& buttonMap)
{
    CArray<UINT> buttons;
    CArray<INT> icons;
    CMFCToolBarImages* images = CMFCToolBar::GetImages();
    images->SetImageSize(CSize(16, 16));


    for (INT_PTR i = 0; i < buttonMap.GetSize(); i++)
    {
        UINT action = buttonMap[i].Action;
        UINT icon = buttonMap[i].Icon;

        buttons.Add(action);

        if (icon == 0)
        {
            icons.Add(0);
            continue;
        }

        const auto icn = static_cast<HICON>(LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(icon), IMAGE_ICON, 16,
                                                      16,
                                                      LR_DEFAULTCOLOR));
        icons.Add(images->AddIcon(icn));
        DestroyIcon(icn);
    }

    toolBar.SetButtons(buttons.GetData(), static_cast<int>(buttons.GetSize()));

    VERIFY(buttons.GetSize() == icons.GetSize());

    for (INT_PTR i = 0; i < buttons.GetSize(); i++)
    {
        if (icons[i] == 0)
            continue;

        const int nBtnIdx = toolBar.CommandToIndex(buttons[i]);
        toolBar.GetButton(nBtnIdx)->SetImage(icons[i]);
    }
}

CMainWnd::CMainWnd()
{
    Create(NULL, _T("Personal Web Server"), WS_OVERLAPPEDWINDOW | WS_VISIBLE);
}

void CMainWnd::RecalcLayout(BOOL bNotify)
{
    CFrameWndEx::RecalcLayout(bNotify);
    
    if (_logView.GetSafeHwnd())
    {
        CRect rect;
        GetDockingManager()->GetClientAreaBounds(rect);
        
        rect.DeflateRect(4, 4, 4, 4);
        _logView.MoveWindow(rect);
    }
}

int CMainWnd::OnCreate(CREATESTRUCT* lpCreateStruct)
{
    if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
        return -1;
    
    SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), false);
    SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), true);
    LoadAccelTable(MAKEINTRESOURCE(ID_MENU_ACCEL));

    _logView.Create(this);

    _menu.LoadMenu(IDR_MENU_ROOT);
    _menuBar.Create(this);
    _menuBar.SetPaneStyle(_menuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);
    _menuBar.CreateFromMenu(_menu);

    _toolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
    CArray<SToolbarItem> buttonMap;
    buttonMap.Add({.Action = ID_START_SERVER, .Icon = IDR_RUN});
    buttonMap.Add({.Action = ID_STOP_SERVER, .Icon = IDR_STOP});
    buttonMap.Add({.Action = 0, .Icon = 0});
    buttonMap.Add({.Action = ID_CLEAR_LOG, .Icon = IDR_CLEAR});
    LoadToolbar(_toolBar, buttonMap);
    
    EnableDocking(CBRS_ALIGN_ANY);
    _menuBar.EnableDocking(CBRS_ALIGN_TOP);
    _toolBar.EnableDocking(CBRS_ALIGN_TOP);

    DockPane(&_menuBar);
    DockPane(&_toolBar);
    return 0;
}

void CMainWnd::OnStartServer()
{
    if (CFolderPickerDialog dialog; dialog.DoModal() == IDOK)
    {
        _serverRunning = true;
        const CString fileName = dialog.GetPathName();

        const auto server = GetServer();
        server->Path = fileName;
        server->Start();
    }
}

void CMainWnd::OnStopServer()
{
    _serverRunning = false;

    dynamic_cast<CApp*>(AfxGetApp())->Server->Stop();
}

void CMainWnd::OnClearLog()
{
    _logView.ClearLog();
}

LRESULT CMainWnd::OnLogMessage(WPARAM, LPARAM)
{
    const auto app = dynamic_cast<CApp*>(AfxGetApp());
    CSingleLock lock(&app->LogCs, TRUE);
    while (!app->Log.IsEmpty())
    {
        _logView.AddLogMessage(app->Log.RemoveTail());
    }

    return 0L;
}

void CMainWnd::OnUpdateStartServer(CCmdUI* pCmdUi)
{
    switch (pCmdUi->m_nID)
    {
    case ID_START_SERVER:
        pCmdUi->Enable(_serverRunning == false);
        break;
    case ID_STOP_SERVER:
        pCmdUi->Enable(_serverRunning == true);
        break;
    default:
        break;
    }
}

void CMainWnd::OnSize(const UINT nType, const int cx, const int cy)
{
    CFrameWnd::OnSize(nType, cx, cy);
}

#pragma region Menus

void CMainWnd::OnSave()
{
    static auto extensions = _T("Log File (*.log)|*.log|Text File (*.txt)|*.txt");
    if (CFileDialog dialog(FALSE, _T("*.log"), _T("server.log"), OFN_OVERWRITEPROMPT, extensions); dialog.DoModal() == IDOK)
    {
        const CString path = dialog.GetPathName();
        CFile logFile(path, CFile::modeCreate | CFile::modeWrite);
        CString logText;
        _logView.GetCtrl().GetWindowText(logText);

        const CStringA logUtf8(CT2A(logText, CP_UTF8));

        logFile.Write(logUtf8.GetString(), logUtf8.GetLength());
        logFile.Close();
    }
}

void CMainWnd::OnExit()
{
    AfxPostQuitMessage(0);
}

void CMainWnd::OnSelectAll()
{
    _logView.GetCtrl().SetSel(0, -1);
}

void CMainWnd::OnCopy()
{
    _logView.GetCtrl().Copy();
}

void CMainWnd::OnSettings()
{
    CSettingsDlg settingsDlg(this);
    settingsDlg.DoModal();
}

void CMainWnd::OnAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

#pragma endregion


CHttpServer* CMainWnd::GetServer()
{
    return static_cast<CApp*>(AfxGetApp())->Server;
}
