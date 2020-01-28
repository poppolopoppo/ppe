#pragma once

#include "HAL/Generic/GenericPlatformLaunch.h"

#ifndef PLATFORM_LINUX
#   error "invalid include for current platform"
#endif

#include "Diagnostic/CurrentProcess.h"
#include "HAL/PlatformIncludes.h"
#include "HAL/Linux/resource.h"

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

    FGenericPlatformLaunch::OnPlatformLaunch(hInstance, nCmdShow, filename, argc, argv);
    FCurrentProcess::Get().SetAppIcon(IDI_WINDOW_ICON);

    using application_type = FGenericPlatformLaunch::application_type;
    const int exitCode = FGenericPlatformLaunch::RunApplication<application_type>();

    FGenericPlatformLaunch::OnPlatformShutdown();

    return exitCode;
}
