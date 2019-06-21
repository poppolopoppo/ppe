#include "stdafx.h"

#include "ApplicationConsole.h"

#include "HAL/PlatformConsole.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationConsole::FApplicationConsole(FWString&& name)
:   FApplicationBase(std::move(name)) {}
//----------------------------------------------------------------------------
FApplicationConsole::~FApplicationConsole() = default;
//----------------------------------------------------------------------------
void FApplicationConsole::Start() {
    FPlatformConsole::Open();
    FApplicationBase::Start();
}
//----------------------------------------------------------------------------
void FApplicationConsole::Shutdown() {
    FApplicationBase::Shutdown();
    FPlatformConsole::Close();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
