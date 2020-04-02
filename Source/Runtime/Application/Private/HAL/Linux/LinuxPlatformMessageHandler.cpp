#include "stdafx.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformMessageHandler.h"

#include "HAL/Linux/LinuxWindow.h"
#include "HAL/PlatformIncludes.h"
#include "Misc/Function.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FLinuxPlatformMessageHandler::PumpMessages(FLinuxWindow* windowIFP) {
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

#endif //!PLATFORM_LINUX
