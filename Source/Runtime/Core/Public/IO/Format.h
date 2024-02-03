#pragma once

#include "Core.h"

#include "IO/String_fwd.h"
#include "IO/TextWriter_fwd.h"
#include "Memory/MemoryView.h"

/*
// Format() usage samples :
// -------------------------
//
//ex1:
//      wchar_t buffer[1024];
//      size_t length = Format(buffer, L"string = {2}, decimal = {0}, float = {1}\n", "test", 42, 0.123456f);
//      std::cout << buffer;
//      ->  L"string = 0.123456, decimal = test, float = 42\n"
//
//ex2:
//      FWString wstr = Format(L"num={0} alphabool={0:a}", true);
//      ->  L"num=1 alphabool=true"
//      Format(std::cout, "truncated = {0:<4}", "1234789");
//      ->  "truncated = 1234"
//
//ex3:
//      Format(std::cout, "trunc = {0:-9}", "1234");
//      ->  "     1234"
//      Format(std::cout, "string = {0:10} {0:-10U}, decimal = {1:8X} {1:#8x}, float = {2:f3} {2:10f4}\n", "test", 0xBADCAFE, -0.123456f);
//      ->  "string =       TEST TEST      , decimal =  BADCAFE 0badcafe, float = -0.123    -0.1235\n"
//
//ex4:
//      Format(std::cout, "{}", '\t\n');
//      ->  "   \
//          "
//      Format(std::cout, "escaped = {:\\}", '\t\n');
//      ->  "escaped = \t\n"
//      Format(std::cout, "escaped and quoted = {:q\\}", '\t\n');
//      ->  "escaped and quoted = \"\t\n\""
//
//ex5:
//      Format(std::cout, "{:*16}\n", '-');
//      ->  "----------------
//          "
//      Format(std::cout, "{:#4*4}\n", 42);
//      ->  "0042004200420042
//          "
//
*/

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
struct TBasicFormatTraits {
    enum EFlags : _Char {
        null =              STRING_LITERAL(_Char, '\0'),
        lbrace =            STRING_LITERAL(_Char, '{'),
        rbrace =            STRING_LITERAL(_Char, '}'),
        colon =             STRING_LITERAL(_Char, ':'),
        multiply =          STRING_LITERAL(_Char, '*'),
        zero =              STRING_LITERAL(_Char, '0'),

        fmt_alpha =         STRING_LITERAL(_Char, 'a'),
        fmt_ALPHA =         STRING_LITERAL(_Char, 'A'),

        fmt_bin =           STRING_LITERAL(_Char, 'b'),
        fmt_BIN =           STRING_LITERAL(_Char, 'B'),

        fmt_dec =           STRING_LITERAL(_Char, 'd'),
        fmt_DEC =           STRING_LITERAL(_Char, 'D'),

        fmt_hex =           STRING_LITERAL(_Char, 'x'),
        fmt_HEX =           STRING_LITERAL(_Char, 'X'),

        fmt_oct =           STRING_LITERAL(_Char, 'o'),
        fmt_OCT =           STRING_LITERAL(_Char, 'O'),

        fmt_fixed =         STRING_LITERAL(_Char, 'f'),
        fmt_FIXED =         STRING_LITERAL(_Char, 'F'),

        fmt_scient =        STRING_LITERAL(_Char, 's'),
        fmt_SCIENT =        STRING_LITERAL(_Char, 'S'),

        fmt_UPPER =         STRING_LITERAL(_Char, 'U'),
        fmt_lower =         STRING_LITERAL(_Char, 'l'),

        fmt_Capital =       STRING_LITERAL(_Char, 'C'),
        fmt_minus =         STRING_LITERAL(_Char, '-'),
        fmt_compact =       STRING_LITERAL(_Char, '_'),
        fmt_NONCOMPACT =    STRING_LITERAL(_Char, '^'),

        fmt_zeropad =       STRING_LITERAL(_Char, '#'),
        fmt_center =        STRING_LITERAL(_Char, '@'),
        fmt_truncR =        STRING_LITERAL(_Char, '<'),
        fmt_truncL =        STRING_LITERAL(_Char, '>'),

        fmt_escape =        STRING_LITERAL(_Char, '\\'),
        fmt_quote =         STRING_LITERAL(_Char, 'q'),
    };

