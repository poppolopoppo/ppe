#include "stdafx.h"

#include "HAL/Linux/LinuxPlatformMouse.h"

#ifdef PLATFORM_LINUX

#include "Input/MouseState.h"
#include "HAL/Linux/LinuxWindow.h"
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
static ELinuxCursorType GLinuxCursorType = ELinuxCursorType::Default;
static FAtomicSpinLock& LinuxMouseCS_() {
    ONE_TIME_DEFAULT_INITIALIZE(FAtomicSpinLock, GInstance);
    return GInstance;
}
//----------------------------------------------------------------------------
static void MouseSetPosition_(const FLinuxWindow& window, const FLinuxMessage& msg, FMouseState* mouse) {
    const int cx = GET_X_LPARAM(msg.LParam);
    const int cy = GET_Y_LPARAM(msg.LParam);

    int sx = cx;
    int sy = cy;
    Verify(FLinuxPlatformMouse::ClientToScreen(window, &sx, &sy));

    const float rx = LinearStep(cx, 0, checked_cast<int>(window.Width()));
    const float ry = LinearStep(cy, 0, checked_cast<int>(window.Height()));

    mouse->SetPosition(sx, sy, cx, cy, rx, ry);
}
//----------------------------------------------------------------------------
static bool MouseMessageHandler_(const FLinuxWindow& window, const FLinuxMessage& msg, FMouseState* mouse) {
    Assert(mouse);
    Assert(window.Handle());

    switch (msg.Type) {
    case ELinuxMessageType::MouseMove:
        MouseSetPosition_(window, msg, mouse);
        return true;

    case ELinuxMessageType::MouseHover:
        mouse->SetInside(true);
        MouseSetPosition_(window, msg, mouse);
        return true;
    case ELinuxMessageType::MouseLeave:
        mouse->SetInside(false);
        return true;

    case ELinuxMessageType::LButtonUp:
        mouse->SetButtonUp(EMouseButton::Button0);
        return true;
    case ELinuxMessageType::LButtonDown:
        mouse->SetButtonDown(EMouseButton::Button0);
        return true;

    case ELinuxMessageType::RButtonUp:
        mouse->SetButtonUp(EMouseButton::Button1);
        return true;
    case ELinuxMessageType::RButtonDown:
        mouse->SetButtonDown(EMouseButton::Button1);
        return true;

    case ELinuxMessageType::MButtonUp:
        mouse->SetButtonUp(EMouseButton::Button2);
        return true;
    case ELinuxMessageType::MButtonDown:
        mouse->SetButtonDown(EMouseButton::Button2);
        return true;

    case ELinuxMessageType::XButtonUp:
        if (GET_KEYSTATE_WPARAM(msg.WParam) & MK_XBUTTON1)
            mouse->SetButtonUp(EMouseButton::Thumb0);
        if (GET_KEYSTATE_WPARAM(msg.WParam) & MK_XBUTTON2)
            mouse->SetButtonUp(EMouseButton::Thumb1);
        return true;
    case ELinuxMessageType::XButtonDown:
        if (GET_KEYSTATE_WPARAM(msg.WParam) & MK_XBUTTON1)
            mouse->SetButtonDown(EMouseButton::Thumb0);
        if (GET_KEYSTATE_WPARAM(msg.WParam) & MK_XBUTTON2)
            mouse->SetButtonDown(EMouseButton::Thumb1);
        return true;

    case ELinuxMessageType::MouseWheel:
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
auto FLinuxPlatformMouse::CursorType() -> ECursorType {
    const FAtomicSpinLock::FScope scopeLock(LinuxMouseCS_());
    return GLinuxCursorType;
}
//----------------------------------------------------------------------------
auto FLinuxPlatformMouse::SetCursorType(ECursorType type) ->ECursorType {
    const FAtomicSpinLock::FScope scopeLock(LinuxMouseCS_());
    const ECursorType oldCursor = GLinuxCursorType;
    VerifyRelease(::SetSystemCursor(::GetCursor(), ::DWORD(type)));
    return oldCursor;
}
//----------------------------------------------------------------------------
// https://www.dreamincode.net/forums/topic/58083-how-to-replace-cursor-pointer-with-setsystemcursor-from-busy-or-app/
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724947(v=vs.85).aspx
void FLinuxPlatformMouse::ResetCursorType() {
    const FAtomicSpinLock::FScope scopeLock(LinuxMouseCS_());
    // Reloads the system cursors. Set the uiParam parameter to zero and the pvParam parameter to NULL.
    ::SystemParametersInfo(SPI_SETCURSORS, 0, NULL, 0);
    GLinuxCursorType = ELinuxCursorType::Default;
}
//----------------------------------------------------------------------------
// https://docs.microsoft.com/fr-fr/windows/desktop/api/winuser/nf-winuser-getcursorinfo
bool FLinuxPlatformMouse::Visible() {
    ::CURSORINFO info;
    info.cbSize = sizeof(::CURSORINFO);
    VerifyRelease(::GetCursorInfo(&info));
    return (!!(info.flags & CURSOR_SHOWING));
}
//----------------------------------------------------------------------------
// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-showcursor
bool FLinuxPlatformMouse::SetVisible(bool value) {
    return (!!::ShowCursor(value));
}
//----------------------------------------------------------------------------
void FLinuxPlatformMouse::ResetCapture() {
    const FAtomicSpinLock::FScope scopeLock(LinuxMouseCS_());
    ::ReleaseCapture();
}
//----------------------------------------------------------------------------
void FLinuxPlatformMouse::SetCapture(const FLinuxWindow& window) {
    Assert(window.Handle());
    const FAtomicSpinLock::FScope scopeLock(LinuxMouseCS_());
    ::SetCapture(window.HandleWin32());
}
//----------------------------------------------------------------------------
bool FLinuxPlatformMouse::ClientToScreen(const FLinuxWindow& window, int* x, int *y) {
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
bool FLinuxPlatformMouse::ScreenToClient(const FLinuxWindow& window, int* x, int *y) {
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
void FLinuxPlatformMouse::CenterCursorOnWindow(const FLinuxWindow& window) {
    Assert(window.Handle());

    const FAtomicSpinLock::FScope scopeLock(LinuxMouseCS_());

    int x = checked_cast<int>(window.Width() / 2);
    int y = checked_cast<int>(window.Height() / 2);
    Verify(FLinuxPlatformMouse::ScreenToClient(window, &x, &y));

    ::SetCursorPos(x, y);
}
//----------------------------------------------------------------------------
FEventHandle FLinuxPlatformMouse::SetupMessageHandler(FLinuxWindow& window, FMouseState* mouse) {
    Assert(window.Handle());
    Assert(mouse);

    return window.OnMessageWin32().Add([mouse](const FLinuxWindow& w, FLinuxMessage* msg) {
        msg->Handled |= MouseMessageHandler_(w, *msg, mouse);
    });
}
//----------------------------------------------------------------------------
void FLinuxPlatformMouse::RemoveMessageHandler(FLinuxWindow& window, FEventHandle& handle) {
    Assert(window.Handle());
    Assert(handle);

    window.OnMessageWin32().Remove(handle);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_LINUX
