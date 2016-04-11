#pragma once

#include "Core.Application/Application.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class GamepadButton : u32 {
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

    DPadUp = '^',
    DPadLeft = '<',
    DPadRight = '>',
    DPadDown = 'v',
};
//----------------------------------------------------------------------------
StringSlice GamepadButtonToCStr(GamepadButton value);
StringSlice GamepadButtonToXBoxCStr(GamepadButton value);
//----------------------------------------------------------------------------
MemoryView<const GamepadButton> EachGamepadButtons();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
