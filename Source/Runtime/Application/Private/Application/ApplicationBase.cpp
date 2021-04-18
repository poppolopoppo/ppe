#include "stdafx.h"

#include "Application/ApplicationBase.h"

#include "ApplicationModule.h"

#include "Diagnostic/CurrentProcess.h"
#include "HAL/PlatformCrash.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformNotification.h"
#include "Modular/ModularDomain.h"
#include "Time/Timeline.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
static bool ShouldUseDebugSystray_() {
    ONE_TIME_INITIALIZE(bool, GEnabled, not FCurrentProcess::Get().StartedWithDebugger());
    return GEnabled;
}
#endif
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
static void SetupDebugMenuInSystray_() {

    using FAppNotify = Application::FPlatformNotification;

    FAppNotify::ShowSystray();

    FAppNotify::AddSystrayCommand(
        L"Debug",
        L"Create mini dump",
        []() {
        FPlatformCrash::WriteMiniDump();
    });
    FAppNotify::AddSystrayCommand(
        L"Debug",
        L"Debug break",
        []() {
        PPE_DEBUG_BREAK();
    });

#if USE_PPE_PLATFORM_DEBUG
    FAppNotify::AddSystrayCommand(
        L"Memory",
        L"Check memory",
        []() {
        FPlatformDebug::CheckMemory();
    });
#endif
    FAppNotify::AddSystrayCommand(
        L"Memory",
        L"Dump memory leaks",
        []() {
        FMallocDebug::DumpMemoryLeaks();
    });
    FAppNotify::AddSystrayCommand(
        L"Memory",
        L"Release memory in modules",
        []() {
        ReleaseMemoryInModules();
    });
    FAppNotify::AddSystrayCommand(
        L"Memory",
        L"Report all tracking data",
        []() {
        ReportAllTrackingData();
    });

    FAppNotify::AddSystrayCommand(
        L"Process",
        L"Dump memory stats",
        []() {
        FCurrentProcess::Get().LogMemoryStats();
    });
    FAppNotify::AddSystrayCommand(
        L"Process",
        L"Dump process infos",
        []() {
        FCurrentProcess::Get().LogProcessInfos();
    });
    FAppNotify::AddSystrayCommand(
        L"Process",
        L"Dump storage infos",
        []() {
        FCurrentProcess::Get().LogStorageInfos();
    });
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
static void TearDebugMenuInSystray_() {
    using FAppNotify = Application::FPlatformNotification;

    FAppNotify::HideSystray();
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationBase::FApplicationBase(const FModularDomain& domain, FString&& name)
:   FPlatformApplication(domain, std::move(name)) {
    FApplicationModule::Get(Domain())._OnApplicationCreate.Invoke(*this);
}
//----------------------------------------------------------------------------
FApplicationBase::~FApplicationBase() NOEXCEPT {
    FApplicationModule::Get(Domain())._OnApplicationDestroy.Invoke(*this);
}
//----------------------------------------------------------------------------
void FApplicationBase::Start() {
    FPlatformApplication::Start();

#if !USE_PPE_FINAL_RELEASE
    if (ShouldUseDebugSystray_())
        SetupDebugMenuInSystray_();
#endif

    FApplicationModule::Get(Domain())._OnApplicationStart.Invoke(*this);

    ReportAllTrackingData();
}
//----------------------------------------------------------------------------
void FApplicationBase::Tick(FTimespan dt) {
    FPlatformApplication::Tick(dt);

    FApplicationModule::Get(Domain())._OnApplicationTick.Invoke(*this, dt);
}
//----------------------------------------------------------------------------
void FApplicationBase::Shutdown() {
#if !USE_PPE_FINAL_RELEASE
    if (ShouldUseDebugSystray_())
        TearDebugMenuInSystray_();
#endif

    FApplicationModule::Get(Domain())._OnApplicationShutdown.Invoke(*this);

    FPlatformApplication::Shutdown();
}
//----------------------------------------------------------------------------
void FApplicationBase::ApplicationLoop() {
	FTimespan dt;
	FTimeline tick = FTimeline::StartNow();

	while (PumpMessages()) {
		if (tick.Tick_Target60FPS(dt))
			Tick(dt);
	}
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
