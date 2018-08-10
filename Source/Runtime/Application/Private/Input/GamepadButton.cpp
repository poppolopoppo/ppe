#include "stdafx.h"

#include "GamePadButton.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
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
FStringView GamepadButtonToCStr(EGamepadButton value) {
    switch (value)
    {
    case PPE::Application::EGamepadButton::Button0:
        return MakeStringView("Button0");
    case PPE::Application::EGamepadButton::Button1:
        return MakeStringView("Button1");
    case PPE::Application::EGamepadButton::Button2:
        return MakeStringView("Button2");
    case PPE::Application::EGamepadButton::Button3:
        return MakeStringView("Button3");
    case PPE::Application::EGamepadButton::Button4:
        return MakeStringView("Button4");
    case PPE::Application::EGamepadButton::Button5:
        return MakeStringView("Button5");
    case PPE::Application::EGamepadButton::Button6:
        return MakeStringView("Button6");
    case PPE::Application::EGamepadButton::Button7:
        return MakeStringView("Button7");
    case PPE::Application::EGamepadButton::Button8:
        return MakeStringView("Button8");
    case PPE::Application::EGamepadButton::Button9:
        return MakeStringView("Button9");
    case PPE::Application::EGamepadButton::DPadUp:
        return MakeStringView("DPadUp");
    case PPE::Application::EGamepadButton::DPadLeft:
        return MakeStringView("DPadLeft");
    case PPE::Application::EGamepadButton::DPadRight:
        return MakeStringView("DPadRight");
    case PPE::Application::EGamepadButton::DPadDown:
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
    case PPE::Application::EGamepadButton::A:
        return MakeStringView("A");
    case PPE::Application::EGamepadButton::B:
        return MakeStringView("B");
    case PPE::Application::EGamepadButton::X:
        return MakeStringView("X");
    case PPE::Application::EGamepadButton::Y:
        return MakeStringView("Y");
    case PPE::Application::EGamepadButton::LeftThumb:
        return MakeStringView("LeftThumb");
    case PPE::Application::EGamepadButton::RightThumb:
        return MakeStringView("RightThumb");
    case PPE::Application::EGamepadButton::Start:
        return MakeStringView("Start");
    case PPE::Application::EGamepadButton::Back:
        return MakeStringView("Back");
    case PPE::Application::EGamepadButton::LeftShoulder:
        return MakeStringView("LeftShoulder");
    case PPE::Application::EGamepadButton::RightShoulder:
        return MakeStringView("RightShoulder");
    case PPE::Application::EGamepadButton::DPadUp:
        return MakeStringView("DPadUp");
    case PPE::Application::EGamepadButton::DPadLeft:
        return MakeStringView("DPadLeft");
    case PPE::Application::EGamepadButton::DPadRight:
        return MakeStringView("DPadRight");
    case PPE::Application::EGamepadButton::DPadDown:
        return MakeStringView("DPadDown");
    default:
        AssertNotImplemented();
    }
    return FStringView();
}
//----------------------------------------------------------------------------
TMemoryView<const EGamepadButton> EachGamepadButtons() {
    return MakeConstView(GEachGamepadButtons);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
