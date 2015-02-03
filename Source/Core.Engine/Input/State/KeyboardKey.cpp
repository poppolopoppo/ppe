#include "stdafx.h"

#include "KeyboardKey.h"

namespace Core {
namespace Engine {
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
    case Core::Engine::KeyboardKey::_0:
        return "0";
    case Core::Engine::KeyboardKey::_1:
        return "1";
    case Core::Engine::KeyboardKey::_2:
        return "2";
    case Core::Engine::KeyboardKey::_3:
        return "3";
    case Core::Engine::KeyboardKey::_4:
        return "4";
    case Core::Engine::KeyboardKey::_5:
        return "5";
    case Core::Engine::KeyboardKey::_6:
        return "6";
    case Core::Engine::KeyboardKey::_7:
        return "7";
    case Core::Engine::KeyboardKey::_8:
        return "8";
    case Core::Engine::KeyboardKey::_9:
        return "9";
    case Core::Engine::KeyboardKey::A:
        return "A";
    case Core::Engine::KeyboardKey::B:
        return "B";
    case Core::Engine::KeyboardKey::C:
        return "C";
    case Core::Engine::KeyboardKey::D:
        return "D";
    case Core::Engine::KeyboardKey::E:
        return "E";
    case Core::Engine::KeyboardKey::F:
        return "F";
    case Core::Engine::KeyboardKey::G:
        return "G";
    case Core::Engine::KeyboardKey::H:
        return "H";
    case Core::Engine::KeyboardKey::I:
        return "I";
    case Core::Engine::KeyboardKey::J:
        return "J";
    case Core::Engine::KeyboardKey::K:
        return "K";
    case Core::Engine::KeyboardKey::L:
        return "L";
    case Core::Engine::KeyboardKey::M:
        return "M";
    case Core::Engine::KeyboardKey::N:
        return "N";
    case Core::Engine::KeyboardKey::O:
        return "O";
    case Core::Engine::KeyboardKey::P:
        return "P";
    case Core::Engine::KeyboardKey::Q:
        return "Q";
    case Core::Engine::KeyboardKey::R:
        return "R";
    case Core::Engine::KeyboardKey::S:
        return "S";
    case Core::Engine::KeyboardKey::T:
        return "T";
    case Core::Engine::KeyboardKey::U:
        return "U";
    case Core::Engine::KeyboardKey::V:
        return "V";
    case Core::Engine::KeyboardKey::W:
        return "W";
    case Core::Engine::KeyboardKey::X:
        return "X";
    case Core::Engine::KeyboardKey::Y:
        return "Y";
    case Core::Engine::KeyboardKey::Z:
        return "Z";
    case Core::Engine::KeyboardKey::Numpad0:
        return "Numpad0";
    case Core::Engine::KeyboardKey::Numpad1:
        return "Numpad1";
    case Core::Engine::KeyboardKey::Numpad2:
        return "Numpad2";
    case Core::Engine::KeyboardKey::Numpad3:
        return "Numpad3";
    case Core::Engine::KeyboardKey::Numpad4:
        return "Numpad4";
    case Core::Engine::KeyboardKey::Numpad5:
        return "Numpad5";
    case Core::Engine::KeyboardKey::Numpad6:
        return "Numpad6";
    case Core::Engine::KeyboardKey::Numpad7:
        return "Numpad7";
    case Core::Engine::KeyboardKey::Numpad8:
        return "Numpad8";
    case Core::Engine::KeyboardKey::Numpad9:
        return "Numpad9";
    case Core::Engine::KeyboardKey::Add:
        return "Add";
    case Core::Engine::KeyboardKey::Substract:
        return "Substract";
    case Core::Engine::KeyboardKey::Multiply:
        return "Multiply";
    case Core::Engine::KeyboardKey::Divide:
        return "Divide";
    case Core::Engine::KeyboardKey::F1:
        return "F1";
    case Core::Engine::KeyboardKey::F2:
        return "F2";
    case Core::Engine::KeyboardKey::F3:
        return "F3";
    case Core::Engine::KeyboardKey::F4:
        return "F4";
    case Core::Engine::KeyboardKey::F5:
        return "F5";
    case Core::Engine::KeyboardKey::F6:
        return "F6";
    case Core::Engine::KeyboardKey::F7:
        return "F7";
    case Core::Engine::KeyboardKey::F8:
        return "F8";
    case Core::Engine::KeyboardKey::F9:
        return "F9";
    case Core::Engine::KeyboardKey::F10:
        return "F10";
    case Core::Engine::KeyboardKey::F11:
        return "F11";
    case Core::Engine::KeyboardKey::F12:
        return "F12";
    case Core::Engine::KeyboardKey::Up:
        return "Up";
    case Core::Engine::KeyboardKey::Down:
        return "Down";
    case Core::Engine::KeyboardKey::Left:
        return "Left";
    case Core::Engine::KeyboardKey::Right:
        return "Right";
    case Core::Engine::KeyboardKey::Escape:
        return "Escape";
    case Core::Engine::KeyboardKey::Space:
        return "Space";
    case Core::Engine::KeyboardKey::Pause:
        return "Pause";
    case Core::Engine::KeyboardKey::PrintScreen:
        return "PrintScreen";
    case Core::Engine::KeyboardKey::ScrollLock:
        return "ScrollLock";
    case Core::Engine::KeyboardKey::Backspace:
        return "Backspace";
    case Core::Engine::KeyboardKey::Enter:
        return "Enter";
    case Core::Engine::KeyboardKey::Tab:
        return "Tab";
    case Core::Engine::KeyboardKey::Home:
        return "Home";
    case Core::Engine::KeyboardKey::End:
        return "End";
    case Core::Engine::KeyboardKey::Insert:
        return "Insert";
    case Core::Engine::KeyboardKey::Delete:
        return "Delete";
    case Core::Engine::KeyboardKey::PageUp:
        return "PageUp";
    case Core::Engine::KeyboardKey::PageDown:
        return "PageDown";
    case Core::Engine::KeyboardKey::Alt:
        return "Alt";
    case Core::Engine::KeyboardKey::Menu:
        return "Menu";
    case Core::Engine::KeyboardKey::Control:
        return "Control";
    case Core::Engine::KeyboardKey::Shift:
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
} //!namespace Engine
} //!namespace Core
