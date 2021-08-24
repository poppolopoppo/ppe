#pragma once

#include "HAL/Generic/GenericPlatformMouse.h"

#ifndef PLATFORM_GLFW
#   error "invalid include for current platform"
#endif

#include "HAL/PlatformIncludes.h"

namespace PPE {
namespace Application {
class FGLFWWindow;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGLFWCursorType : int {
    _GLFW_DEFAULT_CURSOR = 0,
    _GLFW_ARROW_CURSOR = 0x00036001,
    _GLFW_IBEAM_CURSOR = 0x00036002,
    _GLFW_CROSSHAIR_CURSOR = 0x00036003,
    _GLFW_HAND_CURSOR = 0x00036004,
    _GLFW_HRESIZE_CURSOR = 0x00036005,
    _GLFW_VRESIZE_CURSOR = 0x00036006,

    AppStarting = _GLFW_DEFAULT_CURSOR,
    Arrow = _GLFW_ARROW_CURSOR,
    Cross = _GLFW_CROSSHAIR_CURSOR,
    Hand = _GLFW_HAND_CURSOR,
    Help = _GLFW_HAND_CURSOR,
    IBeam = _GLFW_IBEAM_CURSOR,
    Move = _GLFW_CROSSHAIR_CURSOR,
    No = _GLFW_DEFAULT_CURSOR,
    Pen = _GLFW_IBEAM_CURSOR,
    SizeAll = _GLFW_HRESIZE_CURSOR,
    SizeTop = _GLFW_VRESIZE_CURSOR,
    SizeTopLeft = _GLFW_VRESIZE_CURSOR,
    SizeTopRight = _GLFW_HRESIZE_CURSOR,
    SizeBottom = _GLFW_VRESIZE_CURSOR,
    SizeBottomLeft = _GLFW_VRESIZE_CURSOR,
    SizeBottomRight = _GLFW_HRESIZE_CURSOR,
    SizeLeft = _GLFW_HRESIZE_CURSOR,
    SizeRight = _GLFW_HRESIZE_CURSOR,
    WaitCursor = _GLFW_DEFAULT_CURSOR,

    Unknown = Arrow,
};
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGLFWPlatformMouse : FGenericPlatformMouse {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasMouse, true);

    using ECursorType = EGLFWCursorType;

    static ECursorType CursorType();
    static ECursorType SetCursorType(ECursorType type);
    static void ResetCursorType();

    static bool Visible();
    static bool SetVisible(bool value);

    static void ResetCapture();
    static void SetCapture(const FGLFWWindow& window);

    static bool ClientToScreen(const FGLFWWindow& window, int* x, int *y);
    static bool ScreenToClient(const FGLFWWindow& window, int* x, int *y);

    static void CenterCursorOnWindow(const FGLFWWindow& window);

    static FEventHandle SetupMessageHandler(FGLFWWindow& window, FMouseState* mouse);
    static void RemoveMessageHandler(FGLFWWindow& window, FEventHandle& handle);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
