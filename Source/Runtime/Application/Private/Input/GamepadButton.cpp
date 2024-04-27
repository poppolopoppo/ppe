// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/GamepadButton.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static constexpr EGamepadAxis GEachGamepadAxises[] = {
    EGamepadAxis::Stick0,
    EGamepadAxis::Stick1,
    EGamepadAxis::Stick2,
    EGamepadAxis::Stick3,
    EGamepadAxis::Trigger0,
    EGamepadAxis::Trigger1,
    EGamepadAxis::Trigger2,
    EGamepadAxis::Trigger4,
};
//----------------------------------------------------------------------------
static constexpr EGamepadButton GEachGamepadButtons[] = {
    EGamepadButton::Button0,
    EGamepadButton::Button1,
    EGamepadButton::Button2,
    EGamepadButton::Button3,
    EGamepadButton::Button4,
    EGamepadButton::Button5,
    EGamepadButton::Button6,
    EGamepadButton::Button7,
    EGamepadButton::Button8,
    EGamepadButton::Button9,
    EGamepadButton::DPadUp,
    EGamepadButton::DPadLeft,
    EGamepadButton::DPadRight,
    EGamepadButton::DPadDown,
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringLiteral GamepadAxisToXBoxCStr(EGamepadAxis value) NOEXCEPT {
    switch (value) {
    case EGamepadAxis::Stick0: return "GamepadStick0";
    case EGamepadAxis::Stick1: return "GamepadStick1";
    case EGamepadAxis::Stick2: return "GamepadStick2";
    case EGamepadAxis::Stick3: return "GamepadStick3";
    case EGamepadAxis::Trigger0: return "GamepadTrigger0";
    case EGamepadAxis::Trigger1: return "GamepadTrigger1";
    case EGamepadAxis::Trigger2: return "GamepadTrigger2";
    case EGamepadAxis::Trigger4: return "GamepadTrigger4";
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
FStringLiteral GamepadAxisToCStr(EGamepadAxis value) NOEXCEPT {
    switch (value) {
    case EGamepadAxis::LeftStick: return "LeftStick";
    case EGamepadAxis::RightStick: return "RightStick";
    case EGamepadAxis::LeftTrigger: return "LeftTrigger";
    case EGamepadAxis::RightTrigger:  return "RightTrigger";
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
FStringLiteral GamepadButtonToCStr(EGamepadButton value) NOEXCEPT {
    switch (value) {
    case EGamepadButton::Button0: return "GamepadButton0";
    case EGamepadButton::Button1: return "GamepadButton1";
    case EGamepadButton::Button2: return "GamepadButton2";
    case EGamepadButton::Button3: return "GamepadButton3";
    case EGamepadButton::Button4: return "GamepadButton4";
    case EGamepadButton::Button5: return "GamepadButton5";
    case EGamepadButton::Button6: return "GamepadButton6";
    case EGamepadButton::Button7: return "GamepadButton7";
    case EGamepadButton::Button8: return "GamepadButton8";
    case EGamepadButton::Button9: return "GamepadButton9";
    case EGamepadButton::DPadUp: return "GamepadDPadUp";
    case EGamepadButton::DPadLeft: return "GamepadDPadLeft";
    case EGamepadButton::DPadRight: return "GamepadDPadRight";
    case EGamepadButton::DPadDown: return "GamepadDPadDown";
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
FStringLiteral GamepadButtonToXBoxCStr(EGamepadButton value) NOEXCEPT {
    switch (value) {
    case PPE::Application::EGamepadButton::A:
        return "A";
    case PPE::Application::EGamepadButton::B:
        return "B";
    case PPE::Application::EGamepadButton::X:
        return "X";
    case PPE::Application::EGamepadButton::Y:
        return "Y";
    case PPE::Application::EGamepadButton::LeftThumb:
        return "LeftThumb";
    case PPE::Application::EGamepadButton::RightThumb:
        return "RightThumb";
    case PPE::Application::EGamepadButton::Start:
        return "Start";
    case PPE::Application::EGamepadButton::Back:
        return "Back";
    case PPE::Application::EGamepadButton::LeftShoulder:
        return "LeftShoulder";
    case PPE::Application::EGamepadButton::RightShoulder:
        return "RightShoulder";
    case PPE::Application::EGamepadButton::DPadUp:
        return "DPadUp";
    case PPE::Application::EGamepadButton::DPadLeft:
        return "DPadLeft";
    case PPE::Application::EGamepadButton::DPadRight:
        return "DPadRight";
    case PPE::Application::EGamepadButton::DPadDown:
        return "DPadDown";
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
TMemoryView<const EGamepadAxis> EachGamepadAxises() NOEXCEPT {
    return MakeConstView(GEachGamepadAxises);
}
//----------------------------------------------------------------------------
TMemoryView<const EGamepadButton> EachGamepadButtons() NOEXCEPT {
    return MakeConstView(GEachGamepadButtons);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
