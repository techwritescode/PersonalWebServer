#pragma once

#include <afxdockablepane.h>

class CLogView : public CWnd
{
public:
    CLogView();
    using CWnd::Create;
    virtual BOOL Create(CWnd* pParentWnd);

    void AddLogMessage(const CString& message);
    void ClearLog();
    
    CRichEditCtrl& GetCtrl()
    {
        return _richEditCtrl;
    }

protected:
    afx_msg int OnCreate(CREATESTRUCT* lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnScroll();
    afx_msg void OnLink(NMHDR* pNMHDR, LRESULT* pResult);
    
    
    DECLARE_MESSAGE_MAP()
protected:
    CStatic _frame;
    CRichEditCtrl _richEditCtrl;
    
    BOOL _followLog = TRUE;
    BOOL _programmaticScroll = FALSE;
};
