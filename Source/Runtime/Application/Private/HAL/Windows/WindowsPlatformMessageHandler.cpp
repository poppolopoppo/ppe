// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformMessageHandler.h"

#include "HAL/Windows/WindowsWindow.h"
#include "HAL/PlatformIncludes.h"
#include "Misc/Function.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FWindowsPlatformMessageHandler::PumpGlobalMessages() {
    bool alive = true;

    ::MSG msg;
    if (::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        switch (LOWORD(msg.message)) {
        case WM_QUIT:
            alive = false;
            break;
        default:
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
            break;
        }
    }

    return alive;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformMessageHandler::PumpMessages(FWindowsWindow& window) {
    bool alive = true;

    ::HWND const hWnd = window.HandleWin32();

    ::MSG msg;
    while (::PeekMessageW(&msg, hWnd, 0, 0, PM_REMOVE)) {
        switch (LOWORD(msg.message)) {
        case WM_QUIT:
            // see FWindowsWindow::WindowProcWin32()#WM_DESTROY
            alive = false;
            break;

        default:
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
            break;
        }
    }

    return alive;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
