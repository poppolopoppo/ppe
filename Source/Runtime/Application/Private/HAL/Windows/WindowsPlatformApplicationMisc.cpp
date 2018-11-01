#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformApplicationMisc.h"

#ifdef PLATFORM_WINDOWS

#include "Color/Color.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformIncludes.h"
#include "HAL/PlatformProcess.h"
#include "HAL/Windows/LastError.h"
#include "HAL/Windows/WindowsPlatformNotification.h"
#include "HAL/Windows/WindowsWindow.h"

#include <ShellScalingApi.h>

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATION_API, Application)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FWindowsPlatformApplicationMisc::Start() {
    Verify(SUCCEEDED(::CoInitialize(NULL)));
    FWindowsWindow::Start();
    FWindowsPlatformNotification::Start();
}
//----------------------------------------------------------------------------
void FWindowsPlatformApplicationMisc::Shutdown() {
    FWindowsPlatformNotification::Shutdown();
    FWindowsWindow::Shutdown();
    ::CoUninitialize();
}
//----------------------------------------------------------------------------
bool FWindowsPlatformApplicationMisc::PickScreenColorAt(int x, int y, FColor* color) {
    Assert(color);

    bool succeed = true;

    ::HDC const hDC = ::GetDC(NULL);
    Assert(hDC);

    const ::COLORREF cref = ::GetPixel(hDC, x, y);
    if (Unlikely(cref == CLR_INVALID)) {
        succeed = false;
    }
    else {
        color->R = GetRValue(cref);
        color->G = GetGValue(cref);
        color->B = GetBValue(cref);
        color->A = 0xFF;
    }

    ::ReleaseDC(NULL, hDC);

    return succeed;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformApplicationMisc::PickScreenColorUnderMouse(FColor* color) {
    ::POINT mouse;
    return (::GetCursorPos(&mouse)
        ? PickScreenColorAt(mouse.x, mouse.y, color)
        : false );
}
//----------------------------------------------------------------------------
void FWindowsPlatformApplicationMisc::PreventScreenSaver() {
    // send fake mouse event
    ::INPUT Input;
    Input.type = INPUT_MOUSE;
    Input.mi.dx = 0;
    Input.mi.dy = 0;
    Input.mi.mouseData = 0;
    Input.mi.dwFlags = MOUSEEVENTF_MOVE;
    Input.mi.time = 0;
    Input.mi.dwExtraInfo = 0;
    ::SendInput(1, &Input, sizeof(INPUT));
}
//----------------------------------------------------------------------------
bool FWindowsPlatformApplicationMisc::SetHighDPIAwareness() {
    ::HMODULE const hShcore = FPlatformProcess::AttachToDynamicLibrary(L"Shcore.dll");
    if (NULL == hShcore)
        return false;

    typedef ::HRESULT (STDAPICALLTYPE *FSetProcessDpiAwareness)(
        _In_ ::PROCESS_DPI_AWARENESS value);
    typedef ::HRESULT (STDAPICALLTYPE *FGetProcessDpiAwareness)(
        _In_opt_ ::HANDLE hprocess,
        _Out_ ::PROCESS_DPI_AWARENESS *value);
    typedef ::HRESULT (STDAPICALLTYPE *FGetDpiForMonitor)(
        _In_ ::HMONITOR hmonitor,
        _In_ ::MONITOR_DPI_TYPE dpiType,
        _Out_ ::UINT *dpiX,
        _Out_ ::UINT *dpiY);

    auto hSetProcessDpiAwareness = (FSetProcessDpiAwareness)FPlatformProcess::DynamicLibraryFunction(hShcore, "SetProcessDpiAwareness");
    auto hGetProcessDpiAwareness = (FGetProcessDpiAwareness)FPlatformProcess::DynamicLibraryFunction(hShcore, "GetProcessDpiAwareness");
    auto hGetDpiForMonitor = (FGetDpiForMonitor)FPlatformProcess::DynamicLibraryFunction(hShcore, "GetDpiForMonitor");
    if (not (hSetProcessDpiAwareness && hGetProcessDpiAwareness && hGetDpiForMonitor)) {
        LOG(Application, Warning, L"failed to bind DPI awareness functions from Shcore.dll");
        return false;
    }

    bool success = true;

    ::PROCESS_DPI_AWARENESS CurrentAwareness = PROCESS_DPI_UNAWARE;
    hGetProcessDpiAwareness(NULL, &CurrentAwareness);

    if (CurrentAwareness != PROCESS_PER_MONITOR_DPI_AWARE) {
        LOG(Application, Info, L"setting application aware of per monitor DPI");
        if (not SUCCEEDED(hSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE))) {
            LOG_LASTERROR(Application, L"SetProcessDpiAwareness");
            success = false;
        }
    }

    FPlatformProcess::DetachFromDynamicLibrary(hShcore);

    return success;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
