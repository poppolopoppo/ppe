#pragma once

#include "HAL/Generic/GenericPlatformMessageHandler.h"

#ifndef PLATFORM_GLFW
#   error "invalid include for current platform"
#endif

#include "Maths/ScalarVector.h"
#include "Misc/Function.h"

namespace PPE {
namespace Application {
class FGLFWWindow;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGLFWMessageKeyboard {
    enum EEventType {
        KeyDown,
        KeyUp,
    };
    EEventType Event;
    EKeyboardKey Key;
};
using FGLFWMessageKeyboardEvent = TFunction<void(FGLFWWindow& window, const FGLFWMessageKeyboard& event)>;
//----------------------------------------------------------------------------
struct FGLFWMessageMouse {
    enum EEventType {
        CursorMove,
        ButtonDown,
        ButtonUp,
        Scroll,
    };
    EEventType Event;
    EMouseButton Button;
    double2 Coords;
};
using FGLFWMessageMouseEvent = TFunction<void(FGLFWWindow& window, const FGLFWMessageMouse& event)>;
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGLFWPlatformMessageHandler : FGenericPlaformMessageHandler {
public:
    static bool PumpGlobalMessages();
    static bool PumpMessages(FGLFWWindow& window);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
