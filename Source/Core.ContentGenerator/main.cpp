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
#   include <windows.h>
#   include "resource.h"
#endif

#include "Core/Diagnostic/CurrentProcess.h"

template <typename _Application>
static int Bootstrap(void *appHandle, int nShowCmd, int argc, const wchar_t**argv) {
    using namespace Core;

    const Application::FApplicationContext appContext;

    const Core::CoreStartup startupCore{ appHandle, nShowCmd, size_t(argc), argv };
    const RTTI::RTTIStartup startupRTTI;
    const Serialize::SerializeStartup startupSerialize;
    const Graphics::GraphicsStartup startupGraphics;
    const Lattice::LatticeStartup startupLattice;
    const Network::NetworkStartup startupNetwork;
    const Pixmap::PixmapStartup startupPixmap;
    const ContentPipeline::ContentPipelineStartup startupContentPipeline;
    const Application::FApplicationStartup startupApplication;

#if defined(PLATFORM_WINDOWS) && CORE_RESOURCES
    FCurrentProcess::Instance().SetAppIcon(IDI_WINDOW_ICON);
#endif

    _Application app;
    return Application::LaunchApplication(appContext, &app);
}

#ifdef PLATFORM_WINDOWS
#   include <Windows.h>
#   include <shellapi.h>
#   include <tchar.h>
int APIENTRY _tWinMain(
    _In_ HINSTANCE      hInstance,
    _In_opt_ HINSTANCE  hPrevInstance,
    _In_ LPWSTR         lpCmdLine,
    _In_ int            nCmdShow ) {
    UNUSED(hPrevInstance);
#else
int main(int argc, const wchar_t* argv[]) {
#endif

#ifdef PLATFORM_WINDOWS
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
    result = Bootstrap<application_type>(hInstance, nCmdShow, argc, const_cast<const wchar_t**>(&argv[0]));
    return result;
}
