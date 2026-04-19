#pragma once
#include "LogView.h"

class CHttpServer;

class CCustomToolBar : public CMFCToolBar
{
public:
    BOOL CanFloat() const override
    {
        return FALSE;
    }
    CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz) override
    {
        CSize size = CMFCToolBar::CalcFixedLayout(bStretch, bHorz);
        
        if (bHorz && GetParent())
        {
            CRect parentRect;
            GetParent()->GetClientRect(parentRect);
            size.cx = parentRect.Width();
        }
        
        return size;
    }
};

class CCustomMenuBar : public CMFCMenuBar
{
public:
    BOOL CanFloat() const override
    {
        return FALSE;
    }
};

class CMainWnd : public CFrameWndEx
{
public:
    CMainWnd();
    
    void RecalcLayout(BOOL bNotify) override;
    
protected:
    afx_msg int OnCreate(CREATESTRUCT* lpCreateStruct);
    afx_msg void OnStartServer();
    afx_msg void OnStopServer();
    afx_msg void OnClearLog();
    afx_msg LRESULT OnLogMessage(WPARAM, LPARAM);
    afx_msg void OnUpdateStartServer(CCmdUI* pCmdUi);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    
    afx_msg void OnSave();
    afx_msg void OnExit();
    afx_msg void OnSelectAll();
    afx_msg void OnCopy();
    afx_msg void OnSettings();
    afx_msg void OnAbout();
    
    
    CHttpServer* GetServer();
    
    DECLARE_MESSAGE_MAP()
protected:
    CMenu _menu;
    CStatic _label;
    CFont _font;
    CImageList _images;

    CLogView _logView;
    CCustomToolBar _toolBar;
    CCustomMenuBar _menuBar;
    
private:
    bool _serverRunning = false;
};
