#pragma once

#include "HAL/Generic/GenericPlatformMouse.h"

#ifndef PLATFORM_LINUX
#   error "invalid include for current platform"
#endif

#include "HAL/PlatformIncludes.h"

namespace PPE {
namespace Application {
class FLinuxWindow;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-setsystemcursor
enum class ELinuxCursorType : ::DWORD {
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
class PPE_APPLICATION_API FLinuxPlatformMouse : FGenericPlatformMouse {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasMouse, true);

    using ECursorType = ELinuxCursorType;

    static ECursorType CursorType();
    static ECursorType SetCursorType(ECursorType type);
    static void ResetCursorType();

    static bool Visible();
    static bool SetVisible(bool value);

    static void ResetCapture();
    static void SetCapture(const FLinuxWindow& window);

    static bool ClientToScreen(const FLinuxWindow& window, int* x, int *y);
    static bool ScreenToClient(const FLinuxWindow& window, int* x, int *y);

    static void CenterCursorOnWindow(const FLinuxWindow& window);

    static FEventHandle SetupMessageHandler(FLinuxWindow& window, FMouseState* mouse);
    static void RemoveMessageHandler(FLinuxWindow& window, FEventHandle& handle);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
