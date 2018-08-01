#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "IO/StringView.h"

#include <regex>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    typename _Traits = std::regex_traits<_Char>
>
using TBasicRegexp = std::basic_regex<_Char, _Traits>;
//----------------------------------------------------------------------------
typedef TBasicRegexp<char>       Regexp;
typedef TBasicRegexp<wchar_t>    WRegexp;
//----------------------------------------------------------------------------
Regexp MakeRegexp(const FStringView& str);
WRegexp MakeRegexp(const FWStringView& wstr);
//----------------------------------------------------------------------------
Regexp MakeRegexpI(const FStringView& str);
WRegexp MakeRegexpI(const FWStringView& wstr);
//----------------------------------------------------------------------------
bool Match(const Regexp& exp, const FStringView& str);
bool Match(const WRegexp& exp, const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
