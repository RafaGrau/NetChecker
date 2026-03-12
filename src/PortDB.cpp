#include "pch.h"
#include "AppTypes.h"

namespace PortDB
{

// ─── Domain Controller ────────────────────────────────────────────────────────
static const std::vector<PortEntry> s_DC =
{
    {  53,  Protocol::TCP, L"DNS",                          true},
    {  53,  Protocol::UDP, L"DNS (UDP)",                    true},
    {  88,  Protocol::TCP, L"Kerberos",                     true},
    {  88,  Protocol::UDP, L"Kerberos (UDP)",               true},
    { 135,  Protocol::TCP, L"RPC Endpoint Mapper",          true},
    { 139,  Protocol::TCP, L"NetBIOS Session",              true},
    { 389,  Protocol::TCP, L"LDAP",                         true},
    { 389,  Protocol::UDP, L"LDAP (UDP)",                   true},
    { 445,  Protocol::TCP, L"SMB",                          true},
    { 464,  Protocol::TCP, L"Kerberos Pwd Change",          true},
    { 464,  Protocol::UDP, L"Kerberos Pwd Change (UDP)",    true},
    { 636,  Protocol::TCP, L"LDAPS",                        true},
    {3268,  Protocol::TCP, L"Global Catalog",               true},
    {3269,  Protocol::TCP, L"Global Catalog SSL",           true},
    {9389,  Protocol::TCP, L"AD Web Services",              true},
    {5985,  Protocol::TCP, L"WinRM HTTP",                   false},
    {5986,  Protocol::TCP, L"WinRM HTTPS",                  false},
};

// ─── Print Server ─────────────────────────────────────────────────────────────
static const std::vector<PortEntry> s_PS =
{
    {  80,  Protocol::TCP, L"HTTP (IPP/Web UI)",            true},
    { 443,  Protocol::TCP, L"HTTPS",                        true},
    { 445,  Protocol::TCP, L"SMB (Print$)",                 true},
    { 515,  Protocol::TCP, L"LPD",                          true},
    { 631,  Protocol::TCP, L"IPP",                          true},
    {9100,  Protocol::TCP, L"RAW (JetDirect)",              true},
    { 135,  Protocol::TCP, L"RPC",                          false},
};

// ─── SCCM Full (Management Point + Distribution Point + SUP) ─────────────────
static const std::vector<PortEntry> s_SCCM_Full =
{
    {  80,  Protocol::TCP, L"HTTP (MP/DP/SUP)",             true},
    { 443,  Protocol::TCP, L"HTTPS (MP/DP/SUP)",            true},
    { 445,  Protocol::TCP, L"SMB",                          true},
    { 135,  Protocol::TCP, L"RPC Endpoint Mapper",          true},
    {8530,  Protocol::TCP, L"WSUS HTTP",                    true},
    {8531,  Protocol::TCP, L"WSUS HTTPS",                   true},
    {2701,  Protocol::TCP, L"Remote Control",               true},
    {2702,  Protocol::TCP, L"Remote Control (Enc.)",        false},
    {10123, Protocol::TCP, L"Client → MP (alt)",            true},
    {10800, Protocol::UDP, L"Wake On LAN",                  false},
    {9 ,    Protocol::UDP, L"Wake On LAN (port 9)",         false},
};

// ─── SCCM Distribution Point only ────────────────────────────────────────────
static const std::vector<PortEntry> s_SCCM_DP =
{
    {  80,  Protocol::TCP, L"HTTP Content",                 true},
    { 443,  Protocol::TCP, L"HTTPS Content",                true},
    { 445,  Protocol::TCP, L"SMB (Content Share)",          true},
    {8530,  Protocol::TCP, L"WSUS HTTP",                    true},
    {8531,  Protocol::TCP, L"WSUS HTTPS",                   true},
};

// ─── Public API ───────────────────────────────────────────────────────────────
std::vector<PortEntry> GetPorts(DestinationType t)
{
    switch (t)
    {
    case DestinationType::DC:         return s_DC;
    case DestinationType::PrintServer: return s_PS;
    case DestinationType::SCCM_Full:  return s_SCCM_Full;
    case DestinationType::SCCM_DP:    return s_SCCM_DP;
    }
    return {};
}

const wchar_t* TypeName(DestinationType t)
{
    switch (t)
    {
    case DestinationType::DC:          return L"Domain Controller";
    case DestinationType::PrintServer: return L"Print Server";
    case DestinationType::SCCM_Full:   return L"SCCM (Full)";
    case DestinationType::SCCM_DP:     return L"SCCM DP";
    }
    return L"Unknown";
}

const wchar_t* TypeTag(DestinationType t)
{
    switch (t)
    {
    case DestinationType::DC:          return L"DC";
    case DestinationType::PrintServer: return L"PrintServer";
    case DestinationType::SCCM_Full:   return L"SCCM_Full";
    case DestinationType::SCCM_DP:     return L"SCCM_DP";
    }
    return L"Unknown";
}

} // namespace PortDB

namespace StrUtil
{
const wchar_t* StatusText(ConnectStatus s)
{
    switch (s)
    {
    case ConnectStatus::OK:          return L"OK";
    case ConnectStatus::FAILED:      return L"CERRADO";
    case ConnectStatus::NO_RESPONSE: return L"SIN RESPUESTA";
    case ConnectStatus::UNKNOWN:     return L"DESCONOCIDO";
    case ConnectStatus::PENDING:     return L"PENDIENTE";
    case ConnectStatus::DISABLED:    return L"\u2014";
    }
    return L"";
}

const wchar_t* ProtoText(Protocol p)
{
    return (p == Protocol::TCP) ? L"TCP" : L"UDP";
}
} // namespace StrUtil
