#include "stdafx.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformMouse.h"

#include "Input/MouseState.h"
#include "HAL/Linux/LinuxWindow.h"
#include "HAL/PlatformIncludes.h"
#include "Maths/MathHelpers.h"
#include "Thread/AtomicSpinLock.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
auto FLinuxPlatformMouse::CursorType() -> ECursorType {
    return ECursorType::Default;
}
//----------------------------------------------------------------------------
auto FLinuxPlatformMouse::SetCursorType(ECursorType type) -> ECursorType {
    UNUSED(type);
    return ECursorType::Default;
}
//----------------------------------------------------------------------------
void FLinuxPlatformMouse::ResetCursorType() {
}
//----------------------------------------------------------------------------
bool FLinuxPlatformMouse::Visible() {
    return true;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformMouse::SetVisible(bool value) {
    UNUSED(value);
    Assert(value);
    return true;
}
//----------------------------------------------------------------------------
void FLinuxPlatformMouse::ResetCapture() {
}
//----------------------------------------------------------------------------
void FLinuxPlatformMouse::SetCapture(const FLinuxWindow& window) {
    UNUSED(window);
}
//----------------------------------------------------------------------------
bool FLinuxPlatformMouse::ClientToScreen(const FLinuxWindow& window, int* x, int *y) {
    UNUSED(window);
    UNUSED(x);
    UNUSED(y);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformMouse::ScreenToClient(const FLinuxWindow& window, int* x, int *y) {
    UNUSED(window);
    UNUSED(x);
    UNUSED(y);
    return false;
}
//----------------------------------------------------------------------------
void FLinuxPlatformMouse::CenterCursorOnWindow(const FLinuxWindow& window) {
    UNUSED(window);
}
//----------------------------------------------------------------------------
FEventHandle FLinuxPlatformMouse::SetupMessageHandler(FLinuxWindow& window, FMouseState* mouse) {
    UNUSED(window);
    UNUSED(mouse);
    return FEventHandle{};
}
//----------------------------------------------------------------------------
void FLinuxPlatformMouse::RemoveMessageHandler(FLinuxWindow& window, FEventHandle& handle) {
    UNUSED(window);
    UNUSED(handle);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_LINUX
