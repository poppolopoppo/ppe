#include "stdafx.h"

#include "TestApp.h"

typedef PPE::Test::FTestApp application_type;

#include "Core.h"
#include "Network.h"
#include "RTTI.h"
#include "Serialize.h"
#include "Pixmap.h"
#include "Application.h"

#ifdef PLATFORM_WINDOWS
#   define PPE_RESOURCES 1
#else
#   define PPE_RESOURCES 0
#endif

#if defined(PLATFORM_WINDOWS) && PPE_RESOURCES
//  Retrieves application icon for windows
#   include "HAL/PlatformIncludes.h"
#   include "resource.h"
#endif

#include "Diagnostic/CurrentProcess.h"

template <typename _Application>
static int Bootstrap(void *appHandle, int nShowCmd, const wchar_t* filename, int argc, const wchar_t**argv) {
    using namespace PPE;

    const Application::FApplicationContext appContext;

    const PPE::FCoreModule moduleCore{ appHandle, nShowCmd, filename, size_t(argc), argv };
    const PPE::Network::FNetworkModule moduleNetwork;
    const PPE::RTTI::FRTTIModule moduleRTTI;
    const PPE::Serialize::FSerializeModule moduleSerialize;
    const PPE::Pixmap::FPixmapModule modulePixmap;
    const PPE::Application::FApplicationModule moduleApplication;

#if defined(PLATFORM_WINDOWS) && PPE_RESOURCES
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
#else
int main(int argc, const wchar_t* argv[]) {
#endif

#ifdef PLATFORM_WINDOWS
    int argc = __argc;
    wchar_t* const* argv = __wargv;
#endif

    const wchar_t* filename = argv[0];
    argv = &argv[1];
    argc--;

    int result = 0;
    result = Bootstrap<application_type>(hInstance, nCmdShow, filename, argc, const_cast<const wchar_t**>(&argv[0]));
    return result;
}