#include "stdafx.h"

#include "ApplicationConsole.h"

#include "HAL/PlatformConsole.h"

#include <stdio.h>
#include <fcntl.h>
#include <io.h>

namespace PPE {
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
} //!namespace PPE
