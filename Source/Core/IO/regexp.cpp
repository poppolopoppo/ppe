#include "stdafx.h"

#include "regexp.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Regexp MakeRegexp(const StringView& str) {
    return Regexp(str.begin(), str.end(), Regexp::ECMAScript|Regexp::optimize);
}
//----------------------------------------------------------------------------
WRegexp MakeRegexp(const WStringView& wstr) {
    return WRegexp(wstr.begin(), wstr.end(), WRegexp::ECMAScript|WRegexp::optimize);
}
//----------------------------------------------------------------------------
Regexp MakeRegexpI(const StringView& str) {
    return Regexp(str.begin(), str.end(), Regexp::ECMAScript|Regexp::optimize|Regexp::icase);
}
//----------------------------------------------------------------------------
WRegexp MakeRegexpI(const WStringView& wstr) {
    return WRegexp(wstr.begin(), wstr.end(), WRegexp::ECMAScript|WRegexp::optimize|WRegexp::icase);
}
//----------------------------------------------------------------------------
bool Match(const Regexp& exp, const StringView& str) {
    return std::regex_match(str.begin(), str.end(), exp, std::regex_constants::match_default);
}
//----------------------------------------------------------------------------
bool Match(const WRegexp& exp, const WStringView& wstr) {
    return std::regex_match(wstr.begin(), wstr.end(), exp, std::regex_constants::match_default);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
