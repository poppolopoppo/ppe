#pragma once

#include "Application_fwd.h"

namespace PPE {
class FEventHandle;
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericPlatformKeyboard {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasKeyboard, false);

    static FEventHandle SetupMessageHandler(FGenericWindow& window, FKeyboardState* keyboard) = delete;
    static void RemoveMessageHandler(FGenericWindow& window, FEventHandle& handle) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
