#pragma once

#include "HAL/Generic/GenericPlatformKeyboard.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
FWD_REFPTR(WindowsWindow);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FWindowsPlatformKeyboard : FGenericPlatformKeyboard {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasKeyboard, true);

    static FEventHandle SetupMessageHandler(FWindowsWindow& window, FKeyboardState* keyboard);
    static void RemoveMessageHandler(FWindowsWindow& window, FEventHandle& handle);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
