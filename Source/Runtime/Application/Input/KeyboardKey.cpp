#include "stdafx.h"

#include "KeyboardKey.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static constexpr EKeyboardKey GEachKeyboardKeys[] = {
    // TNumeric

    EKeyboardKey::_0,
    EKeyboardKey::_1,
    EKeyboardKey::_2,
    EKeyboardKey::_3,
    EKeyboardKey::_4,
    EKeyboardKey::_5,
    EKeyboardKey::_6,
    EKeyboardKey::_7,
    EKeyboardKey::_8,
    EKeyboardKey::_9,

    // Alpha

    EKeyboardKey::A,
    EKeyboardKey::B,
    EKeyboardKey::C,
    EKeyboardKey::D,
    EKeyboardKey::E,
    EKeyboardKey::F,
    EKeyboardKey::G,
    EKeyboardKey::H,
    EKeyboardKey::I,
    EKeyboardKey::J,
    EKeyboardKey::K,
    EKeyboardKey::L,
    EKeyboardKey::M,
    EKeyboardKey::N,
    EKeyboardKey::O,
    EKeyboardKey::P,
    EKeyboardKey::Q,
    EKeyboardKey::R,
    EKeyboardKey::S,
    EKeyboardKey::T,
    EKeyboardKey::U,
    EKeyboardKey::V,
    EKeyboardKey::W,
    EKeyboardKey::X,
    EKeyboardKey::Y,
    EKeyboardKey::Z,

    // Numpad

    EKeyboardKey::Numpad0,
    EKeyboardKey::Numpad1,
    EKeyboardKey::Numpad2,
    EKeyboardKey::Numpad3,
    EKeyboardKey::Numpad4,
    EKeyboardKey::Numpad5,
    EKeyboardKey::Numpad6,
    EKeyboardKey::Numpad7,
    EKeyboardKey::Numpad8,
    EKeyboardKey::Numpad9,

    EKeyboardKey::Add,
    EKeyboardKey::Subtract,
    EKeyboardKey::Multiply,
    EKeyboardKey::Divide,
    EKeyboardKey::Enter,

    // Function

    EKeyboardKey::F1,
    EKeyboardKey::F2,
    EKeyboardKey::F3,
    EKeyboardKey::F4,
    EKeyboardKey::F5,
    EKeyboardKey::F6,
    EKeyboardKey::F7,
    EKeyboardKey::F8,
    EKeyboardKey::F9,
    EKeyboardKey::F10,
    EKeyboardKey::F11,
    EKeyboardKey::F12,

    // Direction

    EKeyboardKey::Up,
    EKeyboardKey::Down,
    EKeyboardKey::Left,
    EKeyboardKey::Right,

    // Specials

    EKeyboardKey::Escape,
    EKeyboardKey::Space,

    EKeyboardKey::Pause,
    EKeyboardKey::PrintScreen,
    EKeyboardKey::ScrollLock,

    EKeyboardKey::Backspace,
    EKeyboardKey::Enter,
    EKeyboardKey::Tab,

    EKeyboardKey::Home,
    EKeyboardKey::End,
    EKeyboardKey::Insert,
    EKeyboardKey::Delete,
    EKeyboardKey::PageUp,
    EKeyboardKey::PageDown,

    // Modifiers

    EKeyboardKey::Alt,
    EKeyboardKey::Menu,          // windows key
    EKeyboardKey::Control,
    EKeyboardKey::Shift,
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView KeyboardKeyToCStr(EKeyboardKey value) {
    switch (value)
    {
    case Core::Application::EKeyboardKey::_0:
        return MakeStringView("0");
    case Core::Application::EKeyboardKey::_1:
        return MakeStringView("1");
    case Core::Application::EKeyboardKey::_2:
        return MakeStringView("2");
    case Core::Application::EKeyboardKey::_3:
        return MakeStringView("3");
    case Core::Application::EKeyboardKey::_4:
        return MakeStringView("4");
    case Core::Application::EKeyboardKey::_5:
        return MakeStringView("5");
    case Core::Application::EKeyboardKey::_6:
        return MakeStringView("6");
    case Core::Application::EKeyboardKey::_7:
        return MakeStringView("7");
    case Core::Application::EKeyboardKey::_8:
        return MakeStringView("8");
    case Core::Application::EKeyboardKey::_9:
        return MakeStringView("9");
    case Core::Application::EKeyboardKey::A:
        return MakeStringView("A");
    case Core::Application::EKeyboardKey::B:
        return MakeStringView("B");
    case Core::Application::EKeyboardKey::C:
        return MakeStringView("C");
    case Core::Application::EKeyboardKey::D:
        return MakeStringView("D");
    case Core::Application::EKeyboardKey::E:
        return MakeStringView("E");
    case Core::Application::EKeyboardKey::F:
        return MakeStringView("F");
    case Core::Application::EKeyboardKey::G:
        return MakeStringView("G");
    case Core::Application::EKeyboardKey::H:
        return MakeStringView("H");
    case Core::Application::EKeyboardKey::I:
        return MakeStringView("I");
    case Core::Application::EKeyboardKey::J:
        return MakeStringView("J");
    case Core::Application::EKeyboardKey::K:
        return MakeStringView("K");
    case Core::Application::EKeyboardKey::L:
        return MakeStringView("L");
    case Core::Application::EKeyboardKey::M:
        return MakeStringView("M");
    case Core::Application::EKeyboardKey::N:
        return MakeStringView("N");
    case Core::Application::EKeyboardKey::O:
        return MakeStringView("O");
    case Core::Application::EKeyboardKey::P:
        return MakeStringView("P");
    case Core::Application::EKeyboardKey::Q:
        return MakeStringView("Q");
    case Core::Application::EKeyboardKey::R:
        return MakeStringView("R");
    case Core::Application::EKeyboardKey::S:
        return MakeStringView("S");
    case Core::Application::EKeyboardKey::T:
        return MakeStringView("T");
    case Core::Application::EKeyboardKey::U:
        return MakeStringView("U");
    case Core::Application::EKeyboardKey::V:
        return MakeStringView("V");
    case Core::Application::EKeyboardKey::W:
        return MakeStringView("W");
    case Core::Application::EKeyboardKey::X:
        return MakeStringView("X");
    case Core::Application::EKeyboardKey::Y:
        return MakeStringView("Y");
    case Core::Application::EKeyboardKey::Z:
        return MakeStringView("Z");
    case Core::Application::EKeyboardKey::Numpad0:
        return MakeStringView("Numpad0");
    case Core::Application::EKeyboardKey::Numpad1:
        return MakeStringView("Numpad1");
    case Core::Application::EKeyboardKey::Numpad2:
        return MakeStringView("Numpad2");
    case Core::Application::EKeyboardKey::Numpad3:
        return MakeStringView("Numpad3");
    case Core::Application::EKeyboardKey::Numpad4:
        return MakeStringView("Numpad4");
    case Core::Application::EKeyboardKey::Numpad5:
        return MakeStringView("Numpad5");
    case Core::Application::EKeyboardKey::Numpad6:
        return MakeStringView("Numpad6");
    case Core::Application::EKeyboardKey::Numpad7:
        return MakeStringView("Numpad7");
    case Core::Application::EKeyboardKey::Numpad8:
        return MakeStringView("Numpad8");
    case Core::Application::EKeyboardKey::Numpad9:
        return MakeStringView("Numpad9");
    case Core::Application::EKeyboardKey::Add:
        return MakeStringView("Add");
    case Core::Application::EKeyboardKey::Subtract:
        return MakeStringView("Subtract");
    case Core::Application::EKeyboardKey::Multiply:
        return MakeStringView("Multiply");
    case Core::Application::EKeyboardKey::Divide:
        return MakeStringView("Divide");
    case Core::Application::EKeyboardKey::F1:
        return MakeStringView("F1");
    case Core::Application::EKeyboardKey::F2:
        return MakeStringView("F2");
    case Core::Application::EKeyboardKey::F3:
        return MakeStringView("F3");
    case Core::Application::EKeyboardKey::F4:
        return MakeStringView("F4");
    case Core::Application::EKeyboardKey::F5:
        return MakeStringView("F5");
    case Core::Application::EKeyboardKey::F6:
        return MakeStringView("F6");
    case Core::Application::EKeyboardKey::F7:
        return MakeStringView("F7");
    case Core::Application::EKeyboardKey::F8:
        return MakeStringView("F8");
    case Core::Application::EKeyboardKey::F9:
        return MakeStringView("F9");
    case Core::Application::EKeyboardKey::F10:
        return MakeStringView("F10");
    case Core::Application::EKeyboardKey::F11:
        return MakeStringView("F11");
    case Core::Application::EKeyboardKey::F12:
        return MakeStringView("F12");
    case Core::Application::EKeyboardKey::Up:
        return MakeStringView("Up");
    case Core::Application::EKeyboardKey::Down:
        return MakeStringView("Down");
    case Core::Application::EKeyboardKey::Left:
        return MakeStringView("Left");
    case Core::Application::EKeyboardKey::Right:
        return MakeStringView("Right");
    case Core::Application::EKeyboardKey::Escape:
        return MakeStringView("Escape");
    case Core::Application::EKeyboardKey::Space:
        return MakeStringView("Space");
    case Core::Application::EKeyboardKey::Pause:
        return MakeStringView("Pause");
    case Core::Application::EKeyboardKey::PrintScreen:
        return MakeStringView("PrintScreen");
    case Core::Application::EKeyboardKey::ScrollLock:
        return MakeStringView("ScrollLock");
    case Core::Application::EKeyboardKey::Backspace:
        return MakeStringView("Backspace");
    case Core::Application::EKeyboardKey::Enter:
        return MakeStringView("Enter");
    case Core::Application::EKeyboardKey::Tab:
        return MakeStringView("Tab");
    case Core::Application::EKeyboardKey::Home:
        return MakeStringView("Home");
    case Core::Application::EKeyboardKey::End:
        return MakeStringView("End");
    case Core::Application::EKeyboardKey::Insert:
        return MakeStringView("Insert");
    case Core::Application::EKeyboardKey::Delete:
        return MakeStringView("Delete");
    case Core::Application::EKeyboardKey::PageUp:
        return MakeStringView("PageUp");
    case Core::Application::EKeyboardKey::PageDown:
        return MakeStringView("PageDown");
    case Core::Application::EKeyboardKey::Alt:
        return MakeStringView("Alt");
    case Core::Application::EKeyboardKey::Menu:
        return MakeStringView("Menu");
    case Core::Application::EKeyboardKey::Control:
        return MakeStringView("Control");
    case Core::Application::EKeyboardKey::Shift:
        return MakeStringView("Shift");
    }
    AssertNotImplemented();
    return FStringView();
}
//----------------------------------------------------------------------------
TMemoryView<const EKeyboardKey> EachKeyboardKeys() {
    return MakeView(GEachKeyboardKeys);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
