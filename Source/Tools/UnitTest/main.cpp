#include "stdafx.h"

#include "TestApp.h"

typedef Core::Test::FTestApp application_type;

#include "Core/Core.h"
#include "Core.Network/Network.h"
#include "Core.RTTI/RTTI.h"
#include "Core.Serialize/Serialize.h"
#include "Core.Pixmap/Pixmap.h"
#include "Core.Application/Application.h"

#ifdef PLATFORM_WINDOWS
#   define CORE_RESOURCES 1
#else
#   define CORE_RESOURCES 0
#endif

#if defined(PLATFORM_WINDOWS) && CORE_RESOURCES
//  Retrieves application icon for windows
#   include "Core/HAL/PlatformIncludes.h"
#   include "resource.h"
#endif

#include "Core/Diagnostic/CurrentProcess.h"

template <typename _Application>
static int Bootstrap(void *appHandle, int nShowCmd, const wchar_t* filename, int argc, const wchar_t**argv) {
    using namespace Core;

    const Application::FApplicationContext appContext;

    const Core::FCoreModule moduleCore{ appHandle, nShowCmd, filename, size_t(argc), argv };
    const Core::Network::FNetworkModule moduleNetwork;
    const Core::RTTI::FRTTIModule moduleRTTI;
    const Core::Serialize::FSerializeModule moduleSerialize;
    const Core::Pixmap::FPixmapModule modulePixmap;
    const Core::Application::FApplicationModule moduleApplication;

#if defined(PLATFORM_WINDOWS) && CORE_RESOURCES
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
