﻿
#include "HAL/Windows/WindowsPlatformLaunch.h"
#include "HAL/Generic/GenericPlatformLaunch-impl.h"

#include "Diagnostic/CurrentProcess.h"
#include "HAL/PlatformIncludes.h"
#include "HAL/Windows/resource.h"

#include <shellapi.h>
#include <tchar.h>

int APIENTRY wWinMain(
    _In_ HINSTANCE      hInstance,
    _In_opt_ HINSTANCE  hPrevInstance,
    _In_ LPWSTR         lpCmdLine,
    _In_ int            nCmdShow) {
    UNUSED(hPrevInstance);
    UNUSED(lpCmdLine);

    int argc = __argc;
    wchar_t* const* argv = __wargv;

    const wchar_t* filename = argv[0];
    argv = &argv[1];
    argc--;

    using namespace PPE;
    using namespace PPE::Application;

    FWindowsPlatformLaunch::OnPlatformLaunch(hInstance, nCmdShow, filename, argc, argv);

    FCurrentProcess::Get().SetAppIcon(IDI_WINDOW_ICON);

    const int exitCode = FWindowsPlatformLaunch::RunApplication<PPE_APPLICATIONLAUNCH_TYPE>();

    FWindowsPlatformLaunch::OnPlatformShutdown();

    return exitCode;
}