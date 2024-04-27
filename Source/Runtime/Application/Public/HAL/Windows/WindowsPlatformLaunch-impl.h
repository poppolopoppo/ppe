
#include "HAL/Windows/WindowsPlatformLaunch.h"
#include "HAL/Generic/GenericPlatformLaunch-impl.h"

#include "Diagnostic/CurrentProcess.h"
#include "HAL/Windows/resource.h"

#include <shellapi.h>
#include <tchar.h>

// to get marquee progress bar working (see WindowsPlatformDialog.cpp)
// https://stackoverflow.com/questions/5681467/pbs-marquee-progressbar-winapi
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int APIENTRY wWinMain(
    _In_ HINSTANCE      hInstance,
    _In_opt_ HINSTANCE  hPrevInstance,
    _In_ LPWSTR         lpCmdLine,
    _In_ int            nCmdShow) {

    using namespace PPE;
    using namespace PPE::Application;

    Unused(hPrevInstance);
    Unused(lpCmdLine);

    int argc = __argc;
    wchar_t* const* argv = __wargv;

    const wchar_t* filename = argv[0];
    argv = &argv[1];
    argc--;

    int exitCode = 0;

    FWindowsPlatformLaunch::OnPlatformLaunch(hInstance, nCmdShow, filename, argc, argv);
    FCurrentProcess::Get().SetAppIcon(IDI_WINDOW_ICON);
    {
        FApplicationDomain appDomain;
        appDomain.LoadDependencies();

        exitCode  = FWindowsPlatformLaunch::RunApplication<PPE_APPLICATIONLAUNCH_TYPE>(appDomain);

        appDomain.UnloadDependencies();
    }

    FWindowsPlatformLaunch::OnPlatformShutdown();

    return exitCode;
}
