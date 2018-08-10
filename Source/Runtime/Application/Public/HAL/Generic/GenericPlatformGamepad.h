#pragma once

#include "Application.h"

#include "HAL/TargetPlatform.h"

#include "Input/GamepadButton.h"
#include "Input/GamepadState.h"

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

    static bool Poll(FControllerId index, FGamepadState* state) = delete;
    static bool Rumble(FControllerId index, float left, float right) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
