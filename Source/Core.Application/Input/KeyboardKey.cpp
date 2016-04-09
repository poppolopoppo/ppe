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
StringSlice KeyboardKeyToCStr(KeyboardKey value) {
    switch (value)
    {
    case Core::Application::KeyboardKey::_0:
        return MakeStringSlice("0");
    case Core::Application::KeyboardKey::_1:
        return MakeStringSlice("1");
    case Core::Application::KeyboardKey::_2:
        return MakeStringSlice("2");
    case Core::Application::KeyboardKey::_3:
        return MakeStringSlice("3");
    case Core::Application::KeyboardKey::_4:
        return MakeStringSlice("4");
    case Core::Application::KeyboardKey::_5:
        return MakeStringSlice("5");
    case Core::Application::KeyboardKey::_6:
        return MakeStringSlice("6");
    case Core::Application::KeyboardKey::_7:
        return MakeStringSlice("7");
    case Core::Application::KeyboardKey::_8:
        return MakeStringSlice("8");
    case Core::Application::KeyboardKey::_9:
        return MakeStringSlice("9");
    case Core::Application::KeyboardKey::A:
        return MakeStringSlice("A");
    case Core::Application::KeyboardKey::B:
        return MakeStringSlice("B");
    case Core::Application::KeyboardKey::C:
        return MakeStringSlice("C");
    case Core::Application::KeyboardKey::D:
        return MakeStringSlice("D");
    case Core::Application::KeyboardKey::E:
        return MakeStringSlice("E");
    case Core::Application::KeyboardKey::F:
        return MakeStringSlice("F");
    case Core::Application::KeyboardKey::G:
        return MakeStringSlice("G");
    case Core::Application::KeyboardKey::H:
        return MakeStringSlice("H");
    case Core::Application::KeyboardKey::I:
        return MakeStringSlice("I");
    case Core::Application::KeyboardKey::J:
        return MakeStringSlice("J");
    case Core::Application::KeyboardKey::K:
        return MakeStringSlice("K");
    case Core::Application::KeyboardKey::L:
        return MakeStringSlice("L");
    case Core::Application::KeyboardKey::M:
        return MakeStringSlice("M");
    case Core::Application::KeyboardKey::N:
        return MakeStringSlice("N");
    case Core::Application::KeyboardKey::O:
        return MakeStringSlice("O");
    case Core::Application::KeyboardKey::P:
        return MakeStringSlice("P");
    case Core::Application::KeyboardKey::Q:
        return MakeStringSlice("Q");
    case Core::Application::KeyboardKey::R:
        return MakeStringSlice("R");
    case Core::Application::KeyboardKey::S:
        return MakeStringSlice("S");
    case Core::Application::KeyboardKey::T:
        return MakeStringSlice("T");
    case Core::Application::KeyboardKey::U:
        return MakeStringSlice("U");
    case Core::Application::KeyboardKey::V:
        return MakeStringSlice("V");
    case Core::Application::KeyboardKey::W:
        return MakeStringSlice("W");
    case Core::Application::KeyboardKey::X:
        return MakeStringSlice("X");
    case Core::Application::KeyboardKey::Y:
        return MakeStringSlice("Y");
    case Core::Application::KeyboardKey::Z:
        return MakeStringSlice("Z");
    case Core::Application::KeyboardKey::Numpad0:
        return MakeStringSlice("Numpad0");
    case Core::Application::KeyboardKey::Numpad1:
        return MakeStringSlice("Numpad1");
    case Core::Application::KeyboardKey::Numpad2:
        return MakeStringSlice("Numpad2");
    case Core::Application::KeyboardKey::Numpad3:
        return MakeStringSlice("Numpad3");
    case Core::Application::KeyboardKey::Numpad4:
        return MakeStringSlice("Numpad4");
    case Core::Application::KeyboardKey::Numpad5:
        return MakeStringSlice("Numpad5");
    case Core::Application::KeyboardKey::Numpad6:
        return MakeStringSlice("Numpad6");
    case Core::Application::KeyboardKey::Numpad7:
        return MakeStringSlice("Numpad7");
    case Core::Application::KeyboardKey::Numpad8:
        return MakeStringSlice("Numpad8");
    case Core::Application::KeyboardKey::Numpad9:
        return MakeStringSlice("Numpad9");
    case Core::Application::KeyboardKey::Add:
        return MakeStringSlice("Add");
    case Core::Application::KeyboardKey::Subtract:
        return MakeStringSlice("Subtract");
    case Core::Application::KeyboardKey::Multiply:
        return MakeStringSlice("Multiply");
    case Core::Application::KeyboardKey::Divide:
        return MakeStringSlice("Divide");
    case Core::Application::KeyboardKey::F1:
        return MakeStringSlice("F1");
    case Core::Application::KeyboardKey::F2:
        return MakeStringSlice("F2");
    case Core::Application::KeyboardKey::F3:
        return MakeStringSlice("F3");
    case Core::Application::KeyboardKey::F4:
        return MakeStringSlice("F4");
    case Core::Application::KeyboardKey::F5:
        return MakeStringSlice("F5");
    case Core::Application::KeyboardKey::F6:
        return MakeStringSlice("F6");
    case Core::Application::KeyboardKey::F7:
        return MakeStringSlice("F7");
    case Core::Application::KeyboardKey::F8:
        return MakeStringSlice("F8");
    case Core::Application::KeyboardKey::F9:
        return MakeStringSlice("F9");
    case Core::Application::KeyboardKey::F10:
        return MakeStringSlice("F10");
    case Core::Application::KeyboardKey::F11:
        return MakeStringSlice("F11");
    case Core::Application::KeyboardKey::F12:
        return MakeStringSlice("F12");
    case Core::Application::KeyboardKey::Up:
        return MakeStringSlice("Up");
    case Core::Application::KeyboardKey::Down:
        return MakeStringSlice("Down");
    case Core::Application::KeyboardKey::Left:
        return MakeStringSlice("Left");
    case Core::Application::KeyboardKey::Right:
        return MakeStringSlice("Right");
    case Core::Application::KeyboardKey::Escape:
        return MakeStringSlice("Escape");
    case Core::Application::KeyboardKey::Space:
        return MakeStringSlice("Space");
    case Core::Application::KeyboardKey::Pause:
        return MakeStringSlice("Pause");
    case Core::Application::KeyboardKey::PrintScreen:
        return MakeStringSlice("PrintScreen");
    case Core::Application::KeyboardKey::ScrollLock:
        return MakeStringSlice("ScrollLock");
    case Core::Application::KeyboardKey::Backspace:
        return MakeStringSlice("Backspace");
    case Core::Application::KeyboardKey::Enter:
        return MakeStringSlice("Enter");
    case Core::Application::KeyboardKey::Tab:
        return MakeStringSlice("Tab");
    case Core::Application::KeyboardKey::Home:
        return MakeStringSlice("Home");
    case Core::Application::KeyboardKey::End:
        return MakeStringSlice("End");
    case Core::Application::KeyboardKey::Insert:
        return MakeStringSlice("Insert");
    case Core::Application::KeyboardKey::Delete:
        return MakeStringSlice("Delete");
    case Core::Application::KeyboardKey::PageUp:
        return MakeStringSlice("PageUp");
    case Core::Application::KeyboardKey::PageDown:
        return MakeStringSlice("PageDown");
    case Core::Application::KeyboardKey::Alt:
        return MakeStringSlice("Alt");
    case Core::Application::KeyboardKey::Menu:
        return MakeStringSlice("Menu");
    case Core::Application::KeyboardKey::Control:
        return MakeStringSlice("Control");
    case Core::Application::KeyboardKey::Shift:
        return MakeStringSlice("Shift");
    }
    AssertNotImplemented();
    return StringSlice();
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
