#include "stdafx.h"

#include "Application/ApplicationBase.h"

#include "ApplicationModule.h"

#include "Diagnostic/CurrentProcess.h"
#include "HAL/PlatformCrash.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformNotification.h"
#include "Modular/ModularDomain.h"
#include "Time/Timeline.h"

#include "Application.h"
#include "Diagnostic/BuildVersion.h"
#include "Diagnostic/Logger.h"
#include "IO/BufferedStream.h"
#include "IO/ConstNames.h"
#include "IO/Filename.h"
#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "VirtualFileSystem_fwd.h"

#if !USE_PPE_FINAL_RELEASE
#   include "Window/WindowService.h"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
static void SetupDebugMenuInSystray_(const FModularServices& services) {
    IWindowService* pWindow = services.GetIFP<IWindowService>();
    if (not pWindow)
        return;

    pWindow->ShowSystray();

    pWindow->AddSystrayCommand(
        L"Debug",
        L"Create mini dump",
        []() {
        FPlatformCrash::WriteMiniDump();
    });
    pWindow->AddSystrayCommand(
        L"Debug",
        L"Debug break",
        []() {
        PPE_DEBUG_BREAK();
    });

#if USE_PPE_PLATFORM_DEBUG
    pWindow->AddSystrayCommand(
        L"Memory",
        L"Check memory",
        []() {
        FPlatformDebug::CheckMemory();
    });
#endif
    pWindow->AddSystrayCommand(
        L"Memory",
        L"Dump memory leaks",
        []() {
        FMallocDebug::DumpMemoryLeaks();
    });
    pWindow->AddSystrayCommand(
        L"Memory",
        L"Release memory in modules",
        []() {
        ReleaseMemoryInModules();
    });
    pWindow->AddSystrayCommand(
        L"Memory",
        L"Report all tracking data",
        []() {
        ReportAllTrackingData();
    });
    pWindow->AddSystrayCommand(
        L"Memory",
        L"Report tracking data to CSV",
    []() {
        const FDirpath savePath(FFSConstNames::SavedDir(), {
            FDirname{ L"TrackingData" } });

        if (VFS_CreateDirectory(savePath)) {
            const FFilename saveFilename{
                savePath,
                FBasename{
                    StringFormat(L"{0}_{1}", RunningApp().Name(), CurrentBuildVersion().Family),
                    FFSConstNames::Csv() }};

            const UStreamWriter saveWriter{
                VFS_RollFile(saveFilename, EAccessPolicy::Binary) };

            if (saveWriter) {
                LOG(Application, Info, L"writing CSV tracking data to file: {0}", saveFilename);
                UsingBufferedStream(saveWriter.get(), [](IBufferedStreamWriter* buffered) {
                    FTextWriter oss{ buffered };
                    ReportCsvTrackingData(&oss);
                });
            }
        }
    });

    pWindow->AddSystrayCommand(
        L"Process",
        L"Dump memory stats",
        []() {
        FCurrentProcess::Get().LogMemoryStats();
    });
    pWindow->AddSystrayCommand(
        L"Process",
        L"Dump process infos",
        []() {
        FCurrentProcess::Get().LogProcessInfos();
    });
    pWindow->AddSystrayCommand(
        L"Process",
        L"Dump storage infos",
        []() {
        FCurrentProcess::Get().LogStorageInfos();
    });
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
static void TearDebugMenuInSystray_(const FModularServices& services) {
    IWindowService* pWindow = services.GetIFP<IWindowService>();
    if (not pWindow)
        return;
    pWindow->HideSystray();
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
    SetupDebugMenuInSystray_(Services());
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
    TearDebugMenuInSystray_(Services());
#endif

    FApplicationModule::Get(Domain())._OnApplicationShutdown.Invoke(*this);

    FPlatformApplication::Shutdown();
}
//----------------------------------------------------------------------------
void FApplicationBase::ApplicationLoop() {
    FTimespan dt;
    FTimeline tick = FTimeline::StartNow();

    while (PumpMessages()) {
        FTimespan tickRate{ Timespan_120hz() };

        if (Unlikely(not HasFocus()))
            tickRate = Timespan_15hz();

        if (tick.Tick_Every(tickRate, dt)) {
            Tick(dt);
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
