#include "stdafx.h"

#include "KeyboardKey.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static constexpr EKeyboardKey gEachKeyboardKeys[] = {
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
    case Core::Engine::EKeyboardKey::_0:
        return MakeStringView("0");
    case Core::Engine::EKeyboardKey::_1:
        return MakeStringView("1");
    case Core::Engine::EKeyboardKey::_2:
        return MakeStringView("2");
    case Core::Engine::EKeyboardKey::_3:
        return MakeStringView("3");
    case Core::Engine::EKeyboardKey::_4:
        return MakeStringView("4");
    case Core::Engine::EKeyboardKey::_5:
        return MakeStringView("5");
    case Core::Engine::EKeyboardKey::_6:
        return MakeStringView("6");
    case Core::Engine::EKeyboardKey::_7:
        return MakeStringView("7");
    case Core::Engine::EKeyboardKey::_8:
        return MakeStringView("8");
    case Core::Engine::EKeyboardKey::_9:
        return MakeStringView("9");
    case Core::Engine::EKeyboardKey::A:
        return MakeStringView("A");
    case Core::Engine::EKeyboardKey::B:
        return MakeStringView("B");
    case Core::Engine::EKeyboardKey::C:
        return MakeStringView("C");
    case Core::Engine::EKeyboardKey::D:
        return MakeStringView("D");
    case Core::Engine::EKeyboardKey::E:
        return MakeStringView("E");
    case Core::Engine::EKeyboardKey::F:
        return MakeStringView("F");
    case Core::Engine::EKeyboardKey::G:
        return MakeStringView("G");
    case Core::Engine::EKeyboardKey::H:
        return MakeStringView("H");
    case Core::Engine::EKeyboardKey::I:
        return MakeStringView("I");
    case Core::Engine::EKeyboardKey::J:
        return MakeStringView("J");
    case Core::Engine::EKeyboardKey::K:
        return MakeStringView("K");
    case Core::Engine::EKeyboardKey::L:
        return MakeStringView("L");
    case Core::Engine::EKeyboardKey::M:
        return MakeStringView("M");
    case Core::Engine::EKeyboardKey::N:
        return MakeStringView("N");
    case Core::Engine::EKeyboardKey::O:
        return MakeStringView("O");
    case Core::Engine::EKeyboardKey::P:
        return MakeStringView("P");
    case Core::Engine::EKeyboardKey::Q:
        return MakeStringView("Q");
    case Core::Engine::EKeyboardKey::R:
        return MakeStringView("R");
    case Core::Engine::EKeyboardKey::S:
        return MakeStringView("S");
    case Core::Engine::EKeyboardKey::T:
        return MakeStringView("T");
    case Core::Engine::EKeyboardKey::U:
        return MakeStringView("U");
    case Core::Engine::EKeyboardKey::V:
        return MakeStringView("V");
    case Core::Engine::EKeyboardKey::W:
        return MakeStringView("W");
    case Core::Engine::EKeyboardKey::X:
        return MakeStringView("X");
    case Core::Engine::EKeyboardKey::Y:
        return MakeStringView("Y");
    case Core::Engine::EKeyboardKey::Z:
        return MakeStringView("Z");
    case Core::Engine::EKeyboardKey::Numpad0:
        return MakeStringView("Numpad0");
    case Core::Engine::EKeyboardKey::Numpad1:
        return MakeStringView("Numpad1");
    case Core::Engine::EKeyboardKey::Numpad2:
        return MakeStringView("Numpad2");
    case Core::Engine::EKeyboardKey::Numpad3:
        return MakeStringView("Numpad3");
    case Core::Engine::EKeyboardKey::Numpad4:
        return MakeStringView("Numpad4");
    case Core::Engine::EKeyboardKey::Numpad5:
        return MakeStringView("Numpad5");
    case Core::Engine::EKeyboardKey::Numpad6:
        return MakeStringView("Numpad6");
    case Core::Engine::EKeyboardKey::Numpad7:
        return MakeStringView("Numpad7");
    case Core::Engine::EKeyboardKey::Numpad8:
        return MakeStringView("Numpad8");
    case Core::Engine::EKeyboardKey::Numpad9:
        return MakeStringView("Numpad9");
    case Core::Engine::EKeyboardKey::Add:
        return MakeStringView("Add");
    case Core::Engine::EKeyboardKey::Subtract:
        return MakeStringView("Subtract");
    case Core::Engine::EKeyboardKey::Multiply:
        return MakeStringView("Multiply");
    case Core::Engine::EKeyboardKey::Divide:
        return MakeStringView("Divide");
    case Core::Engine::EKeyboardKey::F1:
        return MakeStringView("F1");
    case Core::Engine::EKeyboardKey::F2:
        return MakeStringView("F2");
    case Core::Engine::EKeyboardKey::F3:
        return MakeStringView("F3");
    case Core::Engine::EKeyboardKey::F4:
        return MakeStringView("F4");
    case Core::Engine::EKeyboardKey::F5:
        return MakeStringView("F5");
    case Core::Engine::EKeyboardKey::F6:
        return MakeStringView("F6");
    case Core::Engine::EKeyboardKey::F7:
        return MakeStringView("F7");
    case Core::Engine::EKeyboardKey::F8:
        return MakeStringView("F8");
    case Core::Engine::EKeyboardKey::F9:
        return MakeStringView("F9");
    case Core::Engine::EKeyboardKey::F10:
        return MakeStringView("F10");
    case Core::Engine::EKeyboardKey::F11:
        return MakeStringView("F11");
    case Core::Engine::EKeyboardKey::F12:
        return MakeStringView("F12");
    case Core::Engine::EKeyboardKey::Up:
        return MakeStringView("Up");
    case Core::Engine::EKeyboardKey::Down:
        return MakeStringView("Down");
    case Core::Engine::EKeyboardKey::Left:
        return MakeStringView("Left");
    case Core::Engine::EKeyboardKey::Right:
        return MakeStringView("Right");
    case Core::Engine::EKeyboardKey::Escape:
        return MakeStringView("Escape");
    case Core::Engine::EKeyboardKey::Space:
        return MakeStringView("Space");
    case Core::Engine::EKeyboardKey::Pause:
        return MakeStringView("Pause");
    case Core::Engine::EKeyboardKey::PrintScreen:
        return MakeStringView("PrintScreen");
    case Core::Engine::EKeyboardKey::ScrollLock:
        return MakeStringView("ScrollLock");
    case Core::Engine::EKeyboardKey::Backspace:
        return MakeStringView("Backspace");
    case Core::Engine::EKeyboardKey::Enter:
        return MakeStringView("Enter");
    case Core::Engine::EKeyboardKey::Tab:
        return MakeStringView("Tab");
    case Core::Engine::EKeyboardKey::Home:
        return MakeStringView("Home");
    case Core::Engine::EKeyboardKey::End:
        return MakeStringView("End");
    case Core::Engine::EKeyboardKey::Insert:
        return MakeStringView("Insert");
    case Core::Engine::EKeyboardKey::Delete:
        return MakeStringView("Delete");
    case Core::Engine::EKeyboardKey::PageUp:
        return MakeStringView("PageUp");
    case Core::Engine::EKeyboardKey::PageDown:
        return MakeStringView("PageDown");
    case Core::Engine::EKeyboardKey::Alt:
        return MakeStringView("Alt");
    case Core::Engine::EKeyboardKey::Menu:
        return MakeStringView("Menu");
    case Core::Engine::EKeyboardKey::Control:
        return MakeStringView("Control");
    case Core::Engine::EKeyboardKey::Shift:
        return MakeStringView("Shift");
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
TMemoryView<const EKeyboardKey> EachKeyboardKeys() {
    return MakeView(gEachKeyboardKeys);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
