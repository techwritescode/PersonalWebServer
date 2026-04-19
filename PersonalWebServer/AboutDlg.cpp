#include "stdafx.h"
#include "AboutDlg.h"

IMPLEMENT_DYNAMIC(CAboutDlg, CDialog)

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    ON_NOTIFY(NM_CLICK, ID_GITHUB_LINK, OnLinkClick)
END_MESSAGE_MAP()

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUT)
{
}

void CAboutDlg::OnLinkClick(LPNMHDR pNMHDR, LRESULT* pResult)
{
    const NMLINK* pNMLINK = reinterpret_cast<NMLINK*>(pNMHDR);

    ShellExecute(nullptr, _T("open"), pNMLINK->item.szUrl, nullptr, nullptr, SW_SHOW);
    *pResult = 1;
}
