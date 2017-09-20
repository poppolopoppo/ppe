#include "stdafx.h"

#include "Core/Core.h"

#include "Core.RTTI/RTTI.h"
#include "Core.Serialize/Serialize.h"
#include "Core.Graphics/Graphics.h"
#include "Core.Lattice/Lattice.h"
#include "Core.Network/Network.h"
#include "Core.Pixmap/Pixmap.h"
#include "Core.ContentPipeline/ContentPipeline.h"
#include "Core.Application/Application.h"

#include "RobotApp.h"

typedef Core::ContentGenerator::FRobotApp application_type;

#ifdef PLATFORM_WINDOWS
#   define CORE_RESOURCES 1
#else
#   define CORE_RESOURCES 0
#endif

#if defined(PLATFORM_WINDOWS) && CORE_RESOURCES
//  Retrieves application icon for windows
#   include "Core/Misc/Platform_Windows.h"
#   include "resource.h"
#endif

#include "Core/Diagnostic/CurrentProcess.h"

template <typename _Application>
static int Bootstrap(void *appHandle, int nShowCmd, const wchar_t* filename, int argc, const wchar_t**argv) {
    using namespace Core;

    const Application::FApplicationContext appContext;

    const Core::FCoreModule moduleCore{ appHandle, nShowCmd, filename, size_t(argc), argv };
    const RTTI::FRTTIModule moduleRTTI;
    const Serialize::FSerializeModule moduleSerialize;
    const Graphics::FGraphicsModule moduleGraphics;
    const Lattice::FLatticeModule moduleLattice;
    const Network::FNetworkModule moduleNetwork;
    const Pixmap::FPixmapModule modulePixmap;
    const ContentPipeline::FContentPipelineModule moduleContentPipeline;
    const Application::FApplicationModule moduleApplication;

#if defined(PLATFORM_WINDOWS) && CORE_RESOURCES
    FCurrentProcess::Instance().SetAppIcon(IDI_WINDOW_ICON);
#endif

    _Application app;
    return Application::LaunchApplication(appContext, &app);
}

#ifdef PLATFORM_WINDOWS
#   include "Core/Misc/Platform_Windows.h"
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
