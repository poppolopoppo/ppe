﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#ifdef PLATFORM_GLFW

#include "HAL/GLFW/GLFWPlatformGamepad.h"

#include "HAL/GLFW/GLFWPlatformIncludes.h"
#include "HAL/TargetPlatform.h"

#include "Diagnostic/Logger.h"
#include "Input/Device/GamepadState.h"
#include "Maths/PackingHelpers.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FGLFWPlatformGamepad::Poll(FControllerStates* gamepads) {
    bool hasConnectedGamepad = false;

    forrange(controllerId, 0, static_cast<FControllerId>(MaxNumGamepad)) {
        FGamepadState& gamepad = (*gamepads)[controllerId];
        hasConnectedGamepad |= Poll(controllerId, &gamepad);
    }

    return hasConnectedGamepad;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformGamepad::Poll(FControllerId index, FGamepadState* gamepad) {
    Assert(index < static_cast<int>(MaxNumGamepad));
    Assert(gamepad);

    if (glfwJoystickPresent(index)) {
        if (glfwJoystickIsGamepad(index)) {
            gamepad->SetStatus(index, true);

            return true;
        }
        else {

        }
    }

    gamepad->SetStatus(index, false);
    return false;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformGamepad::Rumble(FControllerId index, float left, float right) {
    Unused(index);
    Unused(left);
    Unused(right);
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_GLFW
