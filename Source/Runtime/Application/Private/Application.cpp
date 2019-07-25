#include "stdafx.h"

#include "Application.h"

#include "ApplicationBase.h"

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformCrash.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformProfiler.h"

#if USE_PPE_LOGGER
#   include "IO/FormatHelpers.h"
#   include "IO/StreamProvider.h"
#   include "IO/StringView.h"
#   include "IO/TextWriter.h"
#endif

#define USE_APPLICATION_EXCEPTION_TRAP 0 //%_NOCOMMIT%

PRAGMA_INITSEG_LIB

namespace PPE {
namespace Application {
LOG_CATEGORY(PPE_APPLICATION_API, Application)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Functor>
static void ExceptionTrap_(const FWStringView& step, _Functor&& func) {
#if USE_APPLICATION_EXCEPTION_TRAP
    PPE_TRY
#endif
    {
        func();
    }
#if USE_APPLICATION_EXCEPTION_TRAP
    PPE_CATCH(const FException& e)
    PPE_CATCH_BLOCK({
        LOG(Application, Fatal, L"FException caught while {0} : {1}", step, e);
    })
    PPE_CATCH(const std::exception& e)
    PPE_CATCH_BLOCK({
        LOG(Application, Fatal, L"std::exception caught while {0} : {1}", step, MakeCStringView(e.what()));
    })
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationContext::FApplicationContext() {
    // Set FTZ + DAZ for FP_ASSIST
    FPlatformMaths::Disable_FP_Assist();
    // Signal platform specific code of application start
    FPlatformProcess::OnProcessStart();
    // Install crash exception handlers
    FPlatformCrash::SetExceptionHandlers();
    // Force locale to EN with UTF-8 encoding
    FPlatformMisc::SetUTF8Output();
}
//----------------------------------------------------------------------------
FApplicationContext::~FApplicationContext() {
    // Signal platform specific code of application shutdown
    FPlatformProcess::OnProcessShutdown();
}
//----------------------------------------------------------------------------
int LaunchApplication(const FApplicationContext& context, FApplicationBase* app) {
    UNUSED(context);
    AssertRelease(app);
#if USE_PPE_PLATFORM_PROFILER
    FPlatformProfiler::Name(FPlatformProfiler::ProcessLevel, ToString(app->Name()).data());
#endif
#if !USE_PPE_FINAL_RELEASE
    ReportAllTrackingData();
    FMallocDebug::StartLeakDetector();
#endif
    {
#if USE_APPLICATION_EXCEPTION_TRAP
        PPE_TRY
#endif
        {
            PPE_PROFILING_SCOPE("start application");
            app->Start();
        }
#if USE_APPLICATION_EXCEPTION_TRAP
        PPE_CATCH(const std::exception& e)
        PPE_CATCH_BLOCK({
            UNUSED(e);
            LOG(Application, Fatal, L"exception caught while starting : {0}", MakeCStringView(e.what()));
            FCurrentProcess::Get().SetExitCode(3);
        })
#endif
#if USE_APPLICATION_EXCEPTION_TRAP
        PPE_TRY
#endif
        {
            PPE_PROFILING_SCOPE("shutdown application");
            app->Shutdown();
        }
#if USE_APPLICATION_EXCEPTION_TRAP
        PPE_CATCH(const std::exception& e)
        PPE_CATCH_BLOCK({
            UNUSED(e);
            LOG(Application, Fatal, L"exception caught while shutting down : {0}", MakeCStringView(e.what()));
            FCurrentProcess::Get().SetExitCode(3);
        })
#endif
    }
#if !USE_PPE_FINAL_RELEASE
    ReportAllTrackingData();
    FMallocDebug::ShutdownLeakDetector();
#endif

    return FCurrentProcess::Get().ExitCode();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
