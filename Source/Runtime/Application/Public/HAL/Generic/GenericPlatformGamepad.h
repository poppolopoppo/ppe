#pragma once

#include "Application_fwd.h"

#include "Container/Array.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericPlatformGamepad {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasGamepad, false);
    STATIC_CONST_INTEGRAL(size_t, MaxNumGamepad, 0);

    using FControllerId = size_t;
    using FControllerStates = TArray<FGamepadState, MaxNumGamepad>;

    static bool Poll(FControllerStates* gamepads) = delete;
    static bool Poll(FControllerId index, FGamepadState* gamepad) = delete;
    static bool Rumble(FControllerId index, float left, float right) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
