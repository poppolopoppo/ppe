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
StringSlice GamepadButtonToCStr(GamepadButton value) {
    switch (value)
    {
    case Core::Application::GamepadButton::Button0:
        return MakeStringSlice("Button0");
    case Core::Application::GamepadButton::Button1:
        return MakeStringSlice("Button1");
    case Core::Application::GamepadButton::Button2:
        return MakeStringSlice("Button2");
    case Core::Application::GamepadButton::Button3:
        return MakeStringSlice("Button3");
    case Core::Application::GamepadButton::Button4:
        return MakeStringSlice("Button4");
    case Core::Application::GamepadButton::Button5:
        return MakeStringSlice("Button5");
    case Core::Application::GamepadButton::Button6:
        return MakeStringSlice("Button6");
    case Core::Application::GamepadButton::Button7:
        return MakeStringSlice("Button7");
    case Core::Application::GamepadButton::Button8:
        return MakeStringSlice("Button8");
    case Core::Application::GamepadButton::Button9:
        return MakeStringSlice("Button9");
    case Core::Application::GamepadButton::DPadUp:
        return MakeStringSlice("DPadUp");
    case Core::Application::GamepadButton::DPadLeft:
        return MakeStringSlice("DPadLeft");
    case Core::Application::GamepadButton::DPadRight:
        return MakeStringSlice("DPadRight");
    case Core::Application::GamepadButton::DPadDown:
        return MakeStringSlice("DPadDown");
    default:
        AssertNotImplemented();
    }
    return StringSlice();
}
//----------------------------------------------------------------------------
StringSlice GamepadButtonToXBoxCStr(GamepadButton value) {
    switch (value)
    {
    case Core::Application::GamepadButton::A:
        return MakeStringSlice("A");
    case Core::Application::GamepadButton::B:
        return MakeStringSlice("B");
    case Core::Application::GamepadButton::X:
        return MakeStringSlice("X");
    case Core::Application::GamepadButton::Y:
        return MakeStringSlice("Y");
    case Core::Application::GamepadButton::LeftThumb:
        return MakeStringSlice("LeftThumb");
    case Core::Application::GamepadButton::RightThumb:
        return MakeStringSlice("RightThumb");
    case Core::Application::GamepadButton::Start:
        return MakeStringSlice("Start");
    case Core::Application::GamepadButton::Back:
        return MakeStringSlice("Back");
    case Core::Application::GamepadButton::LeftShoulder:
        return MakeStringSlice("LeftShoulder");
    case Core::Application::GamepadButton::RightShoulder:
        return MakeStringSlice("RightShoulder");
    case Core::Application::GamepadButton::DPadUp:
        return MakeStringSlice("DPadUp");
    case Core::Application::GamepadButton::DPadLeft:
        return MakeStringSlice("DPadLeft");
    case Core::Application::GamepadButton::DPadRight:
        return MakeStringSlice("DPadRight");
    case Core::Application::GamepadButton::DPadDown:
        return MakeStringSlice("DPadDown");
    default:
        AssertNotImplemented();
    }
    return StringSlice();
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