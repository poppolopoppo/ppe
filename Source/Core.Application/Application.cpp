#include "stdafx.h"

#include "Application.h"

#include "ApplicationBase.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"
#include "Core/Diagnostic/CrtDebug.h"
#include "Core/Diagnostic/DialogBox.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/Stream.h"
#include "Core/Misc/CurrentProcess.h"

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

namespace Core {
namespace Application {
POOLTAG_DEF(Application);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#ifdef OS_WINDOWS
static void GuaranteeStackSizeForStackOverflowRecovery_()
{
    HMODULE moduleHandle = GetModuleHandleA("kernel32.dll");
    if(moduleHandle) {
        typedef BOOL (WINAPI *TSetThreadStackGuarantee)(PULONG StackSizeInBytes);
        TSetThreadStackGuarantee setThreadStackGuarantee = (TSetThreadStackGuarantee)GetProcAddress(moduleHandle, "SetThreadStackGuarantee");
        if(setThreadStackGuarantee) {
            ULONG stackSizeInBytes = 0;
            if(1 == (*setThreadStackGuarantee)(&stackSizeInBytes))
            {
                stackSizeInBytes += 1*1024*1024;
                if (1 == (*setThreadStackGuarantee)(&stackSizeInBytes))
                    return;
            }
        }
    }
    LOG(Warning, L"Unable to SetThreadStackGuarantee, Stack Overflows won't be caught properly !");
}
#endif
//----------------------------------------------------------------------------
#ifdef OS_WINDOWS
static void ConfigureCRTHeapForDebugging_() {
#   ifdef _DEBUG
    const int debugCheckHeap = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_EVERY_1024_DF;
    const int debugNecrophilia = _CRTDBG_DELAY_FREE_MEM_DF;
    const int debugLeaks = _CRTDBG_LEAK_CHECK_DF;

    int debugHeapFlag = 0;
    //debugHeapFlag |= debugCheckHeap; %_NOCOMMIT%
    debugHeapFlag |= debugNecrophilia;
    debugHeapFlag |= debugLeaks;

    _CrtSetDbgFlag(debugHeapFlag);

    // Report errors with a dialog box :
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);

    //_CrtSetBreakAlloc(1146); // for leak debugging purpose // %__NOCOMMIT%
#   endif
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ApplicationStartup::Start() {
    POOLTAG(Application)::Start();
}
//----------------------------------------------------------------------------
void ApplicationStartup::Shutdown() {
    POOLTAG(Application)::Shutdown();
}
//----------------------------------------------------------------------------
void ApplicationStartup::ClearAll_UnusedMemory() {
    POOLTAG(Application)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ApplicationContext::ApplicationContext() {
#ifdef OS_WINDOWS
    GuaranteeStackSizeForStackOverflowRecovery_();
    ConfigureCRTHeapForDebugging_();
#endif
}
//----------------------------------------------------------------------------
ApplicationContext::~ApplicationContext() {
#if defined(OS_WINDOWS) && !defined(FINAL_RELEASE)
    CrtMemoryStats memoryStats;
    CrtDumpMemoryStats(&memoryStats);
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
            DialogBox::Ok(wwhat.c_str(), L"Exception caught while starting !", DialogBox::Icon::Exclamation);
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
            DialogBox::Ok(wwhat.c_str(), L"Exception caught while shutting down !", DialogBox::Icon::Exclamation);
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
