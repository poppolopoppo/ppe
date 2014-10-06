#pragma once

#include "Application.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class KeyboardKey : u32 {
    // Numeric

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
    T   = 'T',
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

    Add,
    Substract,
    Multiply,
    Divide,

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

    // Direction

    Up,
    Down,
    Left,
    Right,

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

    // Modifiers

    Alt,
    Menu,           // Windows key
    Control,
    Shift,
};
//----------------------------------------------------------------------------
const char *KeyboardKeyToCStr(KeyboardKey value);
//----------------------------------------------------------------------------
MemoryView<const KeyboardKey> EachKeyboardKeys();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
