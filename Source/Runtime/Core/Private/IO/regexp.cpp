// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#define EXPORT_PPE_RUNTIME_CORE_REGEX

#include "IO/regexp.h"

#include "Diagnostic/Logger.h"
#include "IO/String.h"
#include "IO/StringView.h"

namespace PPE {
LOG_CATEGORY(, Regex)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicRegexp<_Char>::TBasicRegexp(const stringview_type& expr, ECase icase) {
    Compile(expr, icase);
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
bool TBasicRegexp<_Char>::Capture(FMatches* outMatch, const stringview_type& str) const {
    Assert(outMatch);
    return std::regex_search(str.begin(), str.end(), *outMatch, _re);
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicRegexp<_Char>::Swap(TBasicRegexp& other) NOEXCEPT {
    std::swap(_re, other._re);
}
//----------------------------------------------------------------------------
template <typename _Char>
auto TBasicRegexp<_Char>::BuiltinFormat(EBuiltinRegexp fmt, ECase sensitive/* = ECase::Sensitive */) -> TBasicRegexp {
    stringview_type pattern;

    switch (fmt) {
    case EBuiltinRegexp::Base64:
        pattern = MakeStringView(STRING_LITERAL(_Char, R"regexp(^([a-zA-Z0-9]+)$)regexp")); break;
    case EBuiltinRegexp::Date:
        pattern = MakeStringView(STRING_LITERAL(_Char, R"regexp(^(?:(?:31(\/|-|\.)(?:0?[13578]|1[02]|(?:Jan|Mar|May|Jul|Aug|Oct|Dec)))\1|(?:(?:29|30)(\/|-|\.)(?:0?[1,3-9]|1[0-2]|(?:Jan|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec))\2))(?:(?:1[6-9]|[2-9]\d)?\d{2})$|^(?:29(\/|-|\.)(?:0?2|(?:Feb))\3(?:(?:(?:1[6-9]|[2-9]\d)?(?:0[48]|[2468][048]|[13579][26])|(?:(?:16|[2468][048]|[3579][26])00))))$|^(?:0?[1-9]|1\d|2[0-8])(\/|-|\.)(?:(?:0?[1-9]|(?:Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep))|(?:1[0-2]|(?:Oct|Nov|Dec)))\4(?:(?:1[6-9]|[2-9]\d)?\d{2})$)regexp")); break;
    case EBuiltinRegexp::DateTime:
        pattern = MakeStringView(STRING_LITERAL(_Char, R"regexp(^\d\d\d\d-(0?[1-9]|1[0-2])-(0?[1-9]|[12][0-9]|3[01]) (00|[0-9]|1[0-9]|2[0-3]):([0-9]|[0-5][0-9]):([0-9]|[0-5][0-9])$)regexp")); break;
    case EBuiltinRegexp::Email:
        pattern = MakeStringView(STRING_LITERAL(_Char, R"regexp(^((?:[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*|"(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21\x23-\x5b\x5d-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])*")@(?:(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?|\[(?:(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9]))\.){3}(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9])|[a-z0-9-]*[a-z0-9]:(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21-\x5a\x53-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])+)\]))$)regexp")); break;
    case EBuiltinRegexp::Guid:
        pattern = MakeStringView(STRING_LITERAL(_Char, R"regexp(^((?im)^[{(]?[0-9A-F]{8}[-]?(?:[0-9A-F]{4}[-]?){3}[0-9A-F]{12}[)}]?$)regexp")); break;
    case EBuiltinRegexp::Uuid:
        pattern = MakeStringView(STRING_LITERAL(_Char, R"regexp(^([a-f0-9]{8}-[a-f0-9]{4}-4[a-f0-9]{3}-[89aAbB][a-f0-9]{3}-[a-f0-9]{12})$)regexp")); break;
    case EBuiltinRegexp::Ipv4:
        pattern = MakeStringView(STRING_LITERAL(_Char, R"regexp(^(25[0–5]|2[0–4][0–9]|[01]?[0–9][0–9]?).(25[0–5]|2[0–4][0–9]|[01]?[0–9][0–9]?).(25[0–5]|2[0–4][0–9]|[01]?[0–9][0–9]?).(25[0–5]|2[0–4][0–9]|[01]?[0–9][0–9]?)$)regexp")); break;
    case EBuiltinRegexp::Ipv6:
        pattern = MakeStringView(STRING_LITERAL(_Char, R"regexp(^((([0–9A-Fa-f]{1,4}:){7}[0–9A-Fa-f]{1,4})|(([0–9A-Fa-f]{1,4}:){6}:[0–9A-Fa-f]{1,4})|(([0–9A-Fa-f]{1,4}:){5}:([0–9A-Fa-f]{1,4}:)?[0–9A-Fa-f]{1,4})|(([0–9A-Fa-f]{1,4}:){4}:([0–9A-Fa-f]{1,4}:){0,2}[0–9A-Fa-f]{1,4})|(([0–9A-Fa-f]{1,4}:){3}:([0–9A-Fa-f]{1,4}:){0,3}[0–9A-Fa-f]{1,4})|(([0–9A-Fa-f]{1,4}:){2}:([0–9A-Fa-f]{1,4}:){0,4}[0–9A-Fa-f]{1,4})|(([0–9A-Fa-f]{1,4}:){6}((b((25[0–5])|(1d{2})|(2[0–4]d)|(d{1,2}))b).){3}(b((25[0–5])|(1d{2})|(2[0–4]d)|(d{1,2}))b))|(([0–9A-Fa-f]{1,4}:){0,5}:((b((25[0–5])|(1d{2})|(2[0–4]d)|(d{1,2}))b).){3}(b((25[0–5])|(1d{2})|(2[0–4]d)|(d{1,2}))b))|(::([0–9A-Fa-f]{1,4}:){0,5}((b((25[0–5])|(1d{2})|(2[0–4]d)|(d{1,2}))b).){3}(b((25[0–5])|(1d{2})|(2[0–4]d)|(d{1,2}))b))|([0–9A-Fa-f]{1,4}::([0–9A-Fa-f]{1,4}:){0,5}[0–9A-Fa-f]{1,4})|(::([0–9A-Fa-f]{1,4}:){0,6}[0–9A-Fa-f]{1,4})|(([0–9A-Fa-f]{1,4}:){1,7}:))$)regexp")); break;
    case EBuiltinRegexp::Url:
        pattern = MakeStringView(STRING_LITERAL(_Char, R"regexp(^([a-z0-9+.-]+):(?://(?:((?:[a-z0-9-._~!$&'()*+,;=:]|%[0-9A-F]{2})*)@)?((?:[a-z0-9-._~!$&'()*+,;=]|%[0-9A-F]{2})*)(?::(\d*))?(/(?:[a-z0-9-._~!$&'()*+,;=:@/]|%[0-9A-F]{2})*)?|(/?(?:[a-z0-9-._~!$&'()*+,;=:@]|%[0-9A-F]{2})+(?:[a-z0-9-._~!$&'()*+,;=:@/]|%[0-9A-F]{2})*)?)(?:\?((?:[a-z0-9-._~!$&'()*+,;=:/?@]|%[0-9A-F]{2})*))?(?:#((?:[a-z0-9-._~!$&'()*+,;=:/?@]|%[0-9A-F]{2})*))?$)regexp")); break;
    case EBuiltinRegexp::FloatingPoint:
        pattern = MakeStringView(STRING_LITERAL(_Char, R"regexp(^[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)$)regexp")); break;
    case EBuiltinRegexp::ScientificFloat:
        pattern = MakeStringView(STRING_LITERAL(_Char, R"regexp(^([+-]?(\d+([.]\d*)?([eE][+-]?\d+)?|[.]\d+([eE][+-]?\d+)?))$)regexp")); break;
    }

    return TBasicRegexp{ pattern, sensitive };
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
    //PPE_CATCH_BLOCK(
        {
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
#ifdef PLATFORM_WINDOWS
            PPE_REGEXERROR_WSTR(error_parse);
            PPE_REGEXERROR_WSTR(error_syntax);
#endif
        default:
            AssertNotImplemented();
        }
        Unused(errorStr); // when logger is disabled
        LOG(Regex, Error, L"regex_error caught: {0} (code: {1})\n\texpr: {2}", e.what(), errorStr, expr);
        return false;
    }
    //)
#undef PPE_REGEXERROR_WSTR
}
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicRegexp<char>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicRegexp<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
template <typename _Char>
static TBasicTextWriter<_Char>& EBuiltinRegexp_Print_(TBasicTextWriter<_Char>& oss, EBuiltinRegexp fmt) {
    switch (fmt) {
    case EBuiltinRegexp::Base64: return oss << STRING_LITERAL(_Char, "Base64");
    case EBuiltinRegexp::Date: return oss << STRING_LITERAL(_Char, "Date");
    case EBuiltinRegexp::DateTime: return oss << STRING_LITERAL(_Char, "DateTime");
    case EBuiltinRegexp::Email: return oss << STRING_LITERAL(_Char, "Email");
    case EBuiltinRegexp::Guid: return oss << STRING_LITERAL(_Char, "Guid");
    case EBuiltinRegexp::Uuid: return oss << STRING_LITERAL(_Char, "Uuid");
    case EBuiltinRegexp::Ipv4: return oss << STRING_LITERAL(_Char, "Ipv4");
    case EBuiltinRegexp::Ipv6: return oss << STRING_LITERAL(_Char, "Ipv6");
    case EBuiltinRegexp::Url: return oss << STRING_LITERAL(_Char, "Url");
    case EBuiltinRegexp::FloatingPoint: return oss << STRING_LITERAL(_Char, "FloatingPoint");
    case EBuiltinRegexp::ScientificFloat: return oss << STRING_LITERAL(_Char, "ScientificFloat");
    }
    AssertNotReached();
}
} //!namespace
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EBuiltinRegexp fmt) {
    return EBuiltinRegexp_Print_(oss, fmt);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, EBuiltinRegexp fmt) {
    return EBuiltinRegexp_Print_(oss, fmt);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
