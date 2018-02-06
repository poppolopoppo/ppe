#include "stdafx.h"

#include "ApplicationConsole.h"

#ifdef PLATFORM_WINDOWS
#   include "Core/Misc/Platform_Windows.h"
#else
#   error "no support"
#endif

#include "Core/Diagnostic/Logger.h"

#include <stdio.h>
#include <fcntl.h>
#include <io.h>

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
}
//----------------------------------------------------------------------------
void FApplicationConsole::Shutdown() {
    FApplicationBase::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
