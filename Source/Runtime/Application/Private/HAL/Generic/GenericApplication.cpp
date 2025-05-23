﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Generic/GenericApplication.h"

#include "ApplicationModule.h"
#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformMessageHandler.h"
#include "HAL/PlatformTime.h"
#include "Maths/MathHelpers.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATION_API, Application)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGenericApplication::FGenericApplication(FModularDomain& domain, FString&& name)
:   _domain(domain)
,   _name(std::move(name))
,   _services(_name, &_domain.Services())
,   _tickRate(Timespan_120hz)
,   _requestedExit(false)
,   _lowerTickRateInBackground(true) {
    Assert(not _name.empty());

#if !USE_PPE_FINAL_RELEASE
    if (FCurrentProcess::StartedWithDebugger())
        _lowerTickRateInBackground = false;
#endif

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

    _realTime.ResetToZero();
    _requestedExit = false;
}
//----------------------------------------------------------------------------
void FGenericApplication::Run() {
    PPE_SLOG(Application, Emphasis, "run application", {{"application", _name}});
}
//----------------------------------------------------------------------------
void FGenericApplication::Shutdown() {
    PPE_SLOG(Application, Emphasis, "shutdown application", {{"application", _name}});

    _OnApplicationTick.Clear();

    FPlatformTime::LeaveLowResolutionTimer();
}
//----------------------------------------------------------------------------
bool FGenericApplication::PumpMessages() NOEXCEPT {
    return FPlatformMessageHandler::PumpGlobalMessages();
}
//----------------------------------------------------------------------------
void FGenericApplication::Tick(FTimespan dt) {
    _realTime.Tick(dt);
    _applicationTime.Tick(_realTime, _applicationTimeDilation);

    _OnApplicationTick.Invoke(*this, dt);
}
//----------------------------------------------------------------------------
void FGenericApplication::ReleaseMemory() NOEXCEPT {
    PPE_SLOG(Application, Emphasis, "release memory in application", {{"application", _name}});

    _services.ReleaseMemory();
}
//----------------------------------------------------------------------------
void FGenericApplication::SetApplicationTimeDilation(float speed) NOEXCEPT {
    _applicationTimeDilation = speed;
}
//----------------------------------------------------------------------------
void FGenericApplication::SetLowerTickRateInBackground(bool enabled) NOEXCEPT {
    _lowerTickRateInBackground = enabled;
}
//----------------------------------------------------------------------------
void FGenericApplication::SetTickRate(FTimespan period) NOEXCEPT {
    Assert(period > 0);
    _tickRate = (1000.0 / *period);
}
//----------------------------------------------------------------------------
void FGenericApplication::RequestExit() NOEXCEPT {
    _requestedExit = true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
