#include "stdafx.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformMouse.h"

#include "Input/MouseState.h"
#include "HAL/Windows/WindowsWindow.h"
#include "HAL/PlatformIncludes.h"
#include "Maths/MathHelpers.h"
#include "Thread/AtomicSpinLock.h"

#include <windowsx.h>

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static EWindowsCursorType GWindowsCursorType = EWindowsCursorType::Default;
static FAtomicSpinLock& WindowsMouseCS_() {
    ONE_TIME_DEFAULT_INITIALIZE(FAtomicSpinLock, GInstance);
    return GInstance;
}
//----------------------------------------------------------------------------
static void MouseSetPosition_(const FWindowsWindow& window, const FWindowsMessage& msg, FMouseState* mouse) {
    const int cx = GET_X_LPARAM(msg.LParam);
    const int cy = GET_Y_LPARAM(msg.LParam);

    int sx = cx;
    int sy = cy;
    Verify(FWindowsPlatformMouse::ClientToScreen(window, &sx, &sy));

    const float rx = LinearStep(cx, 0, checked_cast<int>(window.Width()));
    const float ry = LinearStep(cy, 0, checked_cast<int>(window.Height()));

    mouse->SetPosition(sx, sy, cx, cy, rx, ry);
}
//----------------------------------------------------------------------------
static bool MouseMessageHandler_(const FWindowsWindow& window, const FWindowsMessage& msg, FMouseState* mouse) {
    Assert(mouse);
    Assert(window.Handle());

    switch (msg.Type) {
    case EWindowsMessageType::MouseMove:
        MouseSetPosition_(window, msg, mouse);
        return true;

    case EWindowsMessageType::MouseHover:
        mouse->SetInside(true);
        MouseSetPosition_(window, msg, mouse);
        return true;
    case EWindowsMessageType::MouseLeave:
        mouse->SetInside(false);
        return true;

    case EWindowsMessageType::LButtonUp:
        mouse->SetButtonUp(EMouseButton::Button0);
        return true;
    case EWindowsMessageType::LButtonDown:
        mouse->SetButtonDown(EMouseButton::Button0);
        return true;

    case EWindowsMessageType::RButtonUp:
        mouse->SetButtonUp(EMouseButton::Button1);
        return true;
    case EWindowsMessageType::RButtonDown:
        mouse->SetButtonDown(EMouseButton::Button1);
        return true;

    case EWindowsMessageType::MButtonUp:
        mouse->SetButtonUp(EMouseButton::Button2);
        return true;
    case EWindowsMessageType::MButtonDown:
        mouse->SetButtonDown(EMouseButton::Button2);
        return true;

    case EWindowsMessageType::XButtonUp:
        if (GET_KEYSTATE_WPARAM(msg.WParam) & MK_XBUTTON1)
            mouse->SetButtonUp(EMouseButton::Thumb0);
        if (GET_KEYSTATE_WPARAM(msg.WParam) & MK_XBUTTON2)
            mouse->SetButtonUp(EMouseButton::Thumb1);
        return true;
    case EWindowsMessageType::XButtonDown:
        if (GET_KEYSTATE_WPARAM(msg.WParam) & MK_XBUTTON1)
            mouse->SetButtonDown(EMouseButton::Thumb0);
        if (GET_KEYSTATE_WPARAM(msg.WParam) & MK_XBUTTON2)
            mouse->SetButtonDown(EMouseButton::Thumb1);
        return true;

    case EWindowsMessageType::MouseWheel:
        mouse->SetWheelDelta(GET_WHEEL_DELTA_WPARAM(msg.WParam));
        return true;

    default:
        return false; // unhandled
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
auto FWindowsPlatformMouse::CursorType() -> ECursorType {
    const FAtomicSpinLock::FScope scopeLock(WindowsMouseCS_());
    return GWindowsCursorType;
}
//----------------------------------------------------------------------------
auto FWindowsPlatformMouse::SetCursorType(ECursorType type) ->ECursorType {
    const FAtomicSpinLock::FScope scopeLock(WindowsMouseCS_());
    const ECursorType oldCursor = GWindowsCursorType;
    VerifyRelease(::SetSystemCursor(::GetCursor(), ::DWORD(type)));
    return oldCursor;
}
//----------------------------------------------------------------------------
// https://www.dreamincode.net/forums/topic/58083-how-to-replace-cursor-pointer-with-setsystemcursor-from-busy-or-app/
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724947(v=vs.85).aspx
void FWindowsPlatformMouse::ResetCursorType() {
    const FAtomicSpinLock::FScope scopeLock(WindowsMouseCS_());
    // Reloads the system cursors. Set the uiParam parameter to zero and the pvParam parameter to NULL.
    ::SystemParametersInfo(SPI_SETCURSORS, 0, NULL, 0);
    GWindowsCursorType = EWindowsCursorType::Default;
}
//----------------------------------------------------------------------------
// https://docs.microsoft.com/fr-fr/windows/desktop/api/winuser/nf-winuser-getcursorinfo
bool FWindowsPlatformMouse::Visible() {
    ::CURSORINFO info;
    info.cbSize = sizeof(::CURSORINFO);
    VerifyRelease(::GetCursorInfo(&info));
    return (!!(info.flags & CURSOR_SHOWING));
}
//----------------------------------------------------------------------------
// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-showcursor
bool FWindowsPlatformMouse::SetVisible(bool value) {
    return (!!::ShowCursor(value));
}
//----------------------------------------------------------------------------
void FWindowsPlatformMouse::ResetCapture() {
    const FAtomicSpinLock::FScope scopeLock(WindowsMouseCS_());
    ::ReleaseCapture();
}
//----------------------------------------------------------------------------
void FWindowsPlatformMouse::SetCapture(const FWindowsWindow& window) {
    Assert(window.Handle());
    const FAtomicSpinLock::FScope scopeLock(WindowsMouseCS_());
    ::SetCapture(window.HandleWin32());
}
//----------------------------------------------------------------------------
bool FWindowsPlatformMouse::ClientToScreen(const FWindowsWindow& window, int* x, int *y) {
    Assert(window.Handle());
    Assert(x);
    Assert(y);

    ::POINT p;
    p.x = *x;
    p.y = *y;
    const bool succeed = ::ClientToScreen(window.HandleWin32(), &p);

    *x = p.x;
    *y = p.y;

    return succeed;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformMouse::ScreenToClient(const FWindowsWindow& window, int* x, int *y) {
    Assert(window.Handle());
    Assert(x);
    Assert(y);

    ::POINT p;
    p.x = *x;
    p.y = *y;
    const bool succeed = ::ScreenToClient(window.HandleWin32(), &p);

    *x = p.x;
    *y = p.y;

    return succeed;
}
//----------------------------------------------------------------------------
void FWindowsPlatformMouse::CenterCursorOnWindow(const FWindowsWindow& window) {
    Assert(window.Handle());

    const FAtomicSpinLock::FScope scopeLock(WindowsMouseCS_());

    int x = checked_cast<int>(window.Width() / 2);
    int y = checked_cast<int>(window.Height() / 2);
    Verify(FWindowsPlatformMouse::ScreenToClient(window, &x, &y));

    ::SetCursorPos(x, y);
}
//----------------------------------------------------------------------------
FEventHandle FWindowsPlatformMouse::SetupMessageHandler(FWindowsWindow& window, FMouseState* mouse) {
    Assert(window.Handle());
    Assert(mouse);

    return window.OnMessageWin32().Add([mouse](const FWindowsWindow& w, FWindowsMessage* msg) {
        msg->Handled |= MouseMessageHandler_(w, *msg, mouse);
    });
}
//----------------------------------------------------------------------------
void FWindowsPlatformMouse::RemoveMessageHandler(FWindowsWindow& window, FEventHandle& handle) {
    Assert(window.Handle());
    Assert(handle);

    window.OnMessageWin32().Remove(handle);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
