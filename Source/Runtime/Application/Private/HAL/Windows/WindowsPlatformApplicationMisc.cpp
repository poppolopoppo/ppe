// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformApplicationMisc.h"

#include "Color/Color.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformIncludes.h"
#include "HAL/PlatformProcess.h"
#include "HAL/Windows/LastError.h"
#include "HAL/Windows/WindowsPlatformGamepad.h"
#include "HAL/Windows/WindowsPlatformNotification.h"
#include "HAL/Windows/WindowsWindow.h"

#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATION_API, Application)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FWindowsPlatformApplicationMisc::Start() {
    FWindowsPlatformGamepad::Start();
    FWindowsWindow::Start();
    FWindowsPlatformNotification::Start();
}
//----------------------------------------------------------------------------
void FWindowsPlatformApplicationMisc::Shutdown() {
    FWindowsPlatformNotification::Shutdown();
    FWindowsWindow::Shutdown();
    FWindowsPlatformGamepad::Shutdown();
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
    bool success = true;

    ::PROCESS_DPI_AWARENESS CurrentAwareness = PROCESS_DPI_UNAWARE;
    ::GetProcessDpiAwareness(NULL, &CurrentAwareness);

    if (CurrentAwareness != PROCESS_PER_MONITOR_DPI_AWARE) {
        LOG(Application, Info, L"setting application aware of per monitor DPI");
        if (not SUCCEEDED(::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE))) {
            LOG_LASTERROR(Application, L"SetProcessDpiAwareness");
            success = false;
        }
    }

    return success;
}
//----------------------------------------------------------------------------
int FWindowsPlatformApplicationMisc::ApplyDPIScale(int pixels, u32 dpi) {
    return ::MulDiv(pixels, static_cast<int>(dpi), DefaultScreenDPI);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
