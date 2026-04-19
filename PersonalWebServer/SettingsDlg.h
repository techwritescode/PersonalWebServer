#pragma once

class CSettingsDlg: public CDialog
{
    DECLARE_DYNAMIC(CSettingsDlg)

public:
    explicit CSettingsDlg(CWnd* pParent = nullptr);
    BOOL OnInitDialog() override;

    int Port;
protected:
    void DoDataExchange(CDataExchange* pDX) override;
    afx_msg void OnBtnOkClicked();
    DECLARE_MESSAGE_MAP()
protected:
    CEdit _editPort;
};