    static CONSTEXPR bool IsValidManipFlag(_Char ch);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define INLINE_FORMAT(_CAPACITY, _FORMAT, ...) \
    ::PPE::InlineFormat(INLINE_MALLOCA(char, _CAPACITY).MakeView(), (_FORMAT), __VA_ARGS__)
//----------------------------------------------------------------------------
#define INLINE_WFORMAT(_CAPACITY, _FORMAT, ...) \
    ::PPE::InlineFormat(INLINE_MALLOCA(wchar_t, _CAPACITY).MakeView(), (_FORMAT), __VA_ARGS__)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& Format(FTextWriter& oss, FStringLiteral str);
PPE_CORE_API FWTextWriter& Format(FWTextWriter& oss, FWStringLiteral wstr);
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
size_t Format(const TMemoryView<_Char>& dst, TBasicStringLiteral<_Char> format, _Arg0&& arg0, _Args&&... args);
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
void Format(TBasicString<_Char>& dst, TBasicStringLiteral<_Char> format, _Arg0&& arg0, _Args&&... args);
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity, typename _Arg0, typename... _Args>
const _Char* Format(TBasicStaticString<_Char, _Capacity>& dst, TBasicStringLiteral<_Char> format, _Arg0&& arg0, _Args&&... args);
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
TBasicTextWriter<_Char>& Format(TBasicTextWriter<_Char>& dst, TBasicStringLiteral<_Char> format, _Arg0&& arg0, _Args&&... args);
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
NODISCARD TBasicString<_Char> StringFormat(TBasicStringLiteral<_Char> format, _Arg0&& arg0, _Args&&... args) {
    TBasicString<_Char> result;
    Format(result, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
NODISCARD TBasicConstChar<_Char> InlineFormat(const TMemoryView<_Char>& dst, TBasicStringLiteral<_Char> format, _Arg0&& arg0, _Args&&... args) {
    const size_t len = Format(dst, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    dst[len] = _Char(0); // end-of-string: InlineFormat() always returns null-terminated strings
    return dst.data();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, size_t _Dim2, typename _Arg0, typename... _Args>
size_t Format(_Char(&dst)[_Dim], const _Char(&format)[_Dim2], _Arg0&& arg0, _Args&&... args) {
    return Format(MakeView(dst), MakeStringLiteral(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Arg0, typename... _Args>
size_t Format(const TMemoryView<_Char>& dst, const _Char(&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
    return Format(dst, MakeStringLiteral(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Arg0, typename... _Args>
void Format(TBasicString<_Char>& dst, const _Char(&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
    Format(dst, MakeStringLiteral(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Arg0, typename... _Args>
TBasicTextWriter<_Char>& Format(TBasicTextWriter<_Char>& dst, const _Char(&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
    Format(dst, MakeStringLiteral(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    return dst;
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Arg0, typename... _Args>
NODISCARD TBasicString<_Char> StringFormat(const _Char(&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
    return StringFormat(MakeStringLiteral(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Arg0, typename... _Args>
NODISCARD TBasicConstChar<_Char> InlineFormat(const TMemoryView<_Char>& dst, const _Char(&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
    return InlineFormat(dst, MakeStringLiteral(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EValidateFormat {
    Valid = 0,
    InvalidFormatString,
    ArgumentOutOfBounds,
    InvalidFormatManip,
    UnusedArguments,
    TooManyArguments,
};
//----------------------------------------------------------------------------
NODISCARD constexpr EValidateFormat ValidateFormatManip(char ch) noexcept;
NODISCARD constexpr EValidateFormat ValidateFormatManip(wchar_t ch) noexcept;
//----------------------------------------------------------------------------
NODISCARD constexpr EValidateFormat ValidateFormatString(const char* str, size_t len, size_t numArgs) noexcept;
NODISCARD constexpr EValidateFormat ValidateFormatString(const wchar_t* str, size_t len, size_t numArgs) noexcept;
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim>
NODISCARD constexpr EValidateFormat ValidateFormatString(const _Char(&fmt)[_Dim], size_t numArgs) noexcept {
    return ValidateFormatString(fmt, _Dim - 1/* - null char */, numArgs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "IO/Format-inl.h"
