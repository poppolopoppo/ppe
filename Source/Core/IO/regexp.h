#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/IO/StringView.h"

#include <regex>

namespace Core {
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
bool FMatch(const Regexp& exp, const FStringView& str);
bool FMatch(const WRegexp& exp, const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
