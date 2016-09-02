#include "stdafx.h"

#include "GamePadButton.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const GamepadButton gEachGamepadButtons[] = {
    GamepadButton::Button0,
    GamepadButton::Button1,
    GamepadButton::Button2,
    GamepadButton::Button3,
    GamepadButton::Button4,
    GamepadButton::Button5,
    GamepadButton::Button6,
    GamepadButton::Button7,
    GamepadButton::Button8,
    GamepadButton::Button9,
    GamepadButton::DPadUp,
    GamepadButton::DPadLeft,
    GamepadButton::DPadRight,
    GamepadButton::DPadDown,
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView GamepadButtonToCStr(GamepadButton value) {
    switch (value)
    {
    case Core::Application::GamepadButton::Button0:
        return MakeStringView("Button0");
    case Core::Application::GamepadButton::Button1:
        return MakeStringView("Button1");
    case Core::Application::GamepadButton::Button2:
        return MakeStringView("Button2");
    case Core::Application::GamepadButton::Button3:
        return MakeStringView("Button3");
    case Core::Application::GamepadButton::Button4:
        return MakeStringView("Button4");
    case Core::Application::GamepadButton::Button5:
        return MakeStringView("Button5");
    case Core::Application::GamepadButton::Button6:
        return MakeStringView("Button6");
    case Core::Application::GamepadButton::Button7:
        return MakeStringView("Button7");
    case Core::Application::GamepadButton::Button8:
        return MakeStringView("Button8");
    case Core::Application::GamepadButton::Button9:
        return MakeStringView("Button9");
    case Core::Application::GamepadButton::DPadUp:
        return MakeStringView("DPadUp");
    case Core::Application::GamepadButton::DPadLeft:
        return MakeStringView("DPadLeft");
    case Core::Application::GamepadButton::DPadRight:
        return MakeStringView("DPadRight");
    case Core::Application::GamepadButton::DPadDown:
        return MakeStringView("DPadDown");
    default:
        AssertNotImplemented();
    }
    return StringView();
}
//----------------------------------------------------------------------------
StringView GamepadButtonToXBoxCStr(GamepadButton value) {
    switch (value)
    {
    case Core::Application::GamepadButton::A:
        return MakeStringView("A");
    case Core::Application::GamepadButton::B:
        return MakeStringView("B");
    case Core::Application::GamepadButton::X:
        return MakeStringView("X");
    case Core::Application::GamepadButton::Y:
        return MakeStringView("Y");
    case Core::Application::GamepadButton::LeftThumb:
        return MakeStringView("LeftThumb");
    case Core::Application::GamepadButton::RightThumb:
        return MakeStringView("RightThumb");
    case Core::Application::GamepadButton::Start:
        return MakeStringView("Start");
    case Core::Application::GamepadButton::Back:
        return MakeStringView("Back");
    case Core::Application::GamepadButton::LeftShoulder:
        return MakeStringView("LeftShoulder");
    case Core::Application::GamepadButton::RightShoulder:
        return MakeStringView("RightShoulder");
    case Core::Application::GamepadButton::DPadUp:
        return MakeStringView("DPadUp");
    case Core::Application::GamepadButton::DPadLeft:
        return MakeStringView("DPadLeft");
    case Core::Application::GamepadButton::DPadRight:
        return MakeStringView("DPadRight");
    case Core::Application::GamepadButton::DPadDown:
        return MakeStringView("DPadDown");
    default:
        AssertNotImplemented();
    }
    return StringView();
}
//----------------------------------------------------------------------------
MemoryView<const GamepadButton> EachGamepadButtons() {
    return MakeConstView(gEachGamepadButtons);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
