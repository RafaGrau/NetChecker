#pragma once
#include "resource.h"

class CNetCheckerApp : public CWinApp
{
public:
    CNetCheckerApp();

    BOOL InitInstance() override;
    int  ExitInstance() override;

    DECLARE_MESSAGE_MAP()

private:
    bool        m_wsaInit       { false };
};

extern CNetCheckerApp theApp;
