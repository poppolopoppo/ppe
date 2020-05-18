#include "stdafx.h"

#include "ApplicationBase.h"

#include "Diagnostic/CurrentProcess.h"
#include "HAL/PlatformCrash.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformNotification.h"
#include "Modular/ModularDomain.h"

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
FApplicationBase::FApplicationBase(const FModularDomain& domain, FWString&& name)
:   FPlatformApplication(domain, std::move(name))
{}
//----------------------------------------------------------------------------
FApplicationBase::~FApplicationBase() = default;
//----------------------------------------------------------------------------
void FApplicationBase::Start() {
    FPlatformApplication::Start();

#if !USE_PPE_FINAL_RELEASE
    if (ShouldUseDebugSystray_())
        SetupDebugMenuInSystray_();
#endif
}
//----------------------------------------------------------------------------
void FApplicationBase::Shutdown() {
#if !USE_PPE_FINAL_RELEASE
    if (ShouldUseDebugSystray_())
        TearDebugMenuInSystray_();
#endif

    FPlatformApplication::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
