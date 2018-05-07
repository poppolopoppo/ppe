#include "stdafx.h"

#include "Application.h"

#include "ApplicationBase.h"

#if CORE_APPLICATION_GRAPHICS && defined(PLATFORM_WINDOWS)
#   include "Input/XInputWrapper.h"
#   define WITH_APPLICATION_XINPUT
#endif

#include "Core/Allocator/PoolAllocatorTag-impl.h"
#include "Core/Diagnostic/CrtDebug.h"
#include "Core/Diagnostic/CurrentProcess.h"
#include "Core/Diagnostic/DialogBox.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/Misc/TargetPlatform.h"

#ifdef USE_DEBUG_LOGGER
#   include "Core/IO/FormatHelpers.h"
#   include "Core/IO/StreamProvider.h"
#   include "Core/IO/StringView.h"
#   include "Core/IO/TextWriter.h"
#endif

#include <clocale>
#include <locale.h>

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
#ifdef USE_DEBUG_LOGGER
static void PrintMemStats_(const Core::FCrtMemoryStats& memoryStats) {
    LOG(Application, Info,
        L"Memory statistics :\n"
        L" - Total free size          = {0}\n"
        L" - Largest free block       = {1}\n"
        L" - Total used size          = {2}\n"
        L" - Largest used block       = {3}\n"
        L" - Total overhead size      = {4}\n"
        L" - Total committed size     = {5}\n"
        L" - External fragmentation   = {6}%",
        Fmt::FSizeInBytes{ memoryStats.TotalFreeSize },
        Fmt::FSizeInBytes{ memoryStats.LargestFreeBlockSize },
        Fmt::FSizeInBytes{ memoryStats.TotalUsedSize },
        Fmt::FSizeInBytes{ memoryStats.LargestUsedBlockSize },
        Fmt::FSizeInBytes{ memoryStats.TotalOverheadSize },
        Fmt::FSizeInBytes{ memoryStats.TotalOverheadSize + memoryStats.TotalFreeSize + memoryStats.TotalUsedSize },
        (memoryStats.ExternalFragmentation() * 100));
}
#endif
//----------------------------------------------------------------------------
#if PLATFORM_WINDOWS
static void ConfigureCRTHeapForDebugging_() {
#   if defined(USE_CORE_MEMORY_DEBUGGING) || defined(_DEBUG)
    constexpr int debugHeapEnabled  = _CRTDBG_ALLOC_MEM_DF;
    constexpr int debugCheckMemory  = _CRTDBG_CHECK_EVERY_1024_DF;
    constexpr int debugNecrophilia  = _CRTDBG_DELAY_FREE_MEM_DF;
    constexpr int debugLeaks        = _CRTDBG_LEAK_CHECK_DF;

    UNUSED(debugHeapEnabled);
    UNUSED(debugCheckMemory);
    UNUSED(debugNecrophilia);
    UNUSED(debugLeaks);

    int debugHeapFlag = 0
        | debugHeapEnabled
        //| debugCheckMemory //%_NOCOMMIT%
        //| debugNecrophilia //%_NOCOMMIT%
        | debugLeaks;

    UNUSED(debugHeapFlag);

    _CrtSetDbgFlag(debugHeapFlag);

    // Report errors with a dialog box :
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);

    //_CrtSetBreakAlloc(447); // for leak debugging purpose // %_NOCOMMIT%
    //_CrtSetBreakAlloc(1246); // for leak debugging purpose // %_NOCOMMIT%

#   endif
}
#endif
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
    // Install crash exception handlers
    FPlatformCrashDump::SetExceptionHandlers();

    // Force locale to EN with UTF-8 encoding
    std::setlocale(LC_ALL, "en_US.UTF-8");

    // Set CRT heap debug options for debugging windows heap
#ifdef PLATFORM_WINDOWS
    ConfigureCRTHeapForDebugging_();
#endif
}
//----------------------------------------------------------------------------
FApplicationContext::~FApplicationContext() {
#ifdef USE_DEBUG_LOGGER
#   if defined(PLATFORM_WINDOWS)
    FCrtMemoryStats memoryStats;
    CrtDumpMemoryStats(&memoryStats);
#   endif
    PrintMemStats_(memoryStats);
#endif
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
            LOG(Application, Fatal, L"exception caught while shutting down : {0}", MakeCStringView(e.what()));
        })
#endif
    }
#ifndef FINAL_RELEASE
    ReportAllTrackingData();
    ShutdownLeakDetector();
#endif
    return FCurrentProcess::Instance().ExitCode();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
