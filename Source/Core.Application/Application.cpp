#include "stdafx.h"

#include "Application.h"

#include "ApplicationBase.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"
#include "Core/Diagnostic/CrtDebug.h"
#include "Core/Diagnostic/CurrentProcess.h"
#include "Core/Diagnostic/DialogBox.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/Stream.h"

#ifdef OS_WINDOWS
#   include <sstream>
#   include <windows.h>

    class FOutputDebugStream : public std::wstringstream {
    public:
        ~FOutputDebugStream() { ::OutputDebugStringW(str().c_str()); }
    };

#endif

#include <sstream>

#define WITH_APPLICATION_TRY_CATCH 0 //%_NOCOMMIT%

#ifdef CPP_VISUALSTUDIO
#   pragma warning(disable: 4073) // initialiseurs placés dans la zone d'initialisation d'une bibliothèque
#   pragma init_seg(lib)
#else
#   error "missing compiler specific command"
#endif

namespace Core {
namespace Application {
POOL_TAG_DEF(Application);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
static void PrintMemStats_(const Core::FCrtMemoryStats& memoryStats) {
#ifdef USE_DEBUG_LOGGER
    FOutputDebugStream()
#else
    std::cerr
#endif
        << "Memory statistics :" << eol
        << " - Total free size          = " << memoryStats.TotalFreeSize << eol
        << " - Largest free block       = " << memoryStats.LargestFreeBlockSize << eol
        << " - Total used size          = " << memoryStats.TotalUsedSize << eol
        << " - Largest used block       = " << memoryStats.LargestUsedBlockSize << eol
        << " - Total overhead size      = " << memoryStats.TotalOverheadSize << eol
        << " - Total comitted size      = " << Core::SizeInBytes{ memoryStats.TotalOverheadSize.Value + memoryStats.TotalFreeSize.Value + memoryStats.TotalUsedSize.Value } << eol
        << " - External fragmentation   = " << (memoryStats.ExternalFragmentation() * 100) << "%" << eol;
}
#endif
//----------------------------------------------------------------------------
#ifdef OS_WINDOWS
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
#ifdef OS_WINDOWS
    ConfigureCRTHeapForDebugging_();
#endif
}
//----------------------------------------------------------------------------
FApplicationContext::~FApplicationContext() {
#ifndef FINAL_RELEASE
#if defined(OS_WINDOWS)
    FCrtMemoryStats memoryStats;
    CrtDumpMemoryStats(&memoryStats);
#endif
    PrintMemStats_(memoryStats);
#endif
}
//----------------------------------------------------------------------------
int LaunchApplication(const FApplicationContext& context, FApplicationBase* app) {
    UNUSED(context);
    AssertRelease(app);
    {
#if WITH_APPLICATION_TRY_CATCH
        try
#endif
        {
            app->Start();
        }
#if WITH_APPLICATION_TRY_CATCH
        catch (const std::exception& e)
        {
            const FWString wwhat = ToWString(e.what());
            Dialog::Ok(wwhat.c_str(), L"FException caught while starting !", Dialog::Icon::Exclamation);
            AssertNotReached();
        }
#endif

#if WITH_APPLICATION_TRY_CATCH
        try
#endif
        {
            app->Shutdown();
        }
#if WITH_APPLICATION_TRY_CATCH
        catch (const std::exception& e)
        {
            const FWString wwhat = ToWString(e.what());
            Dialog::Ok(wwhat.c_str(), L"FException caught while shutting down !", Dialog::Icon::Exclamation);
            AssertNotReached();
        }
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
