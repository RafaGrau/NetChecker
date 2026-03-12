#pragma once
#include "AppTypes.h"
#include "resource.h"

// ──────────────────────────────────────────────────────────────────────────────
// CConfigEditorDlg
// Full-featured configuration editor: add/remove servers, edit their port lists.
// Pass an AppConfig by reference; changes are written back only on IDOK.
// ──────────────────────────────────────────────────────────────────────────────
class CConfigEditorDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CConfigEditorDlg)
public:
    explicit CConfigEditorDlg(AppConfig& cfg, CWnd* pParent = nullptr);
    enum { IDD = IDD_CONFIG_EDITOR };

protected:
    void DoDataExchange(CDataExchange* pDX) override;
    BOOL OnInitDialog() override;
    void OnOK() override;

    afx_msg void OnBtnAddServer();
    afx_msg void OnBtnRemServer();
    afx_msg void OnBtnAddPort();
    afx_msg void OnBtnUpdPort();
    afx_msg void OnBtnDelPort();
    afx_msg void OnTabSelChange  (NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnListItemChange(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnListColumnClick(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()

private:
    AppConfig&                     m_cfg;
    std::vector<DestinationConfig> m_servers;   // working copy
    int                            m_curSrv{ -1 };

    // Controls
    CEdit       m_edName;
    CWnd        m_edIP;            // SysIPAddress32
    CComboBox   m_cbType;
    CTabCtrl    m_tab;
    CListCtrl   m_list;
    CEdit       m_edPort;
    CComboBox   m_cbProto;
    CEdit       m_edDesc;

    // ── sort state ────────────────────────────────────────────────────────────
    int  m_sortCol { -1 };   // -1 = unsorted, 0 = Port, 1 = Protocol
    bool m_sortAsc { true };

    // ── helpers ───────────────────────────────────────────────────────────────
    void RefreshTabs();
    void SwitchToServer(int idx);
    void PopulateList(int srvIdx);
    void ListRowToEditor(int row);
    bool EditorToPortEntry(PortEntry& pe);
    void UpdateButtonStates();
    void PositionList();

    std::wstring GetIP() const;
    void         SetIP(const std::wstring& ip);
};
