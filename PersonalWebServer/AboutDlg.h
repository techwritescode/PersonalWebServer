#pragma once

class CAboutDlg : public CDialog
{
    DECLARE_DYNAMIC(CAboutDlg)
public:
    CAboutDlg();
    
protected:
    afx_msg void OnLinkClick(LPNMHDR pNMHDR, LRESULT* pResult);
    
    DECLARE_MESSAGE_MAP()
};
