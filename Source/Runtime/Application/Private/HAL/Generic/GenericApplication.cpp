// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Generic/GenericApplication.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformMessageHandler.h"
#include "HAL/PlatformTime.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATION_API, Application)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGenericApplication::FGenericApplication(FModularDomain& domain, FString&& name)
:   _domain(domain)
,   _name(std::move(name))
,   _services(_name, &_domain.Services()) {
    Assert(not _name.empty());

    _domain.Services().Add<IApplicationService>(this);
}
//----------------------------------------------------------------------------
FGenericApplication::~FGenericApplication() {

    _domain.Services().Remove<IApplicationService>();
}
//----------------------------------------------------------------------------
void FGenericApplication::Start() {
    PPE_SLOG(Application, Emphasis, "start application", {{"application", _name}});

    FPlatformTime::EnterHighResolutionTimer();

    _elapsed = 0.;
}
//----------------------------------------------------------------------------
bool FGenericApplication::PumpMessages() NOEXCEPT {
    return FPlatformMessageHandler::PumpGlobalMessages();
}
//----------------------------------------------------------------------------
void FGenericApplication::Tick(FTimespan dt) {
    _elapsed += dt;
}
//----------------------------------------------------------------------------
void FGenericApplication::ReleaseMemory() NOEXCEPT {
    PPE_SLOG(Application, Emphasis, "release memory in application", {{"application", _name}});

    _services.ReleaseMemory();
}
//----------------------------------------------------------------------------
void FGenericApplication::Shutdown() {
    PPE_SLOG(Application, Emphasis, "shutdown application", {{"application", _name}});

    FPlatformTime::LeaveLowResolutionTimer();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
