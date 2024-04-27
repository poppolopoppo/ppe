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
    EKeyboardKey::_T,
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

    EKeyboardKey::CapsLock,
    EKeyboardKey::NumLock,

    EKeyboardKey::Comma,             // ,
    EKeyboardKey::Equals,             // ,
    EKeyboardKey::Plus,              // +
    EKeyboardKey::Minus,             // -
    EKeyboardKey::Period,            // .
    EKeyboardKey::Semicolon,         // ;
    EKeyboardKey::Underscore,         // ;

    EKeyboardKey::Ampersand,          // &
    EKeyboardKey::Apostrophe,         // '
    EKeyboardKey::Asterix,            // *
    EKeyboardKey::Caret,              // ^
    EKeyboardKey::Colon,              // :
    EKeyboardKey::Dollar,             // $
    EKeyboardKey::Exclamation,        // !
    EKeyboardKey::Tilde,              // ~
    EKeyboardKey::Quote,              // "

    EKeyboardKey::Slash,              // /
    EKeyboardKey::Backslash,          //
    EKeyboardKey::LeftBracket,        // [
    EKeyboardKey::RightBracket,       // ]
    EKeyboardKey::LeftParantheses,    // (
    EKeyboardKey::RightParantheses,   // )

    // Directions

    EKeyboardKey::LeftArrow,
    EKeyboardKey::RightArrow,
    EKeyboardKey::UpArrow,
    EKeyboardKey::DownArrow,

    // Modifiers

    EKeyboardKey::LeftAlt,
    EKeyboardKey::RightAlt,
    EKeyboardKey::LeftControl,
    EKeyboardKey::RightControl,
    EKeyboardKey::LeftShift,
    EKeyboardKey::RightShift,
    EKeyboardKey::LeftSuper,
    EKeyboardKey::RightSuper,
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringLiteral KeyboardKeyToCStr(EKeyboardKey value) NOEXCEPT {
    switch (value) {
    case EKeyboardKey::_0:
        return "0";
    case EKeyboardKey::_1:
        return "1";
    case EKeyboardKey::_2:
        return "2";
    case EKeyboardKey::_3:
        return "3";
    case EKeyboardKey::_4:
        return "4";
    case EKeyboardKey::_5:
        return "5";
    case EKeyboardKey::_6:
        return "6";
    case EKeyboardKey::_7:
        return "7";
    case EKeyboardKey::_8:
        return "8";
    case EKeyboardKey::_9:
        return "9";
    case EKeyboardKey::A:
        return "A";
    case EKeyboardKey::B:
        return "B";
    case EKeyboardKey::C:
        return "C";
    case EKeyboardKey::D:
        return "D";
    case EKeyboardKey::E:
        return "E";
    case EKeyboardKey::F:
        return "F";
    case EKeyboardKey::G:
        return "G";
    case EKeyboardKey::H:
        return "H";
    case EKeyboardKey::I:
        return "I";
    case EKeyboardKey::J:
        return "J";
    case EKeyboardKey::K:
        return "K";
    case EKeyboardKey::L:
        return "L";
    case EKeyboardKey::M:
        return "M";
    case EKeyboardKey::N:
        return "N";
    case EKeyboardKey::O:
        return "O";
    case EKeyboardKey::P:
        return "P";
    case EKeyboardKey::Q:
        return "Q";
    case EKeyboardKey::R:
        return "R";
    case EKeyboardKey::S:
        return "S";
    case EKeyboardKey::_T:
        return "T";
    case EKeyboardKey::U:
        return "U";
    case EKeyboardKey::V:
        return "V";
    case EKeyboardKey::W:
        return "W";
    case EKeyboardKey::X:
        return "X";
    case EKeyboardKey::Y:
        return "Y";
    case EKeyboardKey::Z:
        return "Z";
    case EKeyboardKey::Numpad0:
        return "Numpad0";
    case EKeyboardKey::Numpad1:
        return "Numpad1";
    case EKeyboardKey::Numpad2:
        return "Numpad2";
    case EKeyboardKey::Numpad3:
        return "Numpad3";
    case EKeyboardKey::Numpad4:
        return "Numpad4";
    case EKeyboardKey::Numpad5:
        return "Numpad5";
    case EKeyboardKey::Numpad6:
        return "Numpad6";
    case EKeyboardKey::Numpad7:
        return "Numpad7";
    case EKeyboardKey::Numpad8:
        return "Numpad8";
    case EKeyboardKey::Numpad9:
        return "Numpad9";
    case EKeyboardKey::F1:
        return "F1";
    case EKeyboardKey::F2:
        return "F2";
    case EKeyboardKey::F3:
        return "F3";
    case EKeyboardKey::F4:
        return "F4";
    case EKeyboardKey::F5:
        return "F5";
    case EKeyboardKey::F6:
        return "F6";
    case EKeyboardKey::F7:
        return "F7";
    case EKeyboardKey::F8:
        return "F8";
    case EKeyboardKey::F9:
        return "F9";
    case EKeyboardKey::F10:
        return "F10";
    case EKeyboardKey::F11:
        return "F11";
    case EKeyboardKey::F12:
        return "F12";
    case EKeyboardKey::Escape:
        return "Escape";
    case EKeyboardKey::Space:
        return "Space";
    case EKeyboardKey::Pause:
        return "Pause";
    case EKeyboardKey::PrintScreen:
        return "PrintScreen";
    case EKeyboardKey::ScrollLock:
        return "ScrollLock";
    case EKeyboardKey::Backspace:
        return "Backspace";
    case EKeyboardKey::Enter:
        return "Enter";
    case EKeyboardKey::Tab:
        return "Tab";
    case EKeyboardKey::Home:
        return "Home";
    case EKeyboardKey::End:
        return "End";
    case EKeyboardKey::Insert:
        return "Insert";
    case EKeyboardKey::Delete:
        return "Delete";
    case EKeyboardKey::PageUp:
        return "PageUp";
    case EKeyboardKey::PageDown:
        return "PageDown";
    case EKeyboardKey::LeftAlt: return "LeftAlt";
    case EKeyboardKey::RightAlt: return "RightAlt";
    case EKeyboardKey::LeftSuper:  return "LeftCommand";
    case EKeyboardKey::RightSuper:  return "RightCommand";
    case EKeyboardKey::LeftControl: return "LeftControl";
    case EKeyboardKey::RightControl: return "RightControl";
    case EKeyboardKey::LeftShift: return "LeftShift";
    case EKeyboardKey::RightShift: return "RightShift";
    case EKeyboardKey::Comma:
        return ",";
    case EKeyboardKey::Equals:
        return "=";
    case EKeyboardKey::Plus:
        return "+";
    case EKeyboardKey::Minus:
        return "-";
    case EKeyboardKey::Period:
        return ".";
    case EKeyboardKey::Semicolon:
        return ";";
    case EKeyboardKey::Underscore:
        return "_";
    case EKeyboardKey::CapsLock:
        return "CapsLock";
    case EKeyboardKey::NumLock:
        return "NumLock";
    case EKeyboardKey::LeftArrow: return "LeftArrow";
    case EKeyboardKey::RightArrow: return "RightArrow";
    case EKeyboardKey::UpArrow: return "UpArrow";
    case EKeyboardKey::DownArrow: return "DownArrow";
    case EKeyboardKey::Ampersand: return "&";
    case EKeyboardKey::Apostrophe: return "'";
    case EKeyboardKey::Asterix: return "*";
    case EKeyboardKey::Caret: return "^";
    case EKeyboardKey::Colon: return ":";
    case EKeyboardKey::Dollar: return "$";
    case EKeyboardKey::Exclamation: return "!";
    case EKeyboardKey::Tilde: return "~";
    case EKeyboardKey::Quote: return "\"";
    case EKeyboardKey::Slash: return "/";
    case EKeyboardKey::Backslash: return "\\";
    case EKeyboardKey::LeftBracket: return "[";
    case EKeyboardKey::RightBracket: return "]";
    case EKeyboardKey::LeftParantheses: return "(";
    case EKeyboardKey::RightParantheses: return ")";
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
TMemoryView<const EKeyboardKey> EachKeyboardKeys() NOEXCEPT {
    return MakeView(GEachKeyboardKeys);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
