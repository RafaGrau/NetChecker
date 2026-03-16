#include "pch.h"
#include "HtmlExporter.h"
#include <fstream>
#include <sstream>
#include <ctime>

static std::wstring HtmlColor(ConnectStatus s)
{
    switch (s) {
    case ConnectStatus::OK:      return L"#009000";
    case ConnectStatus::FAILED:  return L"#c80000";
    case ConnectStatus::UNKNOWN: return L"#b47800";
    default:                     return L"#969696";
    }
}

bool HtmlExporter::Export(const wchar_t* path,
                          const std::vector<DestinationResult>& results,
                          const std::wstring& sourceIP)
{
    // Timestamp
    time_t now = time(nullptr);
    struct tm lt {};
    localtime_s(&lt, &now);
    wchar_t ts[64];
    wcsftime(ts, 64, L"%Y-%m-%d %H:%M:%S", &lt);

    std::wofstream f(path);
    if (!f.is_open()) return false;

    f << L"<!DOCTYPE html>\n<html lang=\"es\">\n<head>\n"
      << L"<meta charset=\"UTF-8\">\n"
      << L"<title>NetChecker Report</title>\n"
      << L"<style>\n"
      << L"body{font-family:Segoe UI,Arial,sans-serif;font-size:13px;background:#f5f5f5;color:#222;}\n"
      << L"h1{background:#1a3a5c;color:#fff;padding:12px 20px;margin:0;font-size:17px;font-weight:400;}\n"
      << L"h1 span.src{font-size:13px;color:#c8d8ea;margin-left:12px;}\n"
      << L"p.meta{background:#dde;padding:5px 20px;margin:0;font-size:11px;color:#445;}\n"
      << L"h2{background:#2a5080;color:#fff;padding:6px 16px;margin:16px 0 0;font-size:14px;font-weight:400;}\n"
      << L"table{border-collapse:collapse;width:100%;margin-bottom:16px;table-layout:fixed;}\n"
      << L"col.c-port{width:70px;}\n"
      << L"col.c-proto{width:90px;}\n"
      << L"col.c-desc{width:auto;}\n"
      << L"col.c-status{width:130px;}\n"
      << L"col.c-lat{width:110px;}\n"
      << L"th{background:#1a3a5c;color:#fff;padding:5px 8px;text-align:left;font-weight:400;}\n"
      << L"td{padding:4px 8px;border-bottom:1px solid #ccc;}\n"
      << L"tr:nth-child(even)td{background:#eef1f6;}\n"
      << L".ok{color:#009000;font-weight:600}\n"
      << L".fail{color:#c80000;font-weight:600}\n"
      << L".noresp{color:#4444aa;font-weight:600}\n"
      << L".unk{color:#b47800;font-weight:600}\n"
      << L".pend{color:#969696}\n"
      << L"</style></head><body>\n"
      << L"<h1>&#x1F5CE; NetChecker Connectivity Report"
      << L"<span class=\"src\"> &mdash; IP de origen: "
      << (sourceIP.empty() ? L"(desconocida)" : sourceIP)
      << L"</span></h1>\n"
      << L"<p class=\"meta\">Generado: " << ts << L"</p>\n";

    for (const auto& dr : results)
    {
        f << L"<h2>" << dr.config.name
          << L" &nbsp;<small>(" << dr.config.ip << L" &mdash; "
          << PortDB::TypeName(dr.config.type) << L")</small></h2>\n"
          << L"<table>"
          << L"<colgroup><col class=\"c-port\"><col class=\"c-proto\"><col class=\"c-desc\">"
          << L"<col class=\"c-status\"><col class=\"c-lat\"></colgroup>"
          << L"<tr><th>Puerto</th><th>Protocolo</th>"
          << L"<th>Descripci\x00f3n</th><th>Estado</th><th>Latencia (ms)</th></tr>\n";

        // TCP ports first, then UDP
        for (int pass = 0; pass < 2; ++pass)
        {
            Protocol passProto = (pass == 0) ? Protocol::TCP : Protocol::UDP;
            for (const auto& pr : dr.portResults)
            {
                if (pr.entry.protocol != passProto) continue;

                if (!pr.enabled) continue;  // skip ports not selected for checking
                const wchar_t* cls = L"pend";
                switch (pr.status) {
                case ConnectStatus::OK:          cls = L"ok";     break;
                case ConnectStatus::FAILED:      cls = L"fail";   break;
                case ConnectStatus::NO_RESPONSE: cls = L"noresp"; break;
                case ConnectStatus::UNKNOWN:     cls = L"unk";    break;
                default: break;
                }
                f << L"<tr><td>" << pr.entry.port << L"</td><td>"
                  << StrUtil::ProtoText(pr.entry.protocol) << L"</td><td>"
                  << pr.entry.description
                  << L"</td><td class=\"" << cls << L"\">"
                  << StrUtil::StatusText(pr.status) << L"</td><td>"
                  << (pr.status == ConnectStatus::OK ? std::to_wstring(pr.latencyMs) : L"&mdash;")
                  << L"</td></tr>\n";
            }
        }
        f << L"</table>\n";
    }

    f << L"</body></html>\n";
    return f.good();
}
