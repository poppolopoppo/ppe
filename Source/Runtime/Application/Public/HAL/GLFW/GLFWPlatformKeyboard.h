#pragma once

#include "HAL/Generic/GenericPlatformKeyboard.h"

#ifndef PLATFORM_GLFW
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
FWD_REFPTR(GLFWWindow);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGLFWPlatformKeyboard : FGenericPlatformKeyboard {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasKeyboard, true);

    static FEventHandle SetupMessageHandler(FGLFWWindow& window, FKeyboardState* keyboard);
    static void RemoveMessageHandler(FGLFWWindow& window, FEventHandle& handle);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
