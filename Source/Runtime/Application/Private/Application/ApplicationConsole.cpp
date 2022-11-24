// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Application/ApplicationConsole.h"

#include "HAL/PlatformConsole.h"
#include "HAL/PlatformProcess.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationConsole::FApplicationConsole(FModularDomain& domain, FString&& name)
:   FApplicationBase(domain, std::move(name)) {}
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
void FApplicationConsole::Daemonize() {
	FPlatformProcess::SetPriority(
		FPlatformProcess::CurrentProcess(),
		EProcessPriority::Idle );

    ApplicationLoop();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
