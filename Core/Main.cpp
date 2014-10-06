#include "stdafx.h"

#include "Core/Core.h"
#include "Core/Logger.h"
#include "Core/MessageBox.h"
#include "Core/UniquePtr.h"

#include "Core.Graphics/Graphics.h"
#include "Core.Engine/Engine.h"
#include "Core.Application/ApplicationWindow.h"

#include <iostream>

#ifdef OS_WINDOWS
//  Retrieves application icon for windows
#   include <windows.h>
#   ifdef MessageBox
#       undef MessageBox
#   endif
#   include "resource.h"
#   ifndef IDI_ICON1
#       define IDI_ICON1 101 // work around buggy resource handling ...
#   endif
#endif

#define APPLICATION_TYPE 3

#if   (0 == APPLICATION_TYPE)
#   include "Core.Test/ApplicationTest.h"
typedef Core::ApplicationTest application_type;
#elif (1 == APPLICATION_TYPE)
#   include "Core.Test/GameTest.h"
typedef Core::GameTest application_type;
#elif (2 == APPLICATION_TYPE)
#   include "Core.Test/GameTest2.h"
typedef Core::GameTest2 application_type
#elif (3 == APPLICATION_TYPE)
#   include "Core.Test/GameTest3.h"
typedef Core::GameTest3 application_type;
#else
#   error "unknown application type"
#endif

static void Print(const Core::CrtMemoryStats& memoryStats) {
#ifdef OS_WINDOWS
    char buffer[4096];
    {
        Core::OCStrStream oss(buffer);
        oss     << "Memory statistics :" << std::endl
                << " - Total free size          = " << memoryStats.TotalFreeSize << std::endl
                << " - Largest free block       = " << memoryStats.LargestFreeBlockSize << std::endl
                << " - Total used size          = " << memoryStats.TotalUsedSize << std::endl
                << " - Largest used block       = " << memoryStats.LargestUsedBlockSize << std::endl
                << " - Total overhead size      = " << memoryStats.TotalOverheadSize << std::endl
                << " - Total comitted size      = " << Core::SizeInBytes{ memoryStats.TotalOverheadSize.Value + memoryStats.TotalFreeSize.Value + memoryStats.TotalUsedSize.Value } << std::endl
                << " - External fragmentation   = " << (memoryStats.ExternalFragmentation() * 100) << "%" << std::endl;
    }
    OutputDebugStringA(buffer);
#else
    std::cerr << "Memory statistics :" << std::endl
        << " - Total free size          = " << memoryStats.TotalFreeSize << std::endl
        << " - Largest free block       = " << memoryStats.LargestFreeBlockSize << std::endl
        << " - Total used size          = " << memoryStats.TotalUsedSize << std::endl
        << " - Largest used block       = " << memoryStats.LargestUsedBlockSize << std::endl
        << " - Total overhead size      = " << memoryStats.TotalOverheadSize << std::endl
        << " - Total comitted size      = " << Core::SizeInBytes{ memoryStats.TotalOverheadSize.Value + memoryStats.TotalFreeSize.Value + memoryStats.TotalUsedSize.Value } << std::endl
        << " - External fragmentation   = " << (memoryStats.ExternalFragmentation() * 100) << "%" << std::endl;
#endif
}

template <typename _Application>
static int Bootstrap(void *applicationHandle, int nShowCmd, int argc, const wchar_t**argv) {
    using namespace Core;

    CoreStartup startup{ applicationHandle, nShowCmd, size_t(argc), argv };
    Graphics::GraphicsStartup startupGraphics;
    Engine::EngineStartup startupEngine;

#ifdef OS_WINDOWS
    CurrentProcess::Instance().SetAppIcon(IDI_ICON1);
#endif

#ifdef _DEBUG
    try
#endif
    {
        _Application app;
        Application::LaunchApplication(&app);
    }
#ifdef _DEBUG
    catch (const std::exception& e)
    {
        const WString wwhat = ToWString(e.what());
        MessageBox::Ok(wwhat.c_str(), L"Exception caught !", MessageBox::Icon::Exclamation);
        AssertNotReached();
    }
#endif

    ReportDomainTrackingData();
    return CurrentProcess::Instance().ExitCode();
}

#ifdef OS_WINDOWS
#   include <Windows.h>
#   include <shellapi.h>
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd) {
#else
int main(int argc, const wchar_t* argv[]) {
#endif

#ifdef OS_WINDOWS
#   ifdef _DEBUG
    //_CrtSetBreakAlloc(1146); // for leak debugging purpose // %__NOCOMMIT%
#   endif

    int argc;
    wchar_t *const *argv = CommandLineToArgvW(lpCmdLine, &argc);
#endif

    int result = 0;
    using namespace Core;
    {
        result = Bootstrap<application_type>(hInstance, nShowCmd, argc, const_cast<const wchar_t**>(&argv[0]));

#ifdef OS_WINDOWS
        CrtMemoryStats memoryStats;
        CrtDumpMemoryStats(&memoryStats);
        Print(memoryStats);
#endif
    }
    return result;
}
