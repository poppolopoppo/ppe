// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "KeyboardKey.h"

namespace Core {
namespace Engine {
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
    case PPE::Engine::EKeyboardKey::_0:
        return MakeStringView("0");
    case PPE::Engine::EKeyboardKey::_1:
        return MakeStringView("1");
    case PPE::Engine::EKeyboardKey::_2:
        return MakeStringView("2");
    case PPE::Engine::EKeyboardKey::_3:
        return MakeStringView("3");
    case PPE::Engine::EKeyboardKey::_4:
        return MakeStringView("4");
    case PPE::Engine::EKeyboardKey::_5:
        return MakeStringView("5");
    case PPE::Engine::EKeyboardKey::_6:
        return MakeStringView("6");
    case PPE::Engine::EKeyboardKey::_7:
        return MakeStringView("7");
    case PPE::Engine::EKeyboardKey::_8:
        return MakeStringView("8");
    case PPE::Engine::EKeyboardKey::_9:
        return MakeStringView("9");
    case PPE::Engine::EKeyboardKey::A:
        return MakeStringView("A");
    case PPE::Engine::EKeyboardKey::B:
        return MakeStringView("B");
    case PPE::Engine::EKeyboardKey::C:
        return MakeStringView("C");
    case PPE::Engine::EKeyboardKey::D:
        return MakeStringView("D");
    case PPE::Engine::EKeyboardKey::E:
        return MakeStringView("E");
    case PPE::Engine::EKeyboardKey::F:
        return MakeStringView("F");
    case PPE::Engine::EKeyboardKey::G:
        return MakeStringView("G");
    case PPE::Engine::EKeyboardKey::H:
        return MakeStringView("H");
    case PPE::Engine::EKeyboardKey::I:
        return MakeStringView("I");
    case PPE::Engine::EKeyboardKey::J:
        return MakeStringView("J");
    case PPE::Engine::EKeyboardKey::K:
        return MakeStringView("K");
    case PPE::Engine::EKeyboardKey::L:
        return MakeStringView("L");
    case PPE::Engine::EKeyboardKey::M:
        return MakeStringView("M");
    case PPE::Engine::EKeyboardKey::N:
        return MakeStringView("N");
    case PPE::Engine::EKeyboardKey::O:
        return MakeStringView("O");
    case PPE::Engine::EKeyboardKey::P:
        return MakeStringView("P");
    case PPE::Engine::EKeyboardKey::Q:
        return MakeStringView("Q");
    case PPE::Engine::EKeyboardKey::R:
        return MakeStringView("R");
    case PPE::Engine::EKeyboardKey::S:
        return MakeStringView("S");
    case PPE::Engine::EKeyboardKey::T:
        return MakeStringView("T");
    case PPE::Engine::EKeyboardKey::U:
        return MakeStringView("U");
    case PPE::Engine::EKeyboardKey::V:
        return MakeStringView("V");
    case PPE::Engine::EKeyboardKey::W:
        return MakeStringView("W");
    case PPE::Engine::EKeyboardKey::X:
        return MakeStringView("X");
    case PPE::Engine::EKeyboardKey::Y:
        return MakeStringView("Y");
    case PPE::Engine::EKeyboardKey::Z:
        return MakeStringView("Z");
    case PPE::Engine::EKeyboardKey::Numpad0:
        return MakeStringView("Numpad0");
    case PPE::Engine::EKeyboardKey::Numpad1:
        return MakeStringView("Numpad1");
    case PPE::Engine::EKeyboardKey::Numpad2:
        return MakeStringView("Numpad2");
    case PPE::Engine::EKeyboardKey::Numpad3:
        return MakeStringView("Numpad3");
    case PPE::Engine::EKeyboardKey::Numpad4:
        return MakeStringView("Numpad4");
    case PPE::Engine::EKeyboardKey::Numpad5:
        return MakeStringView("Numpad5");
    case PPE::Engine::EKeyboardKey::Numpad6:
        return MakeStringView("Numpad6");
    case PPE::Engine::EKeyboardKey::Numpad7:
        return MakeStringView("Numpad7");
    case PPE::Engine::EKeyboardKey::Numpad8:
        return MakeStringView("Numpad8");
    case PPE::Engine::EKeyboardKey::Numpad9:
        return MakeStringView("Numpad9");
    case PPE::Engine::EKeyboardKey::Add:
        return MakeStringView("Add");
    case PPE::Engine::EKeyboardKey::Subtract:
        return MakeStringView("Subtract");
    case PPE::Engine::EKeyboardKey::Multiply:
        return MakeStringView("Multiply");
    case PPE::Engine::EKeyboardKey::Divide:
        return MakeStringView("Divide");
    case PPE::Engine::EKeyboardKey::F1:
        return MakeStringView("F1");
    case PPE::Engine::EKeyboardKey::F2:
        return MakeStringView("F2");
    case PPE::Engine::EKeyboardKey::F3:
        return MakeStringView("F3");
    case PPE::Engine::EKeyboardKey::F4:
        return MakeStringView("F4");
    case PPE::Engine::EKeyboardKey::F5:
        return MakeStringView("F5");
    case PPE::Engine::EKeyboardKey::F6:
        return MakeStringView("F6");
    case PPE::Engine::EKeyboardKey::F7:
        return MakeStringView("F7");
    case PPE::Engine::EKeyboardKey::F8:
        return MakeStringView("F8");
    case PPE::Engine::EKeyboardKey::F9:
        return MakeStringView("F9");
    case PPE::Engine::EKeyboardKey::F10:
        return MakeStringView("F10");
    case PPE::Engine::EKeyboardKey::F11:
        return MakeStringView("F11");
    case PPE::Engine::EKeyboardKey::F12:
        return MakeStringView("F12");
    case PPE::Engine::EKeyboardKey::Up:
        return MakeStringView("Up");
    case PPE::Engine::EKeyboardKey::Down:
        return MakeStringView("Down");
    case PPE::Engine::EKeyboardKey::Left:
        return MakeStringView("Left");
    case PPE::Engine::EKeyboardKey::Right:
        return MakeStringView("Right");
    case PPE::Engine::EKeyboardKey::Escape:
        return MakeStringView("Escape");
    case PPE::Engine::EKeyboardKey::Space:
        return MakeStringView("Space");
    case PPE::Engine::EKeyboardKey::Pause:
        return MakeStringView("Pause");
    case PPE::Engine::EKeyboardKey::PrintScreen:
        return MakeStringView("PrintScreen");
    case PPE::Engine::EKeyboardKey::ScrollLock:
        return MakeStringView("ScrollLock");
    case PPE::Engine::EKeyboardKey::Backspace:
        return MakeStringView("Backspace");
    case PPE::Engine::EKeyboardKey::Enter:
        return MakeStringView("Enter");
    case PPE::Engine::EKeyboardKey::Tab:
        return MakeStringView("Tab");
    case PPE::Engine::EKeyboardKey::Home:
        return MakeStringView("Home");
    case PPE::Engine::EKeyboardKey::End:
        return MakeStringView("End");
    case PPE::Engine::EKeyboardKey::Insert:
        return MakeStringView("Insert");
    case PPE::Engine::EKeyboardKey::Delete:
        return MakeStringView("Delete");
    case PPE::Engine::EKeyboardKey::PageUp:
        return MakeStringView("PageUp");
    case PPE::Engine::EKeyboardKey::PageDown:
        return MakeStringView("PageDown");
    case PPE::Engine::EKeyboardKey::Alt:
        return MakeStringView("Alt");
    case PPE::Engine::EKeyboardKey::Menu:
        return MakeStringView("Menu");
    case PPE::Engine::EKeyboardKey::Control:
        return MakeStringView("Control");
    case PPE::Engine::EKeyboardKey::Shift:
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
