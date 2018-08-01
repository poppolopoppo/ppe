#include "stdafx.h"

#include "Application.h"

#include "ApplicationBase.h"

#if CORE_APPLICATION_GRAPHICS && defined(PLATFORM_WINDOWS)
#   include "Input/XInputWrapper.h"
#   define WITH_APPLICATION_XINPUT
#endif

#include "Core/Allocator/PoolAllocatorTag-impl.h"
#include "Core/Diagnostic/CurrentProcess.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/HAL/PlatformCrash.h"
#include "Core/HAL/PlatformMisc.h"
#include "Core/HAL/PlatformProcess.h"

#ifdef USE_DEBUG_LOGGER
#   include "Core/IO/FormatHelpers.h"
#   include "Core/IO/StreamProvider.h"
#   include "Core/IO/StringView.h"
#   include "Core/IO/TextWriter.h"
#endif

#include <clocale>
#include <locale.h>

#ifdef PLATFORM_WINDOWS
#   include <mbctype.h>
#endif

#define USE_APPLICATION_EXCEPTION_TRAP 1 //%_NOCOMMIT%

PRAGMA_INITSEG_LIB

namespace Core {
namespace Application {
POOL_TAG_DEF(Application);
LOG_CATEGORY(CORE_APPLICATION_API, Application)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Functor>
static void ExceptionTrap_(const FWStringView& step, _Functor&& func) {
#if USE_APPLICATION_EXCEPTION_TRAP
    CORE_TRY
#endif
    {
        func();
    }
#if USE_APPLICATION_EXCEPTION_TRAP
    CORE_CATCH(const FException& e)
    CORE_CATCH_BLOCK({
        LOG(Application, Fatal, L"FException caught while {0} : {1}", step, e);
    })
    CORE_CATCH(const std::exception& e)
    CORE_CATCH_BLOCK({
        LOG(Application, Fatal, L"std::exception caught while {0} : {1}", step, MakeCStringView(e.what()));
    })
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FApplicationModule::Start() {
    CORE_MODULE_START(Application);

    POOL_TAG(Application)::Start();

#ifdef WITH_APPLICATION_XINPUT
    FXInputWrapper::Create();
#endif
}
//----------------------------------------------------------------------------
void FApplicationModule::Shutdown() {
    CORE_MODULE_SHUTDOWN(Application);

#ifdef WITH_APPLICATION_XINPUT
    FXInputWrapper::Destroy();
#endif

    POOL_TAG(Application)::Shutdown();
}
//----------------------------------------------------------------------------
void FApplicationModule::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(Application);

    POOL_TAG(Application)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationContext::FApplicationContext() {
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
#ifndef FINAL_RELEASE
    StartLeakDetector();
#endif
    {
#if USE_APPLICATION_EXCEPTION_TRAP
        CORE_TRY
#endif
        {
            app->Start();
        }
#if USE_APPLICATION_EXCEPTION_TRAP
        CORE_CATCH(const std::exception& e)
        CORE_CATCH_BLOCK({
            UNUSED(e);
            LOG(Application, Fatal, L"exception caught while starting : {0}", MakeCStringView(e.what()));
        })
#endif

#if USE_APPLICATION_EXCEPTION_TRAP
        CORE_TRY
#endif
        {
            app->Shutdown();
        }
#if USE_APPLICATION_EXCEPTION_TRAP
        CORE_CATCH(const std::exception& e)
        CORE_CATCH_BLOCK({
            UNUSED(e);
            LOG(Application, Fatal, L"exception caught while shutting down : {0}", MakeCStringView(e.what()));
        })
#endif
    }
#ifndef FINAL_RELEASE
    ReportAllTrackingData();
    ShutdownLeakDetector();
#endif
    return FCurrentProcess::Get().ExitCode();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
