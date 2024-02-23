// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Application/ApplicationBase.h"

#include "ApplicationModule.h"

#include "Diagnostic/CurrentProcess.h"
#include "HAL/PlatformCrash.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformNotification.h"
#include "Modular/ModularDomain.h"
#include "Time/Timeline.h"

#include "Application.h"
#include "CoreModule.h"
#include "Diagnostic/Logger.h"
#include "IO/BufferedStream.h"
#include "IO/ConstNames.h"
#include "IO/Filename.h"
#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "VirtualFileSystem_fwd.h"
#include "Maths/MathHelpers.h"

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
        L"Debug"_view,
        L"Create mini dump"_view,
        []() {
        FPlatformCrash::WriteMiniDump();
    });
    pWindow->AddSystrayCommand(
        L"Debug"_view,
        L"Debug break"_view,
        []() {
        PPE_DEBUG_BREAK();
    });

#if USE_PPE_PLATFORM_DEBUG
    pWindow->AddSystrayCommand(
        L"Memory"_view,
        L"Check memory"_view,
        []() {
        FPlatformDebug::CheckMemory();
    });
#endif
    pWindow->AddSystrayCommand(
        L"Memory"_view,
        L"Dump memory leaks"_view,
        []() {
        FMallocDebug::DumpMemoryLeaks();
    });
    pWindow->AddSystrayCommand(
        L"Memory"_view,
        L"Release memory in modules"_view,
        []() {
        ReleaseMemoryInModules();
    });
    pWindow->AddSystrayCommand(
        L"Memory"_view,
        L"Report all tracking data"_view,
        []() {
        ReportAllTrackingData();
    });
    pWindow->AddSystrayCommand(
        L"Memory"_view,
        L"Report tracking data to CSV"_view,
    []() {
        const FDirpath savePath(FFSConstNames::SavedDir(), {
            FDirname{ L"TrackingData" } });

        if (VFS_CreateDirectory(savePath)) {
            FWString basename = StringFormat(L"{0}_{1}", RunningApp().Name(), FCoreModule::StaticInfo.BuildVersion.Family);
            FileSystem::Sanitize(basename.MutableView());

            const FFilename saveFilename{ savePath, FBasename(basename, FFSConstNames::Csv()) };

            const UStreamWriter saveWriter{
                VFS_RollFile(saveFilename, EAccessPolicy::Binary) };

            if (saveWriter) {
                PPE_LOG(Application, Info, "writing CSV tracking data to file: {0}", saveFilename);
                UsingBufferedStream(saveWriter.get(), [](IBufferedStreamWriter* buffered) {
                    FTextWriter oss{ buffered };
                    ReportCsvTrackingData(&oss);
                });
            }
        }
    });

    pWindow->AddSystrayCommand(
        L"Process"_view,
        L"Dump memory stats"_view,
        []() {
        FCurrentProcess::Get().LogMemoryStats();
    });
    pWindow->AddSystrayCommand(
        L"Process"_view,
        L"Dump process infos"_view,
        []() {
        FCurrentProcess::Get().LogProcessInfos();
    });
    pWindow->AddSystrayCommand(
        L"Process"_view,
        L"Dump storage infos"_view,
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
FApplicationBase::FApplicationBase(FModularDomain& domain, FString&& name)
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

    FModularServices& services = Services_();

#if !USE_PPE_FINAL_RELEASE
    SetupDebugMenuInSystray_(services);
#endif

    FApplicationModule::Get(Domain())._OnApplicationStart.Invoke(*this, services);

    ReportAllTrackingData();
}
//----------------------------------------------------------------------------
void FApplicationBase::Tick(FTimespan dt) {
    FPlatformApplication::Tick(dt);

    FApplicationModule::Get(Domain())._OnApplicationTick.Invoke(*this, dt);
}
//----------------------------------------------------------------------------
void FApplicationBase::Shutdown() {
    FModularServices& services = Services_();

#if !USE_PPE_FINAL_RELEASE
    TearDebugMenuInSystray_(services);
#endif

    FApplicationModule::Get(Domain())._OnApplicationShutdown.Invoke(*this, services);

    FPlatformApplication::Shutdown();
}
//----------------------------------------------------------------------------
void FApplicationBase::RequestExit() NOEXCEPT {
    if (HasRequestedExit())
        return;

    FPlatformApplication::RequestExit();

    PPE_LOG(Application, Info, "application {0} exit requested, will stop next loop...", Name());

    FApplicationModule::Get(Domain())._OnApplicationRequestExit.Invoke(*this);
}
//----------------------------------------------------------------------------
NO_INLINE bool FApplicationBase::ApplicationTick_(FTimespan dt) {
    if (not PumpMessages())
        return false;

    _OnApplicationBeginTick.Invoke(*this);

    Tick(dt);

    _OnApplicationEndTick.Invoke(*this);

    return true;
}
//----------------------------------------------------------------------------
void FApplicationBase::ApplicationLoop() {
    _timeline = FTimeline::StartNow();

    _OnApplicationBeginLoop.Invoke(*this);

    while (not HasRequestedExit()) {
        FTimespan actualTickRate = TickRate();
        if (LowerTickRateInBackground() and not HasFocus())
            actualTickRate = Timespan_1hz;

        FTimespan dt;
        if (Likely(_timeline.Tick_Every(actualTickRate, dt))) {
            if (not ApplicationTick_(dt))
                break;
            continue;
        }

        const FTimespan remaining = (actualTickRate - _timeline.Elapsed());
        FPlatformProcess::Sleep(static_cast<float>(FSeconds(remaining).Value() / 2/* bisect */));
    }

    _OnApplicationEndLoop.Invoke(*this);

    PPE_LOG(Application, Info, "application {0} stopped looping, total uptime = {2} (request:{1})", Name(), HasRequestedExit(), _timeline.Total());

    _timeline.Reset();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
