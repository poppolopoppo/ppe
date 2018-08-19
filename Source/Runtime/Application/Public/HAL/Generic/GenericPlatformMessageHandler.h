#pragma once

#include "Application_fwd.h"

#include "Container/Vector.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericMessageType {
    Null                = 0x0000,
    Create              = 0x0001,
    Destroy             = 0x0002,
    Move                = 0x0003,
    Size                = 0x0005,
    Activate            = 0x0006,
    Focus               = 0x0007,
    NoFocus             = 0x0008,
    Enable              = 0x000A,
    Paint               = 0x000F,
    Close               = 0x0010,
    Quit                = 0x0012,
    Show                = 0x0018,
    KeyDown             = 0x0100,
    KeyUp               = 0x0101,
    SysKeyDown          = 0x0104,
    SysKeyUp            = 0x0105,
    MouseMove           = 0x0200,
    LButtonDown         = 0x0201,
    LButtonUp           = 0x0202,
    LButtonDblClick     = 0x0203,
    RButtonDown         = 0x0204,
    RButtonUp           = 0x0205,
    RButtonDblClick     = 0x0206,
    MButtonDown         = 0x0207,
    MButtonUp           = 0x0208,
    MButtonDblClick     = 0x0209,
    MouseWheel          = 0x020A,
    MouseHover          = 0x02A1,
    MouseLeave          = 0x02A3,
};
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericMessage {
    using FResult = long;

    EGenericMessageType Type;
};
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGenericPlaformMessageHandler {
public:
    using EMessageType = EGenericMessageType;
    using FMessage = FGenericMessage;
    using FDeferredMessages = VECTORINSITU(Message, FMessage, 8);

    static bool PumpMessages(FDeferredMessages* messages, FGenericWindow& window) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
