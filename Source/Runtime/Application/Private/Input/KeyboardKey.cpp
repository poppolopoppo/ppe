// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/KeyboardKey.h"

namespace PPE {
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

    EKeyboardKey::Comma,             // ,
    EKeyboardKey::Plus,              // +
    EKeyboardKey::Minus,             // -
    EKeyboardKey::Period,            // .

    EKeyboardKey::CapsLock,
    EKeyboardKey::NumLock,

    // Modifiers

    EKeyboardKey::Alt,
    EKeyboardKey::Control,
    EKeyboardKey::Shift,

    EKeyboardKey::Menu,
    EKeyboardKey::Super,
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView KeyboardKeyToCStr(EKeyboardKey value) {
    switch (value)
    {
    case PPE::Application::EKeyboardKey::_0:
        return MakeStringView("0");
    case PPE::Application::EKeyboardKey::_1:
        return MakeStringView("1");
    case PPE::Application::EKeyboardKey::_2:
        return MakeStringView("2");
    case PPE::Application::EKeyboardKey::_3:
        return MakeStringView("3");
    case PPE::Application::EKeyboardKey::_4:
        return MakeStringView("4");
    case PPE::Application::EKeyboardKey::_5:
        return MakeStringView("5");
    case PPE::Application::EKeyboardKey::_6:
        return MakeStringView("6");
    case PPE::Application::EKeyboardKey::_7:
        return MakeStringView("7");
    case PPE::Application::EKeyboardKey::_8:
        return MakeStringView("8");
    case PPE::Application::EKeyboardKey::_9:
        return MakeStringView("9");
    case PPE::Application::EKeyboardKey::A:
        return MakeStringView("A");
    case PPE::Application::EKeyboardKey::B:
        return MakeStringView("B");
    case PPE::Application::EKeyboardKey::C:
        return MakeStringView("C");
    case PPE::Application::EKeyboardKey::D:
        return MakeStringView("D");
    case PPE::Application::EKeyboardKey::E:
        return MakeStringView("E");
    case PPE::Application::EKeyboardKey::F:
        return MakeStringView("F");
    case PPE::Application::EKeyboardKey::G:
        return MakeStringView("G");
    case PPE::Application::EKeyboardKey::H:
        return MakeStringView("H");
    case PPE::Application::EKeyboardKey::I:
        return MakeStringView("I");
    case PPE::Application::EKeyboardKey::J:
        return MakeStringView("J");
    case PPE::Application::EKeyboardKey::K:
        return MakeStringView("K");
    case PPE::Application::EKeyboardKey::L:
        return MakeStringView("L");
    case PPE::Application::EKeyboardKey::M:
        return MakeStringView("M");
    case PPE::Application::EKeyboardKey::N:
        return MakeStringView("N");
    case PPE::Application::EKeyboardKey::O:
        return MakeStringView("O");
    case PPE::Application::EKeyboardKey::P:
        return MakeStringView("P");
    case PPE::Application::EKeyboardKey::Q:
        return MakeStringView("Q");
    case PPE::Application::EKeyboardKey::R:
        return MakeStringView("R");
    case PPE::Application::EKeyboardKey::S:
        return MakeStringView("S");
    case PPE::Application::EKeyboardKey::T:
        return MakeStringView("T");
    case PPE::Application::EKeyboardKey::U:
        return MakeStringView("U");
    case PPE::Application::EKeyboardKey::V:
        return MakeStringView("V");
    case PPE::Application::EKeyboardKey::W:
        return MakeStringView("W");
    case PPE::Application::EKeyboardKey::X:
        return MakeStringView("X");
    case PPE::Application::EKeyboardKey::Y:
        return MakeStringView("Y");
    case PPE::Application::EKeyboardKey::Z:
        return MakeStringView("Z");
    case PPE::Application::EKeyboardKey::Numpad0:
        return MakeStringView("Numpad0");
    case PPE::Application::EKeyboardKey::Numpad1:
        return MakeStringView("Numpad1");
    case PPE::Application::EKeyboardKey::Numpad2:
        return MakeStringView("Numpad2");
    case PPE::Application::EKeyboardKey::Numpad3:
        return MakeStringView("Numpad3");
    case PPE::Application::EKeyboardKey::Numpad4:
        return MakeStringView("Numpad4");
    case PPE::Application::EKeyboardKey::Numpad5:
        return MakeStringView("Numpad5");
    case PPE::Application::EKeyboardKey::Numpad6:
        return MakeStringView("Numpad6");
    case PPE::Application::EKeyboardKey::Numpad7:
        return MakeStringView("Numpad7");
    case PPE::Application::EKeyboardKey::Numpad8:
        return MakeStringView("Numpad8");
    case PPE::Application::EKeyboardKey::Numpad9:
        return MakeStringView("Numpad9");
    case PPE::Application::EKeyboardKey::Add:
        return MakeStringView("Add");
    case PPE::Application::EKeyboardKey::Subtract:
        return MakeStringView("Subtract");
    case PPE::Application::EKeyboardKey::Multiply:
        return MakeStringView("Multiply");
    case PPE::Application::EKeyboardKey::Divide:
        return MakeStringView("Divide");
    case PPE::Application::EKeyboardKey::F1:
        return MakeStringView("F1");
    case PPE::Application::EKeyboardKey::F2:
        return MakeStringView("F2");
    case PPE::Application::EKeyboardKey::F3:
        return MakeStringView("F3");
    case PPE::Application::EKeyboardKey::F4:
        return MakeStringView("F4");
    case PPE::Application::EKeyboardKey::F5:
        return MakeStringView("F5");
    case PPE::Application::EKeyboardKey::F6:
        return MakeStringView("F6");
    case PPE::Application::EKeyboardKey::F7:
        return MakeStringView("F7");
    case PPE::Application::EKeyboardKey::F8:
        return MakeStringView("F8");
    case PPE::Application::EKeyboardKey::F9:
        return MakeStringView("F9");
    case PPE::Application::EKeyboardKey::F10:
        return MakeStringView("F10");
    case PPE::Application::EKeyboardKey::F11:
        return MakeStringView("F11");
    case PPE::Application::EKeyboardKey::F12:
        return MakeStringView("F12");
    case PPE::Application::EKeyboardKey::Up:
        return MakeStringView("Up");
    case PPE::Application::EKeyboardKey::Down:
        return MakeStringView("Down");
    case PPE::Application::EKeyboardKey::Left:
        return MakeStringView("Left");
    case PPE::Application::EKeyboardKey::Right:
        return MakeStringView("Right");
    case PPE::Application::EKeyboardKey::Escape:
        return MakeStringView("Escape");
    case PPE::Application::EKeyboardKey::Space:
        return MakeStringView("Space");
    case PPE::Application::EKeyboardKey::Pause:
        return MakeStringView("Pause");
    case PPE::Application::EKeyboardKey::PrintScreen:
        return MakeStringView("PrintScreen");
    case PPE::Application::EKeyboardKey::ScrollLock:
        return MakeStringView("ScrollLock");
    case PPE::Application::EKeyboardKey::Backspace:
        return MakeStringView("Backspace");
    case PPE::Application::EKeyboardKey::Enter:
        return MakeStringView("Enter");
    case PPE::Application::EKeyboardKey::Tab:
        return MakeStringView("Tab");
    case PPE::Application::EKeyboardKey::Home:
        return MakeStringView("Home");
    case PPE::Application::EKeyboardKey::End:
        return MakeStringView("End");
    case PPE::Application::EKeyboardKey::Insert:
        return MakeStringView("Insert");
    case PPE::Application::EKeyboardKey::Delete:
        return MakeStringView("Delete");
    case PPE::Application::EKeyboardKey::PageUp:
        return MakeStringView("PageUp");
    case PPE::Application::EKeyboardKey::PageDown:
        return MakeStringView("PageDown");
    case PPE::Application::EKeyboardKey::Alt:
        return MakeStringView("Alt");
    case PPE::Application::EKeyboardKey::Menu:
        return MakeStringView("Menu");
    case PPE::Application::EKeyboardKey::Control:
        return MakeStringView("Control");
    case PPE::Application::EKeyboardKey::Shift:
        return MakeStringView("Shift");
    case EKeyboardKey::Comma:
        return MakeStringView("Comma");
    case EKeyboardKey::Plus:
        return MakeStringView("Plus");
    case EKeyboardKey::Minus:
        return MakeStringView("Minus");
    case EKeyboardKey::Period:
        return MakeStringView("Period");
    case EKeyboardKey::CapsLock:
        return MakeStringView("CapsLock");
    case EKeyboardKey::NumLock:
        return MakeStringView("NumLock");
    case EKeyboardKey::Super:
        return MakeStringView("Super");
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
TMemoryView<const EKeyboardKey> EachKeyboardKeys() {
    return MakeView(GEachKeyboardKeys);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
