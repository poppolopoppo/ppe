#pragma once

#include "HAL/Generic/GenericPlatformKeyboard.h"

#ifndef PLATFORM_LINUX
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
FWD_REFPTR(LinuxWindow);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FLinuxPlatformKeyboard : FGenericPlatformKeyboard {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasKeyboard, true);

    static FEventHandle SetupMessageHandler(FLinuxWindow& window, FKeyboardState* keyboard);
    static void RemoveMessageHandler(FLinuxWindow& window, FEventHandle& handle);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
