#pragma once

#include "Application_fwd.h"

#include "IO/StringView.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EKeyboardKey : u8 {
    // TNumeric

    _0  = '0',
    _1  = '1',
    _2  = '2',
    _3  = '3',
    _4  = '4',
    _5  = '5',
    _6  = '6',
    _7  = '7',
    _8  = '8',
    _9  = '9',

    // Alpha

    A   = 'A',
    B   = 'B',
    C   = 'C',
    D   = 'D',
    E   = 'E',
    F   = 'F',
    G   = 'G',
    H   = 'H',
    I   = 'I',
    J   = 'J',
    K   = 'K',
    L   = 'L',
    M   = 'M',
    N   = 'N',
    O   = 'O',
    P   = 'P',
    Q   = 'Q',
    R   = 'R',
    S   = 'S',
    _T   = 'T',
    U   = 'U',
    V   = 'V',
    W   = 'W',
    X   = 'X',
    Y   = 'Y',
    Z   = 'Z',

    // Numpad

    Numpad0 = 0,
    Numpad1,
    Numpad2,
    Numpad3,
    Numpad4,
    Numpad5,
    Numpad6,
    Numpad7,
    Numpad8,
    Numpad9,

    // Function

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    // Specials

    Escape  = 'Z' + 1,
    Space,

    Pause,
    PrintScreen,
    ScrollLock,

    Backspace,
    Enter,
    Tab,

    Home,
    End,
    Insert,
    Delete,
    PageUp,
    PageDown,

    Comma,              // ,
    Equals,             // =
    Plus,               // +
    Minus,              // -
    Period,             // .
    Semicolon,          // ;
    Underscore,         // _

    Ampersand,          // &
    Apostrophe,         // '
    Asterix,            // *
    Caret,              // ^
    Colon,              // :
    Dollar,             // $
    Exclamation,        // !
    Tilde,              // ~
    Quote,              // "

    Slash,              // /
    Backslash,          //
    LeftBracket,        // [
    RightBracket,       // ]
    LeftParantheses,    // (
    RightParantheses,   // )

    // Directions

    LeftArrow,
    RightArrow,
    UpArrow,
    DownArrow,

    // Specials

    CapsLock,
    NumLock,

    // Modifiers

    LeftAlt,
    RightAlt,
    LeftControl,
    RightControl,
    LeftShift,
    RightShift,

    LeftSuper, // windows key / apple command
    RightSuper,
};
//----------------------------------------------------------------------------
NODISCARD PPE_APPLICATION_API TMemoryView<const EKeyboardKey> EachKeyboardKeys() NOEXCEPT;
NODISCARD PPE_APPLICATION_API FStringLiteral KeyboardKeyToCStr(EKeyboardKey value) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, EKeyboardKey value) {
    return oss << KeyboardKeyToCStr(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
