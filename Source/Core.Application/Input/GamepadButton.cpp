#include "stdafx.h"

#include "GamePadButton.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static constexpr EGamepadButton gEachGamepadButtons[] = {
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
FStringView GamepadButtonToCStr(EGamepadButton value) {
    switch (value)
    {
    case Core::Application::EGamepadButton::Button0:
        return MakeStringView("Button0");
    case Core::Application::EGamepadButton::Button1:
        return MakeStringView("Button1");
    case Core::Application::EGamepadButton::Button2:
        return MakeStringView("Button2");
    case Core::Application::EGamepadButton::Button3:
        return MakeStringView("Button3");
    case Core::Application::EGamepadButton::Button4:
        return MakeStringView("Button4");
    case Core::Application::EGamepadButton::Button5:
        return MakeStringView("Button5");
    case Core::Application::EGamepadButton::Button6:
        return MakeStringView("Button6");
    case Core::Application::EGamepadButton::Button7:
        return MakeStringView("Button7");
    case Core::Application::EGamepadButton::Button8:
        return MakeStringView("Button8");
    case Core::Application::EGamepadButton::Button9:
        return MakeStringView("Button9");
    case Core::Application::EGamepadButton::DPadUp:
        return MakeStringView("DPadUp");
    case Core::Application::EGamepadButton::DPadLeft:
        return MakeStringView("DPadLeft");
    case Core::Application::EGamepadButton::DPadRight:
        return MakeStringView("DPadRight");
    case Core::Application::EGamepadButton::DPadDown:
        return MakeStringView("DPadDown");
    default:
        AssertNotImplemented();
    }
    return FStringView();
}
//----------------------------------------------------------------------------
FStringView GamepadButtonToXBoxCStr(EGamepadButton value) {
    switch (value)
    {
    case Core::Application::EGamepadButton::A:
        return MakeStringView("A");
    case Core::Application::EGamepadButton::B:
        return MakeStringView("B");
    case Core::Application::EGamepadButton::X:
        return MakeStringView("X");
    case Core::Application::EGamepadButton::Y:
        return MakeStringView("Y");
    case Core::Application::EGamepadButton::LeftThumb:
        return MakeStringView("LeftThumb");
    case Core::Application::EGamepadButton::RightThumb:
        return MakeStringView("RightThumb");
    case Core::Application::EGamepadButton::Start:
        return MakeStringView("Start");
    case Core::Application::EGamepadButton::Back:
        return MakeStringView("Back");
    case Core::Application::EGamepadButton::LeftShoulder:
        return MakeStringView("LeftShoulder");
    case Core::Application::EGamepadButton::RightShoulder:
        return MakeStringView("RightShoulder");
    case Core::Application::EGamepadButton::DPadUp:
        return MakeStringView("DPadUp");
    case Core::Application::EGamepadButton::DPadLeft:
        return MakeStringView("DPadLeft");
    case Core::Application::EGamepadButton::DPadRight:
        return MakeStringView("DPadRight");
    case Core::Application::EGamepadButton::DPadDown:
        return MakeStringView("DPadDown");
    default:
        AssertNotImplemented();
    }
    return FStringView();
}
//----------------------------------------------------------------------------
TMemoryView<const EGamepadButton> EachGamepadButtons() {
    return MakeConstView(gEachGamepadButtons);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
