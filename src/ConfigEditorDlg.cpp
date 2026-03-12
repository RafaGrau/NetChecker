#include "pch.h"
#include <algorithm>
#include "ConfigEditorDlg.h"

IMPLEMENT_DYNAMIC(CConfigEditorDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CConfigEditorDlg, CDialogEx)
    ON_BN_CLICKED(IDC_CFG_BTN_ADD_SRV,  &CConfigEditorDlg::OnBtnAddServer)
    ON_BN_CLICKED(IDC_CFG_BTN_REM_SRV,  &CConfigEditorDlg::OnBtnRemServer)
    ON_BN_CLICKED(IDC_CFG_BTN_ADD_PORT, &CConfigEditorDlg::OnBtnAddPort)
    ON_BN_CLICKED(IDC_CFG_BTN_UPD_PORT, &CConfigEditorDlg::OnBtnUpdPort)
    ON_BN_CLICKED(IDC_CFG_BTN_DEL_PORT, &CConfigEditorDlg::OnBtnDelPort)
    ON_NOTIFY(TCN_SELCHANGE,   IDC_CFG_TAB,  &CConfigEditorDlg::OnTabSelChange)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_CFG_LIST, &CConfigEditorDlg::OnListItemChange)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_CFG_LIST, &CConfigEditorDlg::OnListColumnClick)
END_MESSAGE_MAP()

// ──────────────────────────────────────────────────────────────────────────────
CConfigEditorDlg::CConfigEditorDlg(AppConfig& cfg, CWnd* pParent)
    : CDialogEx(IDD_CONFIG_EDITOR, pParent)
    , m_cfg(cfg)
{
    m_servers = cfg.destinations;   // work on a copy
}

void CConfigEditorDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CFG_EDIT_NAME,    m_edName);
    DDX_Control(pDX, IDC_CFG_EDIT_IP,      m_edIP);
    DDX_Control(pDX, IDC_CFG_COMBO_TYPE,   m_cbType);
    DDX_Control(pDX, IDC_CFG_TAB,          m_tab);
    DDX_Control(pDX, IDC_CFG_LIST,         m_list);
    DDX_Control(pDX, IDC_CFG_EDIT_PORT,    m_edPort);
    DDX_Control(pDX, IDC_CFG_COMBO_PROTO,  m_cbProto);
    DDX_Control(pDX, IDC_CFG_EDIT_DESC,    m_edDesc);
}

// ──────────────────────────────────────────────────────────────────────────────
BOOL CConfigEditorDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Server type combo
    m_cbType.AddString(PortDB::TypeName(DestinationType::DC));
    m_cbType.AddString(PortDB::TypeName(DestinationType::PrintServer));
    m_cbType.AddString(PortDB::TypeName(DestinationType::SCCM_Full));
    m_cbType.AddString(PortDB::TypeName(DestinationType::SCCM_DP));
    m_cbType.SetCurSel(0);

    // Port-editor protocol combo
    m_cbProto.AddString(L"TCP");
    m_cbProto.AddString(L"UDP");
    m_cbProto.SetCurSel(0);

    // List columns
    m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
    m_list.InsertColumn(0, L"Puerto",       LVCFMT_RIGHT, 60);
    m_list.InsertColumn(1, L"Protocolo",    LVCFMT_LEFT,  75);
    m_list.InsertColumn(2, L"Descripci\xf3n", LVCFMT_LEFT, 330);

    // Build tabs from existing servers in the working copy
    RefreshTabs();
    if (!m_servers.empty())
        SwitchToServer(0);

    UpdateButtonStates();
    return TRUE;
}

// ── IP address control helpers ─────────────────────────────────────────────────
std::wstring CConfigEditorDlg::GetIP() const
{
    DWORD addr = 0;
    const_cast<CWnd&>(m_edIP).SendMessage(IPM_GETADDRESS, 0, (LPARAM)&addr);
    if (addr == 0) return L"";
    wchar_t buf[20] = {};
    swprintf_s(buf, L"%u.%u.%u.%u",
        FIRST_IPADDRESS(addr),  SECOND_IPADDRESS(addr),
        THIRD_IPADDRESS(addr),  FOURTH_IPADDRESS(addr));
    return buf;
}

