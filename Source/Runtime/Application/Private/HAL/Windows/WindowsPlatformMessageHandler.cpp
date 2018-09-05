#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformMessageHandler.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsWindow.h"
#include "HAL/PlatformIncludes.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FWindowsPlatformMessageHandler::PumpMessages(FWindowsWindow* windowIFP) {
    bool quit = false;

    ::MSG msg;
    if (nullptr == windowIFP) {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE | PM_QS_SENDMESSAGE)) {
            switch (LOWORD(msg.message)) {
            case WM_QUIT:
                quit = true;
                break;
            default:
                break;
            }
        }
    }
    else {
        ::HWND const hWnd = windowIFP->HandleWin32();
        Assert(hWnd);

        while (::PeekMessageW(&msg, hWnd, 0, 0, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);

            switch (LOWORD(msg.message)) {
            case WM_QUIT:
                quit = true;
                break;
            default:
                break;
            }
        }
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
