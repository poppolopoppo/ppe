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
#   include "Core/IO/TextWriter.h"
#endif

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
    STACKLOCAL_WTEXTWRITER(oss, 1024);
    oss << L"Memory statistics :" << Eol
        << L" - Total free size          = " << Fmt::FSizeInBytes{ memoryStats.TotalFreeSize } << Eol
        << L" - Largest free block       = " << Fmt::FSizeInBytes{ memoryStats.LargestFreeBlockSize } << Eol
        << L" - Total used size          = " << Fmt::FSizeInBytes{ memoryStats.TotalUsedSize } << Eol
        << L" - Largest used block       = " << Fmt::FSizeInBytes{ memoryStats.LargestUsedBlockSize } << Eol
        << L" - Total overhead size      = " << Fmt::FSizeInBytes{ memoryStats.TotalOverheadSize } << Eol
        << L" - Total committed size     = " << Fmt::FSizeInBytes{ memoryStats.TotalOverheadSize + memoryStats.TotalFreeSize + memoryStats.TotalUsedSize } << Eol
        << L" - External fragmentation   = " << (memoryStats.ExternalFragmentation() * 100) << L'%' << Eol;
    LOG(Info, oss.Written());
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

    UNUSED(debugHeapEnabled);
    UNUSED(debugCheckMemory);
    UNUSED(debugNecrophilia);
    UNUSED(debugLeaks);

    int debugHeapFlag = 0
        | debugHeapEnabled
        //| debugCheckMemory //%_NOCOMMIT%
        //| debugNecrophilia //%_NOCOMMIT%
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
            const FWString wwhat = ToWString(MakeCStringView(e.what()));
            Dialog::Ok(wwhat, L"FException caught while starting !", Dialog::Icon::Exclamation);
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
            const FWString wwhat = ToWString(MakeCStringView(e.what()));
            Dialog::Ok(wwhat, L"FException caught while shutting down !", Dialog::Icon::Exclamation);
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