void CConfigEditorDlg::SetIP(const std::wstring& ip)
{
    unsigned a = 0, b = 0, c = 0, d = 0;
    if (swscanf_s(ip.c_str(), L"%u.%u.%u.%u", &a, &b, &c, &d) == 4)
        m_edIP.SendMessage(IPM_SETADDRESS, 0, MAKEIPADDRESS(a, b, c, d));
}

// ── Tab management ─────────────────────────────────────────────────────────────
void CConfigEditorDlg::RefreshTabs()
{
    m_tab.DeleteAllItems();
    for (int i = 0; i < (int)m_servers.size(); ++i)
    {
        TCITEM ti{};
        ti.mask    = TCIF_TEXT;
        CString lbl(m_servers[i].name.c_str());
        ti.pszText = lbl.GetBuffer();
        m_tab.InsertItem(i, &ti);
        lbl.ReleaseBuffer();
    }
    PositionList();
}

void CConfigEditorDlg::SwitchToServer(int idx)
{
    m_curSrv = idx;
    m_sortCol = -1;
    m_sortAsc = true;
    // Clear header sort arrows
    if (m_list.GetSafeHwnd())
    {
        HDITEM hdi{};
        hdi.mask = HDI_FORMAT;
        CHeaderCtrl* pH = m_list.GetHeaderCtrl();
        if (pH) for (int i = 0; i < 2; ++i)
        {
            pH->GetItem(i, &hdi);
            hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
            pH->SetItem(i, &hdi);
        }
    }
    if (idx >= 0 && idx < (int)m_servers.size())
    {
        m_tab.SetCurSel(idx);
        PopulateList(idx);
    }
    else
    {
        m_list.DeleteAllItems();
    }
    // Clear port editor
    m_edPort.SetWindowText(L"");
    m_edDesc.SetWindowText(L"");
    m_cbProto.SetCurSel(0);
    UpdateButtonStates();
}

// ── Port list population ───────────────────────────────────────────────────────
void CConfigEditorDlg::PopulateList(int srvIdx)
{
    m_list.DeleteAllItems();
    if (srvIdx < 0 || srvIdx >= (int)m_servers.size()) return;

    const auto& ports = m_servers[srvIdx].ports;
    for (int i = 0; i < (int)ports.size(); ++i)
    {
        const auto& pe = ports[i];
        m_list.InsertItem(i, std::to_wstring(pe.port).c_str());
        m_list.SetItemText(i, 1, pe.protocol == Protocol::TCP ? L"TCP" : L"UDP");
        m_list.SetItemText(i, 2, pe.description.c_str());
    }
}

void CConfigEditorDlg::ListRowToEditor(int row)
{
    if (row < 0) return;
    m_edPort.SetWindowText(m_list.GetItemText(row, 0));
    CString proto = m_list.GetItemText(row, 1);
    m_cbProto.SelectString(-1, proto);
    m_edDesc.SetWindowText(m_list.GetItemText(row, 2));
}

bool CConfigEditorDlg::EditorToPortEntry(PortEntry& pe)
{
    CString portStr;
    m_edPort.GetWindowText(portStr);
    portStr.Trim();
    if (portStr.IsEmpty())
    {
        MessageBox(L"Introduzca un n\xfamero de puerto.", L"Campo requerido", MB_ICONWARNING);
        m_edPort.SetFocus();
        return false;
    }
    int pnum = _wtoi(portStr);
    if (pnum < 1 || pnum > 65535)
    {
        MessageBox(L"El puerto debe estar entre 1 y 65535.", L"Puerto inv\xe1lido", MB_ICONWARNING);
        m_edPort.SetFocus();
        return false;
    }
    pe.port     = pnum;
    pe.protocol = (m_cbProto.GetCurSel() == 0) ? Protocol::TCP : Protocol::UDP;
    CString desc;
    m_edDesc.GetWindowText(desc);
    pe.description = desc.GetString();
    pe.enabled     = true;
    return true;
}

