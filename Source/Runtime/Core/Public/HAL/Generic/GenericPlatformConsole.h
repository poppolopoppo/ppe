#pragma once

#include "HAL/TargetPlatform.h"
#include "IO/String_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformConsole {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasConsole, false);

    enum EAttribute : unsigned short {
        FG_BLUE         = 0x0001, // text color contains blue.
        FG_GREEN        = 0x0002, // text color contains green.
        FG_RED          = 0x0004, // text color contains red.
        FG_INTENSITY    = 0x0008, // text color is intensified.

        BG_BLUE         = 0x0010, // background color contains blue.
        BG_GREEN        = 0x0020, // background color contains green.
        BG_RED          = 0x0040, // background color contains red.
        BG_INTENSITY    = 0x0080, // background color is intensified.

        Default         = FG_BLUE | FG_GREEN | FG_RED, // gray on black

        FG_BLACK        = 0,
        FG_WHITE        = FG_RED | FG_GREEN | FG_BLUE,
        FG_YELLOW       = FG_RED | FG_GREEN,
        FG_CYAN         = FG_GREEN | FG_BLUE,

        BG_BLACK        = 0,
        BG_WHITE        = BG_RED | BG_GREEN | BG_BLUE,
        BG_YELLOW       = BG_RED | BG_GREEN,
        BG_CYAN         = BG_GREEN | BG_BLUE,

        WHITE_ON_BLACK  = FG_WHITE | BG_BLACK,
        BLACK_ON_WHITE  = BG_WHITE | FG_BLACK,

    };
    ENUM_FLAGS_FRIEND(EAttribute);

    static void Open() = delete;
    static void Close() = delete;

    static size_t Read(const TMemoryView<char>& buffer) = delete;
    static size_t Read(const TMemoryView<wchar_t>& buffer) = delete;

    static void Write(const FStringView& text, EAttribute attrs = Default) = delete;
    static void Write(const FWStringView& text, EAttribute attrs = Default) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
