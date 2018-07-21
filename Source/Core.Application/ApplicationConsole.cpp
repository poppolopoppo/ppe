#include "stdafx.h"

#include "ApplicationConsole.h"

#include "Core/HAL/PlatformConsole.h"

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
    FPlatformConsole::Open();
}
//----------------------------------------------------------------------------
void FApplicationConsole::Shutdown() {
    FPlatformConsole::Close();
    FApplicationBase::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
