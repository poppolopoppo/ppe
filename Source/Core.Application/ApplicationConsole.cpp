#include "stdafx.h"

#include "ApplicationConsole.h"

#ifdef PLATFORM_WINDOWS
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
FApplicationConsole::FApplicationConsole(const wchar_t *appname)
:   FApplicationBase(appname) {}
//----------------------------------------------------------------------------
FApplicationConsole::~FApplicationConsole() {}
//----------------------------------------------------------------------------
void FApplicationConsole::Start() {
    FApplicationBase::Start();

    RedirectIOToConsole();
}
//----------------------------------------------------------------------------
void FApplicationConsole::Shutdown() {
    FApplicationBase::Shutdown();
}
//----------------------------------------------------------------------------
void FApplicationConsole::RedirectIOToConsole() {

    LOG(Info, L"[Application] RedirectIOToConsole()");

    ::AllocConsole();
    ::SetConsoleOutputCP(65001/* UTF-8 */);

    FILE* fp = nullptr;

    if (0 != ::freopen_s(&fp, "conin$","r", stdin))
        AssertNotReached();

    if (0 != ::freopen_s(&fp, "conout$","w", stdout))
        AssertNotReached();

    if (0 != ::freopen_s(&fp, "conout$","w", stderr))
        AssertNotReached();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
