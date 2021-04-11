#pragma once

#include "HAL/Generic/GenericPlatformMessageHandler.h"

#ifndef PLATFORM_LINUX
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
class FLinuxWindow;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ELinuxMessageType { // matching WM_XXX macros value
    Null,
    Create,
    Destroy,
    Move,
    Size,
    Activate,
    Focus,
    NoFocus,
    Enable,
    Paint,
    Close,
    Quit,
    Show,
    KeyDown,
    KeyUp,
    SysKeyDown,
    SysKeyUp,
    MouseMove,
    LButtonDown,
    LButtonUp,
    LButtonDblClick,
    RButtonDown,
    RButtonUp,
    RButtonDblClick,
    MButtonDown,
    MButtonUp,
    MButtonDblClick,
    MouseWheel,
    XButtonDown,
    XButtonUp,
    MouseHover,
    MouseLeave,
};
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FLinuxMessage {
    ELinuxMessageType Type;
    u32 LParam;
    u32 WParam;
    bool Handled;
};
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FLinuxPlatformMessageHandler : FGenericPlaformMessageHandler {
public:
    using EMessageType = ELinuxMessageType;
    using FMessage = FLinuxMessage;
    using FMessageHandler = TFunction<void(const FLinuxWindow&, FLinuxMessage*)>;

    static bool PumpMessages(FLinuxWindow* windowIFP);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
