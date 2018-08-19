#pragma once

#include "Application_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGenericPlatformKeyboard {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasKeyboard, false);

    static bool Poll(FGenericWindow& window, FKeyboardState* keyboard) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