// ── Button handlers ────────────────────────────────────────────────────────────
void CConfigEditorDlg::OnBtnAddServer()
{
    CString name;
    m_edName.GetWindowText(name);
    name.Trim();
    if (name.IsEmpty())
    {
        MessageBox(L"Introduzca un nombre de servidor.", L"Campo requerido", MB_ICONWARNING);
        m_edName.SetFocus();
        return;
    }

    DWORD ipVal = 0;
    m_edIP.SendMessage(IPM_GETADDRESS, 0, (LPARAM)&ipVal);
    if (ipVal == 0)
    {
        MessageBox(L"Introduzca una direcci\xf3n IP v\xe1lida.", L"Campo requerido", MB_ICONWARNING);
        m_edIP.SetFocus();
        return;
    }

    static const DestinationType kTypes[] =
    {
        DestinationType::DC,
        DestinationType::PrintServer,
        DestinationType::SCCM_Full,
        DestinationType::SCCM_DP
    };
    int typeIdx = m_cbType.GetCurSel();
    if (typeIdx < 0) typeIdx = 0;
    DestinationType dt = kTypes[typeIdx];

    DestinationConfig dc;
    dc.name  = name.GetString();
    dc.ip    = GetIP();
    dc.type  = dt;
    dc.ports = PortDB::GetPorts(dt);    // default ports for this server type
    for (auto& pe : dc.ports) pe.enabled = true;

    m_servers.push_back(std::move(dc));
    int newIdx = (int)m_servers.size() - 1;

    // Add tab
    TCITEM ti{};
    ti.mask    = TCIF_TEXT;
    CString lbl(name);
    ti.pszText = lbl.GetBuffer();
    m_tab.InsertItem(newIdx, &ti);
    lbl.ReleaseBuffer();

    // Clear input form
    m_edName.SetWindowText(L"");
    m_edIP.SendMessage(IPM_CLEARADDRESS, 0, 0);
    m_cbType.SetCurSel(0);

    PositionList();
    SwitchToServer(newIdx);
}

void CConfigEditorDlg::OnBtnRemServer()
{
    if (m_curSrv < 0 || m_curSrv >= (int)m_servers.size()) return;

    CString msg;
    msg.Format(L"\xbfEliminar el servidor '%s'?", m_servers[m_curSrv].name.c_str());
    if (MessageBox(msg, L"Confirmar", MB_YESNO | MB_ICONQUESTION) != IDYES) return;

    m_servers.erase(m_servers.begin() + m_curSrv);
    m_tab.DeleteItem(m_curSrv);

    int newSel = min(m_curSrv, (int)m_servers.size() - 1);
    m_curSrv = -1;
    if (newSel >= 0)
        SwitchToServer(newSel);
    else
    {
        m_list.DeleteAllItems();
        UpdateButtonStates();
    }
}

void CConfigEditorDlg::OnBtnAddPort()
{
    if (m_curSrv < 0) return;
    PortEntry pe;
    if (!EditorToPortEntry(pe)) return;

    m_servers[m_curSrv].ports.push_back(pe);

    int row = m_list.GetItemCount();
    m_list.InsertItem(row, std::to_wstring(pe.port).c_str());
    m_list.SetItemText(row, 1, pe.protocol == Protocol::TCP ? L"TCP" : L"UDP");
    m_list.SetItemText(row, 2, pe.description.c_str());

    // Clear editor fields for next entry
    m_edPort.SetWindowText(L"");
    m_edDesc.SetWindowText(L"");
    m_cbProto.SetCurSel(0);
    m_edPort.SetFocus();
}

void CConfigEditorDlg::OnBtnUpdPort()
{
    int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
    if (sel < 0 || m_curSrv < 0) return;

    PortEntry pe;
    if (!EditorToPortEntry(pe)) return;

    if (sel < (int)m_servers[m_curSrv].ports.size())
        m_servers[m_curSrv].ports[sel] = pe;

    m_list.SetItemText(sel, 0, std::to_wstring(pe.port).c_str());
    m_list.SetItemText(sel, 1, pe.protocol == Protocol::TCP ? L"TCP" : L"UDP");
    m_list.SetItemText(sel, 2, pe.description.c_str());
}

void CConfigEditorDlg::OnBtnDelPort()
{
    if (m_curSrv < 0) return;
    int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
    if (sel < 0) return;

    if (sel < (int)m_servers[m_curSrv].ports.size())
        m_servers[m_curSrv].ports.erase(m_servers[m_curSrv].ports.begin() + sel);
    m_list.DeleteItem(sel);

    int count = m_list.GetItemCount();
    if (count > 0)
    {
        int next = min(sel, count - 1);
        m_list.SetItemState(next, LVIS_SELECTED | LVIS_FOCUSED,
                                  LVIS_SELECTED | LVIS_FOCUSED);
        ListRowToEditor(next);
    }
    else
    {
        m_edPort.SetWindowText(L"");
        m_edDesc.SetWindowText(L"");
        m_cbProto.SetCurSel(0);
    }
    UpdateButtonStates();
}

