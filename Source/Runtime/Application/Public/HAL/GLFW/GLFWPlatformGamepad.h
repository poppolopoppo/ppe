#pragma once

#include "HAL/Generic/GenericPlatformGamepad.h"

#ifndef PLATFORM_GLFW
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGLFWPlatformGamepad : FGenericPlatformGamepad {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasGamepad, true);
    STATIC_CONST_INTEGRAL(size_t, MaxNumGamepad, 8);

    using FControllerId = int;
    using FControllerStates = TStaticArray<FGamepadState, MaxNumGamepad>;

    static bool Poll(FControllerStates* gamepads);
    static bool Poll(FControllerId index, FGamepadState* gamepad);
    static bool Rumble(FControllerId index, float left, float right);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
