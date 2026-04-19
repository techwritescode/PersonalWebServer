#include "stdafx.h"
#include "SettingsDlg.h"

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialog)

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
    ON_BN_CLICKED(IDC_SETTINGS_OK, OnBtnOkClicked)
    ON_BN_CLICKED(IDC_SETTINGS_CANCEL, OnCancel)
END_MESSAGE_MAP()

CSettingsDlg::CSettingsDlg(CWnd* pParent) : CDialog(IDD_SETTINGS, pParent)
{
    Port = AfxGetApp()->GetProfileInt(_T("Settings"), _T("Port"), 8080);
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_PORT, _editPort);

    DDX_Text(pDX, IDC_EDIT_PORT, Port);
    DDV_MinMaxUInt(pDX, Port, 0, 65535);
}

void CSettingsDlg::OnBtnOkClicked()
{
    if (UpdateData(TRUE))
    {
        AfxGetApp()->WriteProfileInt(_T("Settings"), _T("Port"), Port);
        OnOK();
    }
}

BOOL CSettingsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    return FALSE;
}
