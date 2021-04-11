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
enum class ELinuxCursorType {
    AppStarting,
    Arrow,
    Cross,
    Hand,
    Help,
    IBeam,
    Move,
    No,
    Pen,
    SizeAll,
    SizeTop,
    SizeTopLeft,
    SizeTopRight,
    SizeBottom,
    SizeBottomLeft,
    SizeBottomRight,
    SizeLeft,
    SizeRight,
    WaitCursor,
    Default,
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
