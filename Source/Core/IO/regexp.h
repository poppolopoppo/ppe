#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/IO/StringSlice.h"

#include <regex>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    typename _Traits = std::regex_traits<_Char>
>
using BasicRegexp = std::basic_regex<_Char, _Traits>;
//----------------------------------------------------------------------------
typedef BasicRegexp<char>       Regexp;
typedef BasicRegexp<wchar_t>    WRegexp;
//----------------------------------------------------------------------------
Regexp MakeRegexp(const StringSlice& str);
WRegexp MakeRegexp(const WStringSlice& wstr);
//----------------------------------------------------------------------------
Regexp MakeRegexpI(const StringSlice& str);
WRegexp MakeRegexpI(const WStringSlice& wstr);
//----------------------------------------------------------------------------
bool Match(const Regexp& exp, const StringSlice& str);
bool Match(const WRegexp& exp, const WStringSlice& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
