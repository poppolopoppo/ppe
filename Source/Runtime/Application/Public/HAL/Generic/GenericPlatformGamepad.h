#pragma once

#include "Application_fwd.h"

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

    static bool Poll(FGenericWindow& window, FControllerId index, FGamepadState* state) = delete;
    static bool Rumble(FGenericWindow& window, FControllerId index, float left, float right) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE