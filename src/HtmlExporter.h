#pragma once
#include "AppTypes.h"
#include <vector>

class HtmlExporter
{
public:
    // Returns true on success
    bool Export(const wchar_t* path,
                const std::vector<DestinationResult>& results,
                const std::wstring& sourceIP);
};
