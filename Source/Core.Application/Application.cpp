#include "stdafx.h"

#include "Application.h"

#include "ApplicationBase.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"
#include "Core/Diagnostic/CrtDebug.h"
#include "Core/Diagnostic/CurrentProcess.h"
#include "Core/Diagnostic/DialogBox.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/Stream.h"
#include "Core/Misc/TargetPlatform.h"

#define WITH_APPLICATION_TRY_CATCH 0 //%_NOCOMMIT%

PRAGMA_INITSEG_LIB

namespace Core {
namespace Application {
POOL_TAG_DEF(Application);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_DEBUG_LOGGER
static void PrintMemStats_(const Core::FCrtMemoryStats& memoryStats) {
    FStackLocalLoggerStream(ELogCategory::Info)
        << "Memory statistics :" << eol
        << " - Total free size          = " << memoryStats.TotalFreeSize << eol
        << " - Largest free block       = " << memoryStats.LargestFreeBlockSize << eol
        << " - Total used size          = " << memoryStats.TotalUsedSize << eol
        << " - Largest used block       = " << memoryStats.LargestUsedBlockSize << eol
        << " - Total overhead size      = " << memoryStats.TotalOverheadSize << eol
        << " - Total comitted size      = " << Core::FSizeInBytes{ memoryStats.TotalOverheadSize.Value + memoryStats.TotalFreeSize.Value + memoryStats.TotalUsedSize.Value } << eol
        << " - External fragmentation   = " << (memoryStats.ExternalFragmentation() * 100) << "%" << eol;
}
#endif
//----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
static void ConfigureCRTHeapForDebugging_() {
#   ifdef _DEBUG
    constexpr int debugHeapEnabled  = _CRTDBG_ALLOC_MEM_DF;
    constexpr int debugCheckMemory  = _CRTDBG_CHECK_EVERY_1024_DF;
    constexpr int debugNecrophilia  = _CRTDBG_DELAY_FREE_MEM_DF;
    constexpr int debugLeaks        = _CRTDBG_LEAK_CHECK_DF;

    int debugHeapFlag = 0
        | debugHeapEnabled
        //| debugCheckMemory //%_NOCOMMIT%
        | debugNecrophilia
        | debugLeaks;

    _CrtSetDbgFlag(debugHeapFlag);

    // Report errors with a dialog box :
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);

    //_CrtSetBreakAlloc(1869); // for leak debugging purpose // %_NOCOMMIT%
#   endif
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FApplicationStartup::Start() {
    CORE_MODULE_START(Application);

    POOL_TAG(Application)::Start();
}
//----------------------------------------------------------------------------
void FApplicationStartup::Shutdown() {
    CORE_MODULE_SHUTDOWN(Application);

    POOL_TAG(Application)::Shutdown();
}
//----------------------------------------------------------------------------
void FApplicationStartup::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(Application);

    POOL_TAG(Application)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationContext::FApplicationContext() {
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
    {
#if WITH_APPLICATION_TRY_CATCH
        CORE_TRY
#endif
        {
            app->Start();
        }
#if WITH_APPLICATION_TRY_CATCH
        CORE_CATCH(const std::exception& e)
        CORE_CATCH_BLOCK({
            const FWString wwhat = ToWString(e.what());
            Dialog::Ok(wwhat.c_str(), L"FException caught while starting !", Dialog::Icon::Exclamation);
            AssertNotReached();
        })
#endif

#if WITH_APPLICATION_TRY_CATCH
        CORE_TRY
#endif
        {
            app->Shutdown();
        }
#if WITH_APPLICATION_TRY_CATCH
        CORE_CATCH(const std::exception& e)
            CORE_CATCH_BLOCK({
            const FWString wwhat = ToWString(e.what());
            Dialog::Ok(wwhat.c_str(), L"FException caught while shutting down !", Dialog::Icon::Exclamation);
            AssertNotReached();
        })
#endif
    }
#ifndef FINAL_RELEASE
    ReportAllTrackingData();
#endif
    return FCurrentProcess::Instance().ExitCode();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
