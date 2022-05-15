#pragma once

#include "Application_fwd.h"

namespace PPE {
class FEventHandle;
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericCursorType {
    Invisible = 0,
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
    Unknown = Arrow,
};
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericPlatformMouse {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasMouse, false);

    using ECursorType = EGenericCursorType;

    static ECursorType SystemCursor() = delete;
    static ECursorType SetSystemCursor(ECursorType type) = delete;
    static void ResetSystemCursor() = delete;

    static void SetWindowCursor(const FGenericWindow& window) = delete;

    static bool Visible() = delete;
    static bool SetVisible(bool value) = delete;

    static void ResetCapture() = delete;
    static void SetCapture(const FGenericWindow& window) = delete;

    static bool ClientToScreen(const FGenericWindow& window, int* x, int *y) = delete;
    static bool ScreenToClient(const FGenericWindow& window, int* x, int *y) = delete;

    static void CenterCursorOnScreen(const FGenericWindow& window) = delete;
    static void SetCursorPosition(int screenX, int screenY) = delete;

    static FEventHandle SetupMessageHandler(FGenericWindow& window, FMouseState* mouse) = delete;
    static void RemoveMessageHandler(FGenericWindow& window, FEventHandle& handle) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
