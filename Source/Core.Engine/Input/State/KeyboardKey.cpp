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
    case Core::Engine::KeyboardKey::_0:
        return MakeStringView("0");
    case Core::Engine::KeyboardKey::_1:
        return MakeStringView("1");
    case Core::Engine::KeyboardKey::_2:
        return MakeStringView("2");
    case Core::Engine::KeyboardKey::_3:
        return MakeStringView("3");
    case Core::Engine::KeyboardKey::_4:
        return MakeStringView("4");
    case Core::Engine::KeyboardKey::_5:
        return MakeStringView("5");
    case Core::Engine::KeyboardKey::_6:
        return MakeStringView("6");
    case Core::Engine::KeyboardKey::_7:
        return MakeStringView("7");
    case Core::Engine::KeyboardKey::_8:
        return MakeStringView("8");
    case Core::Engine::KeyboardKey::_9:
        return MakeStringView("9");
    case Core::Engine::KeyboardKey::A:
        return MakeStringView("A");
    case Core::Engine::KeyboardKey::B:
        return MakeStringView("B");
    case Core::Engine::KeyboardKey::C:
        return MakeStringView("C");
    case Core::Engine::KeyboardKey::D:
        return MakeStringView("D");
    case Core::Engine::KeyboardKey::E:
        return MakeStringView("E");
    case Core::Engine::KeyboardKey::F:
        return MakeStringView("F");
    case Core::Engine::KeyboardKey::G:
        return MakeStringView("G");
    case Core::Engine::KeyboardKey::H:
        return MakeStringView("H");
    case Core::Engine::KeyboardKey::I:
        return MakeStringView("I");
    case Core::Engine::KeyboardKey::J:
        return MakeStringView("J");
    case Core::Engine::KeyboardKey::K:
        return MakeStringView("K");
    case Core::Engine::KeyboardKey::L:
        return MakeStringView("L");
    case Core::Engine::KeyboardKey::M:
        return MakeStringView("M");
    case Core::Engine::KeyboardKey::N:
        return MakeStringView("N");
    case Core::Engine::KeyboardKey::O:
        return MakeStringView("O");
    case Core::Engine::KeyboardKey::P:
        return MakeStringView("P");
    case Core::Engine::KeyboardKey::Q:
        return MakeStringView("Q");
    case Core::Engine::KeyboardKey::R:
        return MakeStringView("R");
    case Core::Engine::KeyboardKey::S:
        return MakeStringView("S");
    case Core::Engine::KeyboardKey::T:
        return MakeStringView("T");
    case Core::Engine::KeyboardKey::U:
        return MakeStringView("U");
    case Core::Engine::KeyboardKey::V:
        return MakeStringView("V");
    case Core::Engine::KeyboardKey::W:
        return MakeStringView("W");
    case Core::Engine::KeyboardKey::X:
        return MakeStringView("X");
    case Core::Engine::KeyboardKey::Y:
        return MakeStringView("Y");
    case Core::Engine::KeyboardKey::Z:
        return MakeStringView("Z");
    case Core::Engine::KeyboardKey::Numpad0:
        return MakeStringView("Numpad0");
    case Core::Engine::KeyboardKey::Numpad1:
        return MakeStringView("Numpad1");
    case Core::Engine::KeyboardKey::Numpad2:
        return MakeStringView("Numpad2");
    case Core::Engine::KeyboardKey::Numpad3:
        return MakeStringView("Numpad3");
    case Core::Engine::KeyboardKey::Numpad4:
        return MakeStringView("Numpad4");
    case Core::Engine::KeyboardKey::Numpad5:
        return MakeStringView("Numpad5");
    case Core::Engine::KeyboardKey::Numpad6:
        return MakeStringView("Numpad6");
    case Core::Engine::KeyboardKey::Numpad7:
        return MakeStringView("Numpad7");
    case Core::Engine::KeyboardKey::Numpad8:
        return MakeStringView("Numpad8");
    case Core::Engine::KeyboardKey::Numpad9:
        return MakeStringView("Numpad9");
    case Core::Engine::KeyboardKey::Add:
        return MakeStringView("Add");
    case Core::Engine::KeyboardKey::Subtract:
        return MakeStringView("Subtract");
    case Core::Engine::KeyboardKey::Multiply:
        return MakeStringView("Multiply");
    case Core::Engine::KeyboardKey::Divide:
        return MakeStringView("Divide");
    case Core::Engine::KeyboardKey::F1:
        return MakeStringView("F1");
    case Core::Engine::KeyboardKey::F2:
        return MakeStringView("F2");
    case Core::Engine::KeyboardKey::F3:
        return MakeStringView("F3");
    case Core::Engine::KeyboardKey::F4:
        return MakeStringView("F4");
    case Core::Engine::KeyboardKey::F5:
        return MakeStringView("F5");
    case Core::Engine::KeyboardKey::F6:
        return MakeStringView("F6");
    case Core::Engine::KeyboardKey::F7:
        return MakeStringView("F7");
    case Core::Engine::KeyboardKey::F8:
        return MakeStringView("F8");
    case Core::Engine::KeyboardKey::F9:
        return MakeStringView("F9");
    case Core::Engine::KeyboardKey::F10:
        return MakeStringView("F10");
    case Core::Engine::KeyboardKey::F11:
        return MakeStringView("F11");
    case Core::Engine::KeyboardKey::F12:
        return MakeStringView("F12");
    case Core::Engine::KeyboardKey::Up:
        return MakeStringView("Up");
    case Core::Engine::KeyboardKey::Down:
        return MakeStringView("Down");
    case Core::Engine::KeyboardKey::Left:
        return MakeStringView("Left");
    case Core::Engine::KeyboardKey::Right:
        return MakeStringView("Right");
    case Core::Engine::KeyboardKey::Escape:
        return MakeStringView("Escape");
    case Core::Engine::KeyboardKey::Space:
        return MakeStringView("Space");
    case Core::Engine::KeyboardKey::Pause:
        return MakeStringView("Pause");
    case Core::Engine::KeyboardKey::PrintScreen:
        return MakeStringView("PrintScreen");
    case Core::Engine::KeyboardKey::ScrollLock:
        return MakeStringView("ScrollLock");
    case Core::Engine::KeyboardKey::Backspace:
        return MakeStringView("Backspace");
    case Core::Engine::KeyboardKey::Enter:
        return MakeStringView("Enter");
    case Core::Engine::KeyboardKey::Tab:
        return MakeStringView("Tab");
    case Core::Engine::KeyboardKey::Home:
        return MakeStringView("Home");
    case Core::Engine::KeyboardKey::End:
        return MakeStringView("End");
    case Core::Engine::KeyboardKey::Insert:
        return MakeStringView("Insert");
    case Core::Engine::KeyboardKey::Delete:
        return MakeStringView("Delete");
    case Core::Engine::KeyboardKey::PageUp:
        return MakeStringView("PageUp");
    case Core::Engine::KeyboardKey::PageDown:
        return MakeStringView("PageDown");
    case Core::Engine::KeyboardKey::Alt:
        return MakeStringView("Alt");
    case Core::Engine::KeyboardKey::Menu:
        return MakeStringView("Menu");
    case Core::Engine::KeyboardKey::Control:
        return MakeStringView("Control");
    case Core::Engine::KeyboardKey::Shift:
        return MakeStringView("Shift");
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
