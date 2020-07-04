#pragma once

#include "HAL/Generic/GenericPlatformGamepad.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FWindowsPlatformGamepad : FGenericPlatformGamepad {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasGamepad, true);
    STATIC_CONST_INTEGRAL(size_t, MaxNumGamepad, 8);

    using FControllerId = FGenericPlatformGamepad::FControllerId;
    using FControllerStates = TStaticArray<FGamepadState, MaxNumGamepad>;

    static bool Poll(FControllerStates* gamepads);
    static bool Poll(FControllerId index, FGamepadState* gamepad);
    static bool Rumble(FControllerId index, float left, float right);

    static void Start();
    static void Shutdown();

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
