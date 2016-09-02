#include "stdafx.h"

#include "KeyboardKey.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const KeyboardKey gEachKeyboardKeys[] = {
    // Numeric

    KeyboardKey::_0,
    KeyboardKey::_1,
    KeyboardKey::_2,
    KeyboardKey::_3,
    KeyboardKey::_4,
    KeyboardKey::_5,
    KeyboardKey::_6,
    KeyboardKey::_7,
    KeyboardKey::_8,
    KeyboardKey::_9,

    // Alpha

    KeyboardKey::A,
    KeyboardKey::B,
    KeyboardKey::C,
    KeyboardKey::D,
    KeyboardKey::E,
    KeyboardKey::F,
    KeyboardKey::G,
    KeyboardKey::H,
    KeyboardKey::I,
    KeyboardKey::J,
    KeyboardKey::K,
    KeyboardKey::L,
    KeyboardKey::M,
    KeyboardKey::N,
    KeyboardKey::O,
    KeyboardKey::P,
    KeyboardKey::Q,
    KeyboardKey::R,
    KeyboardKey::S,
    KeyboardKey::T,
    KeyboardKey::U,
    KeyboardKey::V,
    KeyboardKey::W,
    KeyboardKey::X,
    KeyboardKey::Y,
    KeyboardKey::Z,

    // Numpad

    KeyboardKey::Numpad0,
    KeyboardKey::Numpad1,
    KeyboardKey::Numpad2,
    KeyboardKey::Numpad3,
    KeyboardKey::Numpad4,
    KeyboardKey::Numpad5,
    KeyboardKey::Numpad6,
    KeyboardKey::Numpad7,
    KeyboardKey::Numpad8,
    KeyboardKey::Numpad9,

    KeyboardKey::Add,
    KeyboardKey::Subtract,
    KeyboardKey::Multiply,
    KeyboardKey::Divide,
    KeyboardKey::Enter,

    // Function

    KeyboardKey::F1,
    KeyboardKey::F2,
    KeyboardKey::F3,
    KeyboardKey::F4,
    KeyboardKey::F5,
    KeyboardKey::F6,
    KeyboardKey::F7,
    KeyboardKey::F8,
    KeyboardKey::F9,
    KeyboardKey::F10,
    KeyboardKey::F11,
    KeyboardKey::F12,

    // Direction

    KeyboardKey::Up,
    KeyboardKey::Down,
    KeyboardKey::Left,
    KeyboardKey::Right,

    // Specials

    KeyboardKey::Escape,
    KeyboardKey::Space,

    KeyboardKey::Pause,
    KeyboardKey::PrintScreen,
    KeyboardKey::ScrollLock,

    KeyboardKey::Backspace,
    KeyboardKey::Enter,
    KeyboardKey::Tab,

    KeyboardKey::Home,
    KeyboardKey::End,
    KeyboardKey::Insert,
    KeyboardKey::Delete,
    KeyboardKey::PageUp,
    KeyboardKey::PageDown,

    // Modifiers

    KeyboardKey::Alt,
    KeyboardKey::Menu,          // windows key
    KeyboardKey::Control,
    KeyboardKey::Shift,
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView KeyboardKeyToCStr(KeyboardKey value) {
    switch (value)
    {
    case Core::Application::KeyboardKey::_0:
        return MakeStringView("0");
    case Core::Application::KeyboardKey::_1:
        return MakeStringView("1");
    case Core::Application::KeyboardKey::_2:
        return MakeStringView("2");
    case Core::Application::KeyboardKey::_3:
        return MakeStringView("3");
    case Core::Application::KeyboardKey::_4:
        return MakeStringView("4");
    case Core::Application::KeyboardKey::_5:
        return MakeStringView("5");
    case Core::Application::KeyboardKey::_6:
        return MakeStringView("6");
    case Core::Application::KeyboardKey::_7:
        return MakeStringView("7");
    case Core::Application::KeyboardKey::_8:
        return MakeStringView("8");
    case Core::Application::KeyboardKey::_9:
        return MakeStringView("9");
    case Core::Application::KeyboardKey::A:
        return MakeStringView("A");
    case Core::Application::KeyboardKey::B:
        return MakeStringView("B");
    case Core::Application::KeyboardKey::C:
        return MakeStringView("C");
    case Core::Application::KeyboardKey::D:
        return MakeStringView("D");
    case Core::Application::KeyboardKey::E:
        return MakeStringView("E");
    case Core::Application::KeyboardKey::F:
        return MakeStringView("F");
    case Core::Application::KeyboardKey::G:
        return MakeStringView("G");
    case Core::Application::KeyboardKey::H:
        return MakeStringView("H");
    case Core::Application::KeyboardKey::I:
        return MakeStringView("I");
    case Core::Application::KeyboardKey::J:
        return MakeStringView("J");
    case Core::Application::KeyboardKey::K:
        return MakeStringView("K");
    case Core::Application::KeyboardKey::L:
        return MakeStringView("L");
    case Core::Application::KeyboardKey::M:
        return MakeStringView("M");
    case Core::Application::KeyboardKey::N:
        return MakeStringView("N");
    case Core::Application::KeyboardKey::O:
        return MakeStringView("O");
    case Core::Application::KeyboardKey::P:
        return MakeStringView("P");
    case Core::Application::KeyboardKey::Q:
        return MakeStringView("Q");
    case Core::Application::KeyboardKey::R:
        return MakeStringView("R");
    case Core::Application::KeyboardKey::S:
        return MakeStringView("S");
    case Core::Application::KeyboardKey::T:
        return MakeStringView("T");
    case Core::Application::KeyboardKey::U:
        return MakeStringView("U");
    case Core::Application::KeyboardKey::V:
        return MakeStringView("V");
    case Core::Application::KeyboardKey::W:
        return MakeStringView("W");
    case Core::Application::KeyboardKey::X:
        return MakeStringView("X");
    case Core::Application::KeyboardKey::Y:
        return MakeStringView("Y");
    case Core::Application::KeyboardKey::Z:
        return MakeStringView("Z");
    case Core::Application::KeyboardKey::Numpad0:
        return MakeStringView("Numpad0");
    case Core::Application::KeyboardKey::Numpad1:
        return MakeStringView("Numpad1");
    case Core::Application::KeyboardKey::Numpad2:
        return MakeStringView("Numpad2");
    case Core::Application::KeyboardKey::Numpad3:
        return MakeStringView("Numpad3");
    case Core::Application::KeyboardKey::Numpad4:
        return MakeStringView("Numpad4");
    case Core::Application::KeyboardKey::Numpad5:
        return MakeStringView("Numpad5");
    case Core::Application::KeyboardKey::Numpad6:
        return MakeStringView("Numpad6");
    case Core::Application::KeyboardKey::Numpad7:
        return MakeStringView("Numpad7");
    case Core::Application::KeyboardKey::Numpad8:
        return MakeStringView("Numpad8");
    case Core::Application::KeyboardKey::Numpad9:
        return MakeStringView("Numpad9");
    case Core::Application::KeyboardKey::Add:
        return MakeStringView("Add");
    case Core::Application::KeyboardKey::Subtract:
        return MakeStringView("Subtract");
    case Core::Application::KeyboardKey::Multiply:
        return MakeStringView("Multiply");
    case Core::Application::KeyboardKey::Divide:
        return MakeStringView("Divide");
    case Core::Application::KeyboardKey::F1:
        return MakeStringView("F1");
    case Core::Application::KeyboardKey::F2:
        return MakeStringView("F2");
    case Core::Application::KeyboardKey::F3:
        return MakeStringView("F3");
    case Core::Application::KeyboardKey::F4:
        return MakeStringView("F4");
    case Core::Application::KeyboardKey::F5:
        return MakeStringView("F5");
    case Core::Application::KeyboardKey::F6:
        return MakeStringView("F6");
    case Core::Application::KeyboardKey::F7:
        return MakeStringView("F7");
    case Core::Application::KeyboardKey::F8:
        return MakeStringView("F8");
    case Core::Application::KeyboardKey::F9:
        return MakeStringView("F9");
    case Core::Application::KeyboardKey::F10:
        return MakeStringView("F10");
    case Core::Application::KeyboardKey::F11:
        return MakeStringView("F11");
    case Core::Application::KeyboardKey::F12:
        return MakeStringView("F12");
    case Core::Application::KeyboardKey::Up:
        return MakeStringView("Up");
    case Core::Application::KeyboardKey::Down:
        return MakeStringView("Down");
    case Core::Application::KeyboardKey::Left:
        return MakeStringView("Left");
    case Core::Application::KeyboardKey::Right:
        return MakeStringView("Right");
    case Core::Application::KeyboardKey::Escape:
        return MakeStringView("Escape");
    case Core::Application::KeyboardKey::Space:
        return MakeStringView("Space");
    case Core::Application::KeyboardKey::Pause:
        return MakeStringView("Pause");
    case Core::Application::KeyboardKey::PrintScreen:
        return MakeStringView("PrintScreen");
    case Core::Application::KeyboardKey::ScrollLock:
        return MakeStringView("ScrollLock");
    case Core::Application::KeyboardKey::Backspace:
        return MakeStringView("Backspace");
    case Core::Application::KeyboardKey::Enter:
        return MakeStringView("Enter");
    case Core::Application::KeyboardKey::Tab:
        return MakeStringView("Tab");
    case Core::Application::KeyboardKey::Home:
        return MakeStringView("Home");
    case Core::Application::KeyboardKey::End:
        return MakeStringView("End");
    case Core::Application::KeyboardKey::Insert:
        return MakeStringView("Insert");
    case Core::Application::KeyboardKey::Delete:
        return MakeStringView("Delete");
    case Core::Application::KeyboardKey::PageUp:
        return MakeStringView("PageUp");
    case Core::Application::KeyboardKey::PageDown:
        return MakeStringView("PageDown");
    case Core::Application::KeyboardKey::Alt:
        return MakeStringView("Alt");
    case Core::Application::KeyboardKey::Menu:
        return MakeStringView("Menu");
    case Core::Application::KeyboardKey::Control:
        return MakeStringView("Control");
    case Core::Application::KeyboardKey::Shift:
        return MakeStringView("Shift");
    }
    AssertNotImplemented();
    return StringView();
}
//----------------------------------------------------------------------------
MemoryView<const KeyboardKey> EachKeyboardKeys() {
    return MakeView(gEachKeyboardKeys);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
