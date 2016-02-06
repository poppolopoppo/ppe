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
StringSlice KeyboardKeyToCStr(KeyboardKey value) {
    switch (value)
    {
    case Core::Engine::KeyboardKey::_0:
        return MakeStringSlice("0");
    case Core::Engine::KeyboardKey::_1:
        return MakeStringSlice("1");
    case Core::Engine::KeyboardKey::_2:
        return MakeStringSlice("2");
    case Core::Engine::KeyboardKey::_3:
        return MakeStringSlice("3");
    case Core::Engine::KeyboardKey::_4:
        return MakeStringSlice("4");
    case Core::Engine::KeyboardKey::_5:
        return MakeStringSlice("5");
    case Core::Engine::KeyboardKey::_6:
        return MakeStringSlice("6");
    case Core::Engine::KeyboardKey::_7:
        return MakeStringSlice("7");
    case Core::Engine::KeyboardKey::_8:
        return MakeStringSlice("8");
    case Core::Engine::KeyboardKey::_9:
        return MakeStringSlice("9");
    case Core::Engine::KeyboardKey::A:
        return MakeStringSlice("A");
    case Core::Engine::KeyboardKey::B:
        return MakeStringSlice("B");
    case Core::Engine::KeyboardKey::C:
        return MakeStringSlice("C");
    case Core::Engine::KeyboardKey::D:
        return MakeStringSlice("D");
    case Core::Engine::KeyboardKey::E:
        return MakeStringSlice("E");
    case Core::Engine::KeyboardKey::F:
        return MakeStringSlice("F");
    case Core::Engine::KeyboardKey::G:
        return MakeStringSlice("G");
    case Core::Engine::KeyboardKey::H:
        return MakeStringSlice("H");
    case Core::Engine::KeyboardKey::I:
        return MakeStringSlice("I");
    case Core::Engine::KeyboardKey::J:
        return MakeStringSlice("J");
    case Core::Engine::KeyboardKey::K:
        return MakeStringSlice("K");
    case Core::Engine::KeyboardKey::L:
        return MakeStringSlice("L");
    case Core::Engine::KeyboardKey::M:
        return MakeStringSlice("M");
    case Core::Engine::KeyboardKey::N:
        return MakeStringSlice("N");
    case Core::Engine::KeyboardKey::O:
        return MakeStringSlice("O");
    case Core::Engine::KeyboardKey::P:
        return MakeStringSlice("P");
    case Core::Engine::KeyboardKey::Q:
        return MakeStringSlice("Q");
    case Core::Engine::KeyboardKey::R:
        return MakeStringSlice("R");
    case Core::Engine::KeyboardKey::S:
        return MakeStringSlice("S");
    case Core::Engine::KeyboardKey::T:
        return MakeStringSlice("T");
    case Core::Engine::KeyboardKey::U:
        return MakeStringSlice("U");
    case Core::Engine::KeyboardKey::V:
        return MakeStringSlice("V");
    case Core::Engine::KeyboardKey::W:
        return MakeStringSlice("W");
    case Core::Engine::KeyboardKey::X:
        return MakeStringSlice("X");
    case Core::Engine::KeyboardKey::Y:
        return MakeStringSlice("Y");
    case Core::Engine::KeyboardKey::Z:
        return MakeStringSlice("Z");
    case Core::Engine::KeyboardKey::Numpad0:
        return MakeStringSlice("Numpad0");
    case Core::Engine::KeyboardKey::Numpad1:
        return MakeStringSlice("Numpad1");
    case Core::Engine::KeyboardKey::Numpad2:
        return MakeStringSlice("Numpad2");
    case Core::Engine::KeyboardKey::Numpad3:
        return MakeStringSlice("Numpad3");
    case Core::Engine::KeyboardKey::Numpad4:
        return MakeStringSlice("Numpad4");
    case Core::Engine::KeyboardKey::Numpad5:
        return MakeStringSlice("Numpad5");
    case Core::Engine::KeyboardKey::Numpad6:
        return MakeStringSlice("Numpad6");
    case Core::Engine::KeyboardKey::Numpad7:
        return MakeStringSlice("Numpad7");
    case Core::Engine::KeyboardKey::Numpad8:
        return MakeStringSlice("Numpad8");
    case Core::Engine::KeyboardKey::Numpad9:
        return MakeStringSlice("Numpad9");
    case Core::Engine::KeyboardKey::Add:
        return MakeStringSlice("Add");
    case Core::Engine::KeyboardKey::Substract:
        return MakeStringSlice("Substract");
    case Core::Engine::KeyboardKey::Multiply:
        return MakeStringSlice("Multiply");
    case Core::Engine::KeyboardKey::Divide:
        return MakeStringSlice("Divide");
    case Core::Engine::KeyboardKey::F1:
        return MakeStringSlice("F1");
    case Core::Engine::KeyboardKey::F2:
        return MakeStringSlice("F2");
    case Core::Engine::KeyboardKey::F3:
        return MakeStringSlice("F3");
    case Core::Engine::KeyboardKey::F4:
        return MakeStringSlice("F4");
    case Core::Engine::KeyboardKey::F5:
        return MakeStringSlice("F5");
    case Core::Engine::KeyboardKey::F6:
        return MakeStringSlice("F6");
    case Core::Engine::KeyboardKey::F7:
        return MakeStringSlice("F7");
    case Core::Engine::KeyboardKey::F8:
        return MakeStringSlice("F8");
    case Core::Engine::KeyboardKey::F9:
        return MakeStringSlice("F9");
    case Core::Engine::KeyboardKey::F10:
        return MakeStringSlice("F10");
    case Core::Engine::KeyboardKey::F11:
        return MakeStringSlice("F11");
    case Core::Engine::KeyboardKey::F12:
        return MakeStringSlice("F12");
    case Core::Engine::KeyboardKey::Up:
        return MakeStringSlice("Up");
    case Core::Engine::KeyboardKey::Down:
        return MakeStringSlice("Down");
    case Core::Engine::KeyboardKey::Left:
        return MakeStringSlice("Left");
    case Core::Engine::KeyboardKey::Right:
        return MakeStringSlice("Right");
    case Core::Engine::KeyboardKey::Escape:
        return MakeStringSlice("Escape");
    case Core::Engine::KeyboardKey::Space:
        return MakeStringSlice("Space");
    case Core::Engine::KeyboardKey::Pause:
        return MakeStringSlice("Pause");
    case Core::Engine::KeyboardKey::PrintScreen:
        return MakeStringSlice("PrintScreen");
    case Core::Engine::KeyboardKey::ScrollLock:
        return MakeStringSlice("ScrollLock");
    case Core::Engine::KeyboardKey::Backspace:
        return MakeStringSlice("Backspace");
    case Core::Engine::KeyboardKey::Enter:
        return MakeStringSlice("Enter");
    case Core::Engine::KeyboardKey::Tab:
        return MakeStringSlice("Tab");
    case Core::Engine::KeyboardKey::Home:
        return MakeStringSlice("Home");
    case Core::Engine::KeyboardKey::End:
        return MakeStringSlice("End");
    case Core::Engine::KeyboardKey::Insert:
        return MakeStringSlice("Insert");
    case Core::Engine::KeyboardKey::Delete:
        return MakeStringSlice("Delete");
    case Core::Engine::KeyboardKey::PageUp:
        return MakeStringSlice("PageUp");
    case Core::Engine::KeyboardKey::PageDown:
        return MakeStringSlice("PageDown");
    case Core::Engine::KeyboardKey::Alt:
        return MakeStringSlice("Alt");
    case Core::Engine::KeyboardKey::Menu:
        return MakeStringSlice("Menu");
    case Core::Engine::KeyboardKey::Control:
        return MakeStringSlice("Control");
    case Core::Engine::KeyboardKey::Shift:
        return MakeStringSlice("Shift");
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
