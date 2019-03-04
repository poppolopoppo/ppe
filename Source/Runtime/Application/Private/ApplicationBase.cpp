#include "stdafx.h"

#include "ApplicationBase.h"

#include "Diagnostic/CurrentProcess.h"
#include "HAL/PlatformCrash.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformNotification.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationBase::FApplicationBase(FWString&& name)
    : FPlatformApplication(std::move(name))
{}
//----------------------------------------------------------------------------
FApplicationBase::~FApplicationBase()
{}
//----------------------------------------------------------------------------
void FApplicationBase::Start() {
    FPlatformApplication::Start();

    using FAppNotify = Application::FPlatformNotification;

#if !USE_PPE_FINAL_RELEASE
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
        DumpMemoryLeaks();
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
        L"Dump process infos",
        []() {
        FCurrentProcess::Get().DumpProcessInfos();
    });
    FAppNotify::AddSystrayCommand(
        L"Process",
        L"Dump process memory",
        []() {
        FCurrentProcess::Get().DumpMemoryStats();
    });

#endif //!USE_PPE_FINAL_RELEASE
}
//----------------------------------------------------------------------------
void FApplicationBase::Shutdown() {
    using FAppNotify = Application::FPlatformNotification;

#if !USE_PPE_FINAL_RELEASE
    FAppNotify::HideSystray();
#endif

    FPlatformApplication::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
