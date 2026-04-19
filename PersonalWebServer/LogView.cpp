#include "stdafx.h"
#include "LogView.h"

BEGIN_MESSAGE_MAP(CLogView, CWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_EN_VSCROLL(ID_ACCESS_LOG, OnScroll)
    ON_NOTIFY(EN_LINK, ID_ACCESS_LOG, OnLink)
END_MESSAGE_MAP()

CLogView::CLogView() = default;

BOOL CLogView::Create(CWnd* pParentWnd)
{
    return CWnd::Create(nullptr, _T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, 10, 10), pParentWnd, ID_LOG_VIEW);
}

void CLogView::AddLogMessage(const CString& message)
{
    _programmaticScroll = TRUE;
    const int len = _richEditCtrl.GetTextLength();
    _richEditCtrl.SetSel(len, len);
    _richEditCtrl.ReplaceSel(message + "\r\n");

    if (_followLog)
    {
        (void)_richEditCtrl.SendMessage(WM_VSCROLL, SB_BOTTOM, NULL);
    }
    _programmaticScroll = FALSE;
}

void CLogView::ClearLog()
{
    const int len = _richEditCtrl.GetTextLength();
    _richEditCtrl.SetSel(0, len);
    _richEditCtrl.ReplaceSel(_T(""));
}

int CLogView::OnCreate(CREATESTRUCT* lpCreateStruct)
{
    CWnd::OnCreate(lpCreateStruct);

    const CRect rect(0, 0, 0, 0);
    _frame.Create(_T(""), WS_CHILD | WS_VISIBLE | SS_ETCHEDFRAME, rect, this);
    _richEditCtrl.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                         rect, this, ID_ACCESS_LOG);

    _richEditCtrl.SetUndoLimit(0);
    _richEditCtrl.LimitText(0);
    _richEditCtrl.SetEventMask(ENM_SCROLL | ENM_LINK);
    _richEditCtrl.SetAutoURLDetect(TRUE);
    _richEditCtrl.SetFocus();
    
    return 0;
}

void CLogView::OnSize(const UINT nType, const int cx, const int cy)
{
    CWnd::OnSize(nType, cx, cy);

    _frame.MoveWindow(4, 4, cx - 8, cy - 8);
    _richEditCtrl.MoveWindow(8, 8, cx - 16, cy - 16);
}

void CLogView::OnScroll()
{
    if (_programmaticScroll)
    {
        return;
    }

    const int limit = _richEditCtrl.GetScrollLimit(SB_VERT);
    const int scrollY = _richEditCtrl.GetScrollPos(SB_VERT);

    _followLog = scrollY == limit - 1;
}

void CLogView::OnLink(NMHDR* pNMHDR, LRESULT* pResult)
{
    if (const ENLINK* pEnLink = reinterpret_cast<ENLINK*>(pNMHDR); pEnLink->msg == WM_LBUTTONDOWN)
    {
        CString link;
        _richEditCtrl.GetTextRange(pEnLink->chrg.cpMin, pEnLink->chrg.cpMax, link);
        ShellExecute(nullptr, _T("open"), link, nullptr, nullptr, SW_SHOWNORMAL);
    }

    *pResult = 1;
}
