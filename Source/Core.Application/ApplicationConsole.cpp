#include "stdafx.h"

#include "ApplicationConsole.h"

#ifdef OS_WINDOWS
#   include <windows.h>
#else
#   error "no support"
#endif

#include "Core/Diagnostic/Logger.h"

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ApplicationConsole::ApplicationConsole(const wchar_t *appname)
:   ApplicationBase(appname) {}
//----------------------------------------------------------------------------
ApplicationConsole::~ApplicationConsole() {}
//----------------------------------------------------------------------------
void ApplicationConsole::Start() {
    ApplicationBase::Start();

    RedirectIOToConsole();
}
//----------------------------------------------------------------------------
void ApplicationConsole::Shutdown() {
    ApplicationBase::Shutdown();
}
//----------------------------------------------------------------------------
void ApplicationConsole::RedirectIOToConsole() {

    LOG(Info, L"[Application] RedirectIOToConsole()");

    ::AllocConsole();

    FILE* fp = nullptr;

    fp = freopen("conin$","r",stdin);
    Assert(fp);

    fp = freopen("conout$","w",stdout);
    Assert(fp);

    fp = freopen("conout$","w",stderr);
    Assert(fp);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
