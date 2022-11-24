// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
    auto stringStyle = FPlatformConsole::BG_BLACK | FPlatformConsole::FG_YELLOW;
    auto literalStyle = FPlatformConsole::BG_BLACK | FPlatformConsole::FG_GREEN | FPlatformConsole::FG_INTENSITY;

    if (BitAnd(base, FPlatformConsole::_BG_MASK) != FPlatformConsole::BG_BLACK) {
        stringStyle = ((stringStyle - FPlatformConsole::_BG_MASK) | BitAnd(base, FPlatformConsole::_BG_MASK));
        literalStyle = ((literalStyle - FPlatformConsole::_BG_MASK) | BitAnd(base, FPlatformConsole::_BG_MASK));
    }

    auto style = base;

    static constexpr FPlatformConsole::EAttribute fg_rainbow[] = {
        FPlatformConsole::FG_CYAN,
        FPlatformConsole::FG_BLUE,
        FPlatformConsole::FG_MAGENTA,
        FPlatformConsole::FG_RED,
        FPlatformConsole::FG_YELLOW,
        FPlatformConsole::FG_GREEN,
    };

    bool allnum = true;
    int balance = 0;
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
        case STRING_LITERAL(_Char, '#'):
        case STRING_LITERAL(_Char, '$'):
        case STRING_LITERAL(_Char, '='):
        case STRING_LITERAL(_Char, ':'):
        case STRING_LITERAL(_Char, '\\'):
            style = FPlatformConsole::BG_BLACK | FPlatformConsole::FG_CYAN | FPlatformConsole::FG_INTENSITY; break;
        case STRING_LITERAL(_Char, '+'):
        case STRING_LITERAL(_Char, '-'):
        case STRING_LITERAL(_Char, '*'):
        case STRING_LITERAL(_Char, '/'):
        case STRING_LITERAL(_Char, '|'):
            style = FPlatformConsole::BG_BLACK | FPlatformConsole::FG_YELLOW | FPlatformConsole::FG_INTENSITY; break;
        case STRING_LITERAL(_Char, '<'):
        case STRING_LITERAL(_Char, '('):
        case STRING_LITERAL(_Char, '{'):
        case STRING_LITERAL(_Char, '['):
            style = fg_rainbow[++balance % lengthof(fg_rainbow)] | FPlatformConsole::BG_BLACK | FPlatformConsole::FG_INTENSITY; break;
        case STRING_LITERAL(_Char, '>'):
        case STRING_LITERAL(_Char, ')'):
        case STRING_LITERAL(_Char, '}'):
        case STRING_LITERAL(_Char, ']'):
            style = fg_rainbow[balance-- % lengthof(fg_rainbow)] | FPlatformConsole::BG_BLACK | FPlatformConsole::FG_INTENSITY; break;
        default:
            style = FPlatformConsole::BG_BLACK | FPlatformConsole::FG_WHITE | FPlatformConsole::FG_INTENSITY; break;
        }

        if (BitAnd(style, FPlatformConsole::_BG_MASK) != BitAnd(base, FPlatformConsole::_BG_MASK) &&
            BitAnd(base, FPlatformConsole::_BG_MASK) != FPlatformConsole::BG_BLACK) {
            style = ((style - FPlatformConsole::_BG_MASK) | BitAnd(base, FPlatformConsole::_BG_MASK));
        }

        FPlatformConsole::Write(str.SubRange(i, 1), style);

        style = base;
    }

    if (s > 0) // still handling unbalanced parentheses, but the coloring will be wrong, obviously
        FPlatformConsole::Write(str.CutStartingAt(s), style);
    else if (o != str.size())
        FPlatformConsole::Write(str.CutStartingAt(o), style);
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
