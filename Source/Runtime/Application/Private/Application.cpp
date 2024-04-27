// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Application.h"

#include "Application/ApplicationBase.h"

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
EXTERN_LOG_CATEGORY(PPE_APPLICATION_API, Application)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FApplicationBase* GRunningApp_ = nullptr;
//----------------------------------------------------------------------------
template <typename _Functor>
static void ExceptionTrap_(const FWStringView& step, _Functor&& func) {
    Unused(step);
#if USE_APPLICATION_EXCEPTION_TRAP
    PPE_TRY
#endif
    {
        func();
    }
#if USE_APPLICATION_EXCEPTION_TRAP
    PPE_CATCH(const FException& e)
    PPE_CATCH_BLOCK({
        PPE_LOG(Application, Fatal, "FException caught while {0} : {1}", step, e);
    })
    PPE_CATCH(const std::exception& e)
    PPE_CATCH_BLOCK({
        PPE_LOG(Application, Fatal, "std::exception caught while {0} : {1}", step, MakeCStringView(e.what()));
    })
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationBase& RunningApp() NOEXCEPT {
    Assert(GRunningApp_);
    return (*GRunningApp_);
}
//----------------------------------------------------------------------------
int LaunchApplication(FApplicationBase* app) {
    AssertRelease(app);
    AssertRelease(GRunningApp_ == nullptr);

    GRunningApp_ = app;

#if USE_PPE_PLATFORM_PROFILER
    FPlatformProfiler::Name(FPlatformProfiler::ProcessLevel, ToString(app->Name()).data());
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
            Unused(e);
            PPE_LOG(Application, Fatal, "exception caught while starting: {0}", MakeCStringView(e.what()));
            FCurrentProcess::Get().SetExitCode(3);
        })
#endif
#if USE_APPLICATION_EXCEPTION_TRAP
        PPE_TRY
#endif
        {
            PPE_PROFILING_SCOPE("run application");
            app->Run();
        }
#if USE_APPLICATION_EXCEPTION_TRAP
        PPE_CATCH(const std::exception& e)
        PPE_CATCH_BLOCK({
            Unused(e);
            PPE_LOG(Application, Fatal, "exception caught while running: {0}", MakeCStringView(e.what()));
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
            Unused(e);
            PPE_LOG(Application, Fatal, "exception caught while shutting down: {0}", MakeCStringView(e.what()));
            FCurrentProcess::Get().SetExitCode(3);
        })
#endif
    }

    Assert(app == GRunningApp_);
    GRunningApp_ = nullptr;

    return FCurrentProcess::Get().ExitCode();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
