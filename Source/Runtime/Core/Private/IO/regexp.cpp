#include "stdafx.h"

#include "IO/Regexp.h"

#include "Container/Pair.h"
#include "Container/PerfectHashing.h"
#include "Diagnostic/Logger.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "Meta/Iterator.h"

namespace PPE {
LOG_CATEGORY(, Regex)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
CONSTEXPR auto MakeRegexClassnameLookup(char) {
    CONSTEXPR const TPair<TIterable<const char*>, std::ctype_base::mask> ctypes[] = {
        { MakeIterable("alnum"), std::ctype_base::alnum },
        { MakeIterable("alpha"), std::ctype_base::alpha },
        { MakeIterable("blank"), std::ctype_base::blank },
        { MakeIterable("cntrl"), std::ctype_base::cntrl },
        { MakeIterable("d"), std::ctype_base::digit },
        { MakeIterable("digit"), std::ctype_base::digit },
        { MakeIterable("graph"), std::ctype_base::graph },
        { MakeIterable("lower"), std::ctype_base::lower },
        { MakeIterable("print"), std::ctype_base::print },
        { MakeIterable("punct"), std::ctype_base::punct },
        { MakeIterable("s"), std::ctype_base::space },
        { MakeIterable("space"), std::ctype_base::space },
        { MakeIterable("upper"), std::ctype_base::upper },
        { MakeIterable("w"), std::ctype_base::mask(-1) },
        { MakeIterable("xdigit"), std::ctype_base::xdigit }
    };
    return MinimalPerfectHashMap<false>(ctypes);
}
CONSTEXPR auto MakeRegexClassnameLookup(wchar_t) {
    CONSTEXPR const TPair<TIterable<const wchar_t*>, std::ctype_base::mask> wctypes[] = {
        { MakeIterable(L"alnum"), std::ctype_base::alnum },
        { MakeIterable(L"alpha"), std::ctype_base::alpha },
        { MakeIterable(L"blank"), std::ctype_base::blank },
        { MakeIterable(L"cntrl"), std::ctype_base::cntrl },
        { MakeIterable(L"d"), std::ctype_base::digit },
        { MakeIterable(L"digit"), std::ctype_base::digit },
        { MakeIterable(L"graph"), std::ctype_base::graph },
        { MakeIterable(L"lower"), std::ctype_base::lower },
        { MakeIterable(L"print"), std::ctype_base::print },
        { MakeIterable(L"punct"), std::ctype_base::punct },
        { MakeIterable(L"s"), std::ctype_base::space },
        { MakeIterable(L"space"), std::ctype_base::space },
        { MakeIterable(L"upper"), std::ctype_base::upper },
        { MakeIterable(L"w"), std::ctype_base::mask(-1) },
        { MakeIterable(L"xdigit"), std::ctype_base::xdigit }
    };
    return MinimalPerfectHashMap<false>(wctypes);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
size_t TRegexTraits<_Char>::length(const char_type* s) {
    return PPE::Length(s);
}
//----------------------------------------------------------------------------
template <typename _Char>
auto TRegexTraits<_Char>::translate(char_type ch) const -> char_type {
    return ch;
}
//----------------------------------------------------------------------------
template <typename _Char>
auto TRegexTraits<_Char>::translate_nocase(char_type ch) const -> char_type {
    return PPE::ToLower(ch);
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _FwdIt>
auto TRegexTraits<_Char>::transform(_FwdIt first, _FwdIt last) const -> string_type {
    return string_type{ first, last };
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _FwdIt>
auto TRegexTraits<_Char>::transform_primary(_FwdIt first, _FwdIt last) const -> string_type {
    auto it = MakeOutputIterable(first, last, [](char_type ch) NOEXCEPT {
        return ToLower(ch);
    });
    return string_type{ it.begin(), it.end() };
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _FwdIt>
auto TRegexTraits<_Char>::lookup_collatename(_FwdIt first, _FwdIt last) const -> string_type {
    return string_type{ first, last };
}
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4307)
template <typename _Char>
template <typename _FwdIt>
auto TRegexTraits<_Char>::lookup_classname(_FwdIt first, _FwdIt last,
    bool insensitive/* = false */) const -> char_class_type {
    using token_t = TIterable<const _Char*>;

    static CONSTEXPR const auto MPH = MakeRegexClassnameLookup(_Char{});

    char_class_type result{ 0 };

    const size_t h = hash_fwdit_constexpr(first, last);
    const char_class_type* const classname = MPH.Lookup(h, [&](const token_t& cname) NOEXCEPT {
        return cname.Equals(MakeIterable(first, last));
    });

    if (classname)
        result = *classname;

    // from std::regex_traits<> :
    if (insensitive && (result & (std::ctype_base::lower | std::ctype_base::upper)))
        result |= std::ctype_base::lower | std::ctype_base::upper;

    return result;
}
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
template <>
bool TRegexTraits<char>::isctype(char ch, char_class_type classname) const {
    if (classname == char_class_type(-1))
        // from std::regex_traits<> :
        return ((ch == '_') | std::isalnum(ch));
    else
        switch (classname) {
        case std::ctype_base::alnum:
            return std::isalnum(ch);
        case std::ctype_base::alpha:
            return std::isalpha(ch);
        case std::ctype_base::cntrl:
            return std::iscntrl(ch);
        case std::ctype_base::digit:
            return std::isdigit(ch);
        case std::ctype_base::graph:
            return std::isgraph(ch);
        case std::ctype_base::lower:
            return std::islower(ch);
        case std::ctype_base::print:
            return std::isprint(ch);
        case std::ctype_base::punct:
            return std::ispunct(ch);
        //case std::ctype_base::blank:
        case std::ctype_base::space:
            return std::isspace(ch);
        case std::ctype_base::upper:
            return std::isupper(ch);
        case std::ctype_base::xdigit:
            return std::isxdigit(ch);

        default:
            AssertNotImplemented();
        }
}
template <>
bool TRegexTraits<wchar_t>::isctype(wchar_t ch, char_class_type classname) const {
    if (classname == char_class_type(-1))
        // from std::regex_traits<> :
        return ((ch == L'_')/* assumes L'_' == '_' */ | std::iswalnum(ch));
    else
        switch (classname) {
        case std::ctype_base::alnum:
            return std::iswalnum(ch);
        case std::ctype_base::alpha:
            return std::iswalpha(ch);
        case std::ctype_base::cntrl:
            return std::iswcntrl(ch);
        case std::ctype_base::digit:
            return std::iswdigit(ch);
        case std::ctype_base::graph:
            return std::iswgraph(ch);
        case std::ctype_base::lower:
            return std::iswlower(ch);
        case std::ctype_base::print:
            return std::iswprint(ch);
        case std::ctype_base::punct:
            return std::iswpunct(ch);
        //case std::ctype_base::blank:
        case std::ctype_base::space:
            return std::iswspace(ch);
        case std::ctype_base::upper:
            return std::iswupper(ch);
        case std::ctype_base::xdigit:
            return std::iswxdigit(ch);

        default:
            AssertNotImplemented();
        }
}
//----------------------------------------------------------------------------
template <>
int TRegexTraits<char>::value(char ch, int base) const {
    // from std::regex_traits<> :
    if ((base != 8 && '0' <= ch && ch <= '9') || (base == 8 && '0' <= ch && ch <= '7'))
        return ch - '0';

    if (base != 16)
        return -1;

    if ('a' <= ch && ch <= 'f')
        return ch - 'a' + 10;

    if ('A' <= ch && ch <= 'F')
        return ch - 'A' + 10;

    return -1;
}
template <>
int TRegexTraits<wchar_t>::value(wchar_t ch, int base) const {
    // from std::regex_traits<> :
    if ((base != 8 && L'0' <= ch && ch <= L'9') || (base == 8 && L'0' <= ch && ch <= L'7'))
        return ch - L'0';

    if (base != 16)
        return -1;

    if (L'a' <= ch && ch <= L'f')
        return ch - L'a' + 10;

    if (L'A' <= ch && ch <= L'F')
        return ch - L'A' + 10;

    return -1;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicRegexp<_Char>::TBasicRegexp() NOEXCEPT
{}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicRegexp<_Char>::~TBasicRegexp() = default;
//----------------------------------------------------------------------------
template <typename _Char>
TBasicRegexp<_Char>::TBasicRegexp(regex_type&& re) NOEXCEPT
:   _re(std::move(re))
{}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicRegexp<_Char>::TBasicRegexp(const stringview_type& expr, ECase icase) {
    Compile(expr, icase);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicRegexp<_Char>::TBasicRegexp(TBasicRegexp&& rvalue) NOEXCEPT
:   TBasicRegexp(std::move(rvalue._re))
{}
//----------------------------------------------------------------------------
template <typename _Char>
auto TBasicRegexp<_Char>::operator =(TBasicRegexp&& rvalue) NOEXCEPT -> TBasicRegexp& {
    _re = std::move(rvalue._re);
    return (*this);
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicRegexp<_Char>::Compile(const stringview_type& expr, ECase icase) {
    typename regex_type::flag_type flags = regex_type::ECMAScript | regex_type::optimize;
    if (icase == ECase::Insensitive)
        flags |= regex_type::icase;

    _re = regex_type(expr.begin(), expr.end(), flags);
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TBasicRegexp<_Char>::Match(const stringview_type& str) const {
    return std::regex_match(str.begin(), str.end(), _re, std::regex_constants::match_default);
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicRegexp<_Char>::Swap(TBasicRegexp& other) NOEXCEPT {
    std::swap(_re, other._re);
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TBasicRegexp<_Char>::ValidateSyntax(const stringview_type& expr) NOEXCEPT {
    PPE_TRY {
        TBasicRegexp re{ expr, ECase::Sensitive/* just validating format, don't need the case here */ };
        return true;
    }
#define PPE_REGEXERROR_WSTR(_NAME) \
    case std::regex_constants::_NAME: \
        errorStr = WSTRINGIZE(_NAME); \
        break
    PPE_CATCH(const std::regex_error& e)
    PPE_CATCH_BLOCK({
        FWString errorStr;
        switch (e.code()) {
            PPE_REGEXERROR_WSTR(error_collate);
            PPE_REGEXERROR_WSTR(error_ctype);
            PPE_REGEXERROR_WSTR(error_escape);
            PPE_REGEXERROR_WSTR(error_backref);
            PPE_REGEXERROR_WSTR(error_brack);
            PPE_REGEXERROR_WSTR(error_paren);
            PPE_REGEXERROR_WSTR(error_brace);
            PPE_REGEXERROR_WSTR(error_badbrace);
            PPE_REGEXERROR_WSTR(error_range);
            PPE_REGEXERROR_WSTR(error_space);
            PPE_REGEXERROR_WSTR(error_badrepeat);
            PPE_REGEXERROR_WSTR(error_complexity);
            PPE_REGEXERROR_WSTR(error_stack);
            PPE_REGEXERROR_WSTR(error_parse);
            PPE_REGEXERROR_WSTR(error_syntax);
        default:
            AssertNotImplemented();
        }
        UNUSED(errorStr); // when logger is disabled
        LOG(Regex, Error, L"regex_error caught: {0} (code: {1})\n\texpr: {2}", e.what(), errorStr, expr);
        return false;
    })
#undef PPE_REGEXERROR_WSTR
}
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicRegexp<char>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicRegexp<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
