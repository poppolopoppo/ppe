#include "stdafx.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformGamepad.h"

#include "Input/GamepadState.h"
#include "Maths/PackingHelpers.h"

// #TODO : use libsdl2

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FLinuxPlatformGamepad::Poll(FControllerStates* gamepads) {
    UNUSED(gamepads);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformGamepad::Poll(FControllerId index, FGamepadState* gamepad) {
    UNUSED(index);
    UNUSED(gamepad);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformGamepad::Rumble(FControllerId index, float left, float right) {
    UNUSED(index);
    UNUSED(left);
    UNUSED(right);
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_LINUX
