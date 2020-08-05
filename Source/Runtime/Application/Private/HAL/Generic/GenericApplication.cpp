#include "stdafx.h"

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
FGenericApplication::FGenericApplication(const FModularDomain& domain, FWString&& name)
:    _domain(domain)
,    _name(std::move(name))
,    _services(&_domain.Services()) {
    Assert(not _name.empty());
}
//----------------------------------------------------------------------------
FGenericApplication::~FGenericApplication() NOEXCEPT {}
//----------------------------------------------------------------------------
void FGenericApplication::Start() {
    LOG(Application, Emphasis, L"start application <{0}>", _name);

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
void FGenericApplication::Shutdown() {
    LOG(Application, Emphasis, L"shutdown application <{0}>", _name);

    FPlatformTime::LeaveLowResolutionTimer();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