void CConfigEditorDlg::OnListColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
    auto* pNM = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
    int col = pNM->iSubItem;   // 0 = Port, 1 = Protocol (only those two are sortable)
    if (col != 0 && col != 1) { *pResult = 0; return; }

    if (m_sortCol == col)
        m_sortAsc = !m_sortAsc;         // same column → reverse
    else
    {
        m_sortCol = col;
        m_sortAsc = true;
    }

    if (m_curSrv < 0 || m_curSrv >= (int)m_servers.size()) { *pResult = 0; return; }

    auto& ports = m_servers[m_curSrv].ports;

    if (col == 0)   // sort by Port number
    {
        std::stable_sort(ports.begin(), ports.end(),
            [asc = m_sortAsc](const PortEntry& a, const PortEntry& b)
            { return asc ? a.port < b.port : a.port > b.port; });
    }
    else            // sort by Protocol (TCP < UDP), then by port within group
    {
        std::stable_sort(ports.begin(), ports.end(),
            [asc = m_sortAsc](const PortEntry& a, const PortEntry& b)
            {
                if (a.protocol != b.protocol)
                    return asc ? (a.protocol < b.protocol)
                               : (a.protocol > b.protocol);
                return a.port < b.port;   // secondary: port asc always
            });
    }

    // Update column-header arrow indicator
    HDITEM hdi{};
    hdi.mask = HDI_FORMAT;
    CHeaderCtrl* pH = m_list.GetHeaderCtrl();
    for (int i = 0; i < 2; ++i)
    {
        pH->GetItem(i, &hdi);
        hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
        if (i == col)
            hdi.fmt |= (m_sortAsc ? HDF_SORTUP : HDF_SORTDOWN);
        pH->SetItem(i, &hdi);
    }

    PopulateList(m_curSrv);

    // Clear editor fields after re-sort
    m_edPort.SetWindowText(L"");
    m_edDesc.SetWindowText(L"");
    m_cbProto.SetCurSel(0);
    UpdateButtonStates();

    *pResult = 0;
}

void CConfigEditorDlg::OnTabSelChange(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    SwitchToServer(m_tab.GetCurSel());
    *pResult = 0;
}

void CConfigEditorDlg::OnListItemChange(NMHDR* pNMHDR, LRESULT* pResult)
{
    auto* pNM = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
    if ((pNM->uNewState & LVIS_SELECTED) && !(pNM->uOldState & LVIS_SELECTED))
        ListRowToEditor(pNM->iItem);
    UpdateButtonStates();
    *pResult = 0;
}

// ── Layout helpers ─────────────────────────────────────────────────────────────
void CConfigEditorDlg::PositionList()
{
    if (!m_tab.GetSafeHwnd() || !m_list.GetSafeHwnd()) return;

    CRect rc;
    m_tab.GetWindowRect(&rc);
    ScreenToClient(&rc);
    m_tab.AdjustRect(FALSE, &rc);   // body area of the tab control
    rc.DeflateRect(2, 2);
    m_list.MoveWindow(&rc);
}

void CConfigEditorDlg::UpdateButtonStates()
{
    bool hasSrv = (m_curSrv >= 0 && m_curSrv < (int)m_servers.size());
    bool hasSel = hasSrv && (m_list.GetNextItem(-1, LVNI_SELECTED) >= 0);

    auto enable = [this](int id, bool on) {
        if (auto* p = GetDlgItem(id)) p->EnableWindow(on);
    };

    enable(IDC_CFG_BTN_REM_SRV,  hasSrv);
    enable(IDC_CFG_EDIT_PORT,    hasSrv);
    enable(IDC_CFG_COMBO_PROTO,  hasSrv);
    enable(IDC_CFG_EDIT_DESC,    hasSrv);
    enable(IDC_CFG_BTN_ADD_PORT, hasSrv);
    enable(IDC_CFG_BTN_UPD_PORT, hasSel);
    enable(IDC_CFG_BTN_DEL_PORT, hasSel);
}

// ── OK / Cancel ────────────────────────────────────────────────────────────────
void CConfigEditorDlg::OnOK()
{
    if (m_servers.empty())
    {
        MessageBox(L"A\xf1""ada al menos un servidor antes de guardar.",
                   L"Configuraci\xf3n vac\xeda", MB_ICONWARNING);
        return;
    }
    m_cfg.destinations = m_servers;
    CDialogEx::OnOK();
}
