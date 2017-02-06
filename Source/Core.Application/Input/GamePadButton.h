#pragma once

#include "Core.Application/Application.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGamepadButton : u8 {
    Button0 = 0,
    Button1,
    Button2,
    Button3,
    Button4,
    Button5,
    Button6,
    Button7,
    Button8,
    Button9,

    DPadUp,
    DPadLeft,
    DPadRight,
    DPadDown,

    A = Button0,
    B = Button1,
    X = Button2,
    Y = Button3,

    LeftThumb = Button4,
    RightThumb = Button5,

    LB = LeftThumb,
    RB = RightThumb,

    Start = Button6,
    Back = Button7,

    LeftShoulder = Button8,
    RightShoulder = Button9,

    LS = LeftShoulder,
    RS = RightShoulder,
};
//----------------------------------------------------------------------------
FStringView GamepadButtonToCStr(EGamepadButton value);
FStringView GamepadButtonToXBoxCStr(EGamepadButton value);
//----------------------------------------------------------------------------
TMemoryView<const EGamepadButton> EachGamepadButtons();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
