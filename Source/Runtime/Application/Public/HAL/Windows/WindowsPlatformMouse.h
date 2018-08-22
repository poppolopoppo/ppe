#pragma once

#include "HAL/Generic/GenericPlatformMouse.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include for current platform"
#endif

#include "HAL/PlatformIncludes.h"

namespace PPE {
namespace Application {
class FWindowsWindow;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-setsystemcursor
enum class EWindowsCursorType : ::DWORD {
    AppStarting         = OCR_APPSTARTING,
    Arrow               = OCR_NORMAL,
    Cross               = OCR_CROSS,
    Hand                = OCR_HAND,
    Help                = /*OCR_HELP*/32651,
    IBeam               = OCR_IBEAM,
    Move                = OCR_SIZEALL,
    No                  = OCR_NO,
    Pen                 = /*OCR_PEN*/OCR_UP,
    SizeAll             = OCR_SIZEALL,
    SizeTop             = OCR_SIZENS,
    SizeTopLeft         = OCR_SIZENWSE,
    SizeTopRight        = OCR_SIZENESW,
    SizeBottom          = OCR_SIZENS,
    SizeBottomLeft      = OCR_SIZENESW,
    SizeBottomRight     = OCR_SIZENWSE,
    SizeLeft            = OCR_SIZEWE,
    SizeRight           = OCR_SIZEWE,
    WaitCursor          = OCR_WAIT,
    Default             = Arrow,
};
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FWindowsPlatformMouse : FGenericPlatformMouse {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasMouse, true);

    using ECursorType = EWindowsCursorType;

    static ECursorType CursorType();
    static ECursorType SetCursorType(ECursorType type);
    static void ResetCursorType();

    static bool Visible();
    static bool SetVisible(bool value);

    static void ResetCapture();
    static void SetCapture(const FWindowsWindow& window);

    static bool ClientToScreen(const FWindowsWindow& window, int* x, int *y);
    static bool ScreenToClient(const FWindowsWindow& window, int* x, int *y);

    static void CenterCursorOnWindow(const FWindowsWindow& window);

    static FEventHandle SetupMessageHandler(FWindowsWindow& window, FMouseState* mouse);
    static void RemoveMessageHandler(FWindowsWindow& window, FEventHandle& handle);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
