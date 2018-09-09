#include "stdafx.h"

#include "TestApp.h"

#include "Diagnostic/CurrentProcess.h"

#if defined(PLATFORM_WINDOWS)
//  Retrieves application icon for windows
#   include "HAL/PlatformIncludes.h"
#   include "resource.h"
#   define PPE_RESOURCES 1
#else
#   define PPE_RESOURCES 0
#endif

typedef PPE::Test::FTestApp application_type;

template <typename _Application>
static int Bootstrap(void* appHandle, int showCmd, const wchar_t* filename, int argc, const wchar_t** argv) {
    using namespace PPE;

    const Application::FApplicationContext appContext;

    PPE::FModuleManager manager{ appHandle, showCmd, filename, size_t(argc), argv };
    PPE_STATICMODULES_STARTUP startupStaticModules{ manager };

#if PPE_RESOURCES
    FCurrentProcess::Get().SetAppIcon(IDI_WINDOW_ICON);
#endif

    _Application app;
    return Application::LaunchApplication(appContext, &app);
}

#ifdef PLATFORM_WINDOWS
#   include <shellapi.h>
#   include <tchar.h>
int APIENTRY wWinMain(
    _In_ HINSTANCE      hInstance,
    _In_opt_ HINSTANCE  hPrevInstance,
    _In_ LPWSTR         lpCmdLine,
    _In_ int            nCmdShow ) {
    UNUSED(hPrevInstance);
    UNUSED(lpCmdLine);
    int argc = __argc;
    wchar_t* const* argv = __wargv;
#else
int main(int argc, const wchar_t* argv[]) {
    const void* hInstance = nullptr;
    const int nCmdShow = 0;
#endif

    const wchar_t* filename = argv[0];
    argv = &argv[1];
    argc--;

    return Bootstrap<application_type>(
        hInstance, nCmdShow,
        filename,
        argc, const_cast<const wchar_t**>(&argv[0]) );
}
