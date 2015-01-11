#include "stdafx.h"

#include "Core/Core.h"
#include "Core/Diagnostic/CrtDebug.h"
#include "Core/Diagnostic/DialogBox.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/Memory/UniquePtr.h"

#include "Core.Application/ApplicationWindow.h"
#include "Core.Serialize/Serialize.h"
#include "Core.Engine/Engine.h"
#include "Core.Graphics/Graphics.h"

#include <iostream>

#define APPLICATION_TYPE 2

#if   (0 == APPLICATION_TYPE)
#   include "ApplicationTest.h"
typedef Core::ApplicationTest application_type;
#elif (1 == APPLICATION_TYPE)
#   include "GameTest.h"
typedef Core::GameTest application_type;
#elif (2 == APPLICATION_TYPE)
#   include "GameTest2.h"
typedef Core::GameTest2 application_type;
#elif (3 == APPLICATION_TYPE)
#   include "GameTest3.h"
typedef Core::GameTest3 application_type;
#else
#   error "unknown application type"
#endif

#ifdef OS_WINDOWS
#   define CORE_RESOURCES 1
#else
#   define CORE_RESOURCES 0
#endif

#if defined(OS_WINDOWS) && CORE_RESOURCES
//  Retrieves application icon for windows
#   include <windows.h>
#   include "resource.h"
#endif

static void PrintMemStats(const Core::CrtMemoryStats& memoryStats) {
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
    ::OutputDebugStringA(buffer);
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

    Core::CoreStartup startupCore{ applicationHandle, nShowCmd, size_t(argc), argv };
    Serialize::SerializeStartup startupSerialize;
    Graphics::GraphicsStartup startupGraphics;
    Engine::EngineStartup startupEngine;

#if defined(OS_WINDOWS) && CORE_RESOURCES
    CurrentProcess::Instance().SetAppIcon(IDI_WINDOW_ICON);
#endif

#ifndef FINAL_RELEASE
    try
#endif
    {
        _Application app;
        Application::LaunchApplication(&app);
    }
#ifndef FINAL_RELEASE
    catch (const std::exception& e)
    {
        const WString wwhat = ToWString(e.what());
        DialogBox::Ok(wwhat.c_str(), L"Exception caught !", DialogBox::Icon::Exclamation);
        AssertNotReached();
    }
#endif

#ifndef FINAL_RELEASE
    ReportAllTrackingData();
#endif

    return CurrentProcess::Instance().ExitCode();
}

#ifdef OS_WINDOWS
#   include <Windows.h>
#   include <shellapi.h>
#   include <tchar.h>
int APIENTRY _tWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow) {
#else
int main(int argc, const wchar_t* argv[]) {
#endif

#ifdef OS_WINDOWS
#   ifdef _DEBUG
    int debugHeapFlag = _CRTDBG_ALLOC_MEM_DF |
                        _CRTDBG_CHECK_EVERY_1024_DF |
                        _CRTDBG_DELAY_FREE_MEM_DF |
                        _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(debugHeapFlag);
    // les erreurs / assert de check lance une window qui break
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
    //_CrtSetBreakAlloc(1146); // for leak debugging purpose // %__NOCOMMIT%
#   endif

    int argc;
    wchar_t *const *argv = ::CommandLineToArgvW(lpCmdLine, &argc);
#endif

    int result = 0;
    using namespace Core;
    {
        result = Bootstrap<application_type>(hInstance, nCmdShow, argc, const_cast<const wchar_t**>(&argv[0]));

#if defined(OS_WINDOWS) && !defined(FINAL_RELEASE)
        CrtMemoryStats memoryStats;
        CrtDumpMemoryStats(&memoryStats);
        PrintMemStats(memoryStats);
#endif
    }
    return result;
}
