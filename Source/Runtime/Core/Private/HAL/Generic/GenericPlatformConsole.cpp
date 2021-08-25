#include "stdafx.h"

#include "HAL/Generic/GenericPlatformConsole.h"

#include "IO/StringView.h"
#include "HAL/PlatformConsole.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
static void SyntaxicHighlight_(const TBasicStringView<_Char>& str, FPlatformConsole::EAttribute base) {
    constexpr auto stringStyle = FPlatformConsole::BG_BLACK | FPlatformConsole::FG_YELLOW;
    constexpr auto literalStyle = FPlatformConsole::BG_BLACK | FPlatformConsole::FG_RED | FPlatformConsole::FG_BLUE | FPlatformConsole::FG_INTENSITY;

    auto style = base;

    bool allnum = true;
    size_t o = 0;
    size_t s = 0;
    forrange(i, 0, str.size()) {
        const _Char ch = str[i];

        if (s) {
            if (ch != STRING_LITERAL(_Char, '"'))
                continue;
            FPlatformConsole::Write(str.SubRange(s - 1, 2 + i - s), style);
            s = 0;
            o = i + 1;
            allnum = true;
            continue;
        }
        else if (IsAlnum(ch) || ch == STRING_LITERAL(_Char, '_')) {
            allnum &= IsDigit(ch);
            continue;
        }
        else if (ch == STRING_LITERAL(_Char, '"')) {
            allnum = false;
            s = i + 1;
            style = stringStyle;
            continue;
        }

        if (o != i) {
            FPlatformConsole::Write(str.SubRange(o, i - o), allnum ? literalStyle : style);
            style = base;
            allnum = true;
        }

        o = i + 1;

        switch (ch) {
        case STRING_LITERAL(_Char, '$'):
        case STRING_LITERAL(_Char, '='):
        case STRING_LITERAL(_Char, ':'):
            style = FPlatformConsole::BG_BLACK | FPlatformConsole::FG_GREEN | FPlatformConsole::FG_INTENSITY; break;
        case STRING_LITERAL(_Char, '+'):
        case STRING_LITERAL(_Char, '-'):
        case STRING_LITERAL(_Char, '*'):
        case STRING_LITERAL(_Char, '/'):
        case STRING_LITERAL(_Char, '<'):
        case STRING_LITERAL(_Char, '>'):
        case STRING_LITERAL(_Char, '|'):
            style = FPlatformConsole::BG_BLACK | FPlatformConsole::FG_RED | FPlatformConsole::FG_INTENSITY; break;
        case STRING_LITERAL(_Char, '('):
        case STRING_LITERAL(_Char, ')'):
        case STRING_LITERAL(_Char, '{'):
        case STRING_LITERAL(_Char, '}'):
        case STRING_LITERAL(_Char, '['):
        case STRING_LITERAL(_Char, ']'):
        case STRING_LITERAL(_Char, '\\'):
            style = FPlatformConsole::BG_BLACK | FPlatformConsole::FG_CYAN | FPlatformConsole::FG_INTENSITY; break;
        default:
            style = FPlatformConsole::BG_BLACK | FPlatformConsole::FG_WHITE | FPlatformConsole::FG_INTENSITY; break;
        }

        FPlatformConsole::Write(str.SubRange(i, 1), style);

        style = base;
    }
    Assert_NoAssume(0 == s);

    if (o != str.size())
        FPlatformConsole::Write(str.SubRange(o, str.size() - o), style);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FGenericPlatformConsole::SyntaxicHighlight(const FStringView& str, EAttribute base/* = Default */) {
    SyntaxicHighlight_(str, base);
}
//----------------------------------------------------------------------------
void FGenericPlatformConsole::SyntaxicHighlight(const FWStringView& str, EAttribute base/* = Default */) {
    SyntaxicHighlight_(str, base);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
