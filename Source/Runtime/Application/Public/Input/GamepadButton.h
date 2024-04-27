#pragma once

#include "Application.h"

#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGamepadAxis : u8 {
    Stick0 = 0,
    Stick1,
    Stick2,
    Stick3,

    Trigger0,
    Trigger1,
    Trigger2,
    Trigger4,

    LeftStick = Stick0,
    RightStick = Stick1,

    LS = LeftStick,
    LR = RightStick,

    LeftTrigger = Trigger0,
    RightTrigger = Trigger1,

    LT = LeftTrigger,
    RT = RightTrigger,
};
//----------------------------------------------------------------------------
NODISCARD PPE_APPLICATION_API TMemoryView<const EGamepadAxis> EachGamepadAxises() NOEXCEPT;
NODISCARD PPE_APPLICATION_API FStringLiteral GamepadAxisToCStr(EGamepadAxis value) NOEXCEPT;
NODISCARD PPE_APPLICATION_API FStringLiteral GamepadAxisToXBoxCStr(EGamepadAxis value) NOEXCEPT;
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
NODISCARD PPE_APPLICATION_API TMemoryView<const EGamepadButton> EachGamepadButtons() NOEXCEPT;
NODISCARD PPE_APPLICATION_API FStringLiteral GamepadButtonToCStr(EGamepadButton value) NOEXCEPT;
NODISCARD PPE_APPLICATION_API FStringLiteral GamepadButtonToXBoxCStr(EGamepadButton value) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, EGamepadAxis value) {
    return oss << GamepadAxisToXBoxCStr(value);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, EGamepadButton value) {
    return oss << GamepadButtonToXBoxCStr(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
