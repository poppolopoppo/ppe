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

    class OutputDebugStream : public std::wstringstream {
    public:
        ~OutputDebugStream() { ::OutputDebugStringW(str().c_str()); }
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
static void PrintMemStats_(const Core::CrtMemoryStats& memoryStats) {
#ifdef USE_DEBUG_LOGGER
    OutputDebugStream()
#else
    std::cerr
#endif
        << "Memory statistics :" << std::endl
        << " - Total free size          = " << memoryStats.TotalFreeSize << std::endl
        << " - Largest free block       = " << memoryStats.LargestFreeBlockSize << std::endl
        << " - Total used size          = " << memoryStats.TotalUsedSize << std::endl
        << " - Largest used block       = " << memoryStats.LargestUsedBlockSize << std::endl
        << " - Total overhead size      = " << memoryStats.TotalOverheadSize << std::endl
        << " - Total comitted size      = " << Core::SizeInBytes{ memoryStats.TotalOverheadSize.Value + memoryStats.TotalFreeSize.Value + memoryStats.TotalUsedSize.Value } << std::endl
        << " - External fragmentation   = " << (memoryStats.ExternalFragmentation() * 100) << "%" << std::endl;
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
void ApplicationStartup::Start() {
    CORE_MODULE_START(Application);

    POOL_TAG(Application)::Start();
}
//----------------------------------------------------------------------------
void ApplicationStartup::Shutdown() {
    CORE_MODULE_SHUTDOWN(Application);

    POOL_TAG(Application)::Shutdown();
}
//----------------------------------------------------------------------------
void ApplicationStartup::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(Application);

    POOL_TAG(Application)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ApplicationContext::ApplicationContext() {
#ifdef OS_WINDOWS
    ConfigureCRTHeapForDebugging_();
#endif
}
//----------------------------------------------------------------------------
ApplicationContext::~ApplicationContext() {
#ifndef FINAL_RELEASE
#if defined(OS_WINDOWS)
    CrtMemoryStats memoryStats;
    CrtDumpMemoryStats(&memoryStats);
#endif
    PrintMemStats_(memoryStats);
#endif
}
//----------------------------------------------------------------------------
int LaunchApplication(const ApplicationContext& context, ApplicationBase* app) {
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
            const WString wwhat = ToWString(e.what());
            Dialog::Ok(wwhat.c_str(), L"Exception caught while starting !", Dialog::Icon::Exclamation);
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
            const WString wwhat = ToWString(e.what());
            Dialog::Ok(wwhat.c_str(), L"Exception caught while shutting down !", Dialog::Icon::Exclamation);
            AssertNotReached();
        }
#endif
    }
#ifndef FINAL_RELEASE
    ReportAllTrackingData();
#endif
    return CurrentProcess::Instance().ExitCode();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
