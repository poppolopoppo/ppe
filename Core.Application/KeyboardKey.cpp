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
    KeyboardKey::Substract,
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
const char *KeyboardKeyToCStr(KeyboardKey value) {
    switch (value)
    {
    case Core::Application::KeyboardKey::_0:
        return "0";
    case Core::Application::KeyboardKey::_1:
        return "1";
    case Core::Application::KeyboardKey::_2:
        return "2";
    case Core::Application::KeyboardKey::_3:
        return "3";
    case Core::Application::KeyboardKey::_4:
        return "4";
    case Core::Application::KeyboardKey::_5:
        return "5";
    case Core::Application::KeyboardKey::_6:
        return "6";
    case Core::Application::KeyboardKey::_7:
        return "7";
    case Core::Application::KeyboardKey::_8:
        return "8";
    case Core::Application::KeyboardKey::_9:
        return "9";
    case Core::Application::KeyboardKey::A:
        return "A";
    case Core::Application::KeyboardKey::B:
        return "B";
    case Core::Application::KeyboardKey::C:
        return "C";
    case Core::Application::KeyboardKey::D:
        return "D";
    case Core::Application::KeyboardKey::E:
        return "E";
    case Core::Application::KeyboardKey::F:
        return "F";
    case Core::Application::KeyboardKey::G:
        return "G";
    case Core::Application::KeyboardKey::H:
        return "H";
    case Core::Application::KeyboardKey::I:
        return "I";
    case Core::Application::KeyboardKey::J:
        return "J";
    case Core::Application::KeyboardKey::K:
        return "K";
    case Core::Application::KeyboardKey::L:
        return "L";
    case Core::Application::KeyboardKey::M:
        return "M";
    case Core::Application::KeyboardKey::N:
        return "N";
    case Core::Application::KeyboardKey::O:
        return "O";
    case Core::Application::KeyboardKey::P:
        return "P";
    case Core::Application::KeyboardKey::Q:
        return "Q";
    case Core::Application::KeyboardKey::R:
        return "R";
    case Core::Application::KeyboardKey::S:
        return "S";
    case Core::Application::KeyboardKey::T:
        return "T";
    case Core::Application::KeyboardKey::U:
        return "U";
    case Core::Application::KeyboardKey::V:
        return "V";
    case Core::Application::KeyboardKey::W:
        return "W";
    case Core::Application::KeyboardKey::X:
        return "X";
    case Core::Application::KeyboardKey::Y:
        return "Y";
    case Core::Application::KeyboardKey::Z:
        return "Z";
    case Core::Application::KeyboardKey::Numpad0:
        return "Numpad0";
    case Core::Application::KeyboardKey::Numpad1:
        return "Numpad1";
    case Core::Application::KeyboardKey::Numpad2:
        return "Numpad2";
    case Core::Application::KeyboardKey::Numpad3:
        return "Numpad3";
    case Core::Application::KeyboardKey::Numpad4:
        return "Numpad4";
    case Core::Application::KeyboardKey::Numpad5:
        return "Numpad5";
    case Core::Application::KeyboardKey::Numpad6:
        return "Numpad6";
    case Core::Application::KeyboardKey::Numpad7:
        return "Numpad7";
    case Core::Application::KeyboardKey::Numpad8:
        return "Numpad8";
    case Core::Application::KeyboardKey::Numpad9:
        return "Numpad9";
    case Core::Application::KeyboardKey::Add:
        return "Add";
    case Core::Application::KeyboardKey::Substract:
        return "Substract";
    case Core::Application::KeyboardKey::Multiply:
        return "Multiply";
    case Core::Application::KeyboardKey::Divide:
        return "Divide";
    case Core::Application::KeyboardKey::F1:
        return "F1";
    case Core::Application::KeyboardKey::F2:
        return "F2";
    case Core::Application::KeyboardKey::F3:
        return "F3";
    case Core::Application::KeyboardKey::F4:
        return "F4";
    case Core::Application::KeyboardKey::F5:
        return "F5";
    case Core::Application::KeyboardKey::F6:
        return "F6";
    case Core::Application::KeyboardKey::F7:
        return "F7";
    case Core::Application::KeyboardKey::F8:
        return "F8";
    case Core::Application::KeyboardKey::F9:
        return "F9";
    case Core::Application::KeyboardKey::F10:
        return "F10";
    case Core::Application::KeyboardKey::F11:
        return "F11";
    case Core::Application::KeyboardKey::F12:
        return "F12";
    case Core::Application::KeyboardKey::Up:
        return "Up";
    case Core::Application::KeyboardKey::Down:
        return "Down";
    case Core::Application::KeyboardKey::Left:
        return "Left";
    case Core::Application::KeyboardKey::Right:
        return "Right";
    case Core::Application::KeyboardKey::Escape:
        return "Escape";
    case Core::Application::KeyboardKey::Space:
        return "Space";
    case Core::Application::KeyboardKey::Pause:
        return "Pause";
    case Core::Application::KeyboardKey::PrintScreen:
        return "PrintScreen";
    case Core::Application::KeyboardKey::ScrollLock:
        return "ScrollLock";
    case Core::Application::KeyboardKey::Backspace:
        return "Backspace";
    case Core::Application::KeyboardKey::Enter:
        return "Enter";
    case Core::Application::KeyboardKey::Tab:
        return "Tab";
    case Core::Application::KeyboardKey::Home:
        return "Home";
    case Core::Application::KeyboardKey::End:
        return "End";
    case Core::Application::KeyboardKey::Insert:
        return "Insert";
    case Core::Application::KeyboardKey::Delete:
        return "Delete";
    case Core::Application::KeyboardKey::PageUp:
        return "PageUp";
    case Core::Application::KeyboardKey::PageDown:
        return "PageDown";
    case Core::Application::KeyboardKey::Alt:
        return "Alt";
    case Core::Application::KeyboardKey::Menu:
        return "Menu";
    case Core::Application::KeyboardKey::Control:
        return "Control";
    case Core::Application::KeyboardKey::Shift:
        return "Shift";
    }
    AssertNotImplemented();
    return nullptr;
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
