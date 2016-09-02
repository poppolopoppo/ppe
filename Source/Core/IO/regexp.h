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
using BasicRegexp = std::basic_regex<_Char, _Traits>;
//----------------------------------------------------------------------------
typedef BasicRegexp<char>       Regexp;
typedef BasicRegexp<wchar_t>    WRegexp;
//----------------------------------------------------------------------------
Regexp MakeRegexp(const StringView& str);
WRegexp MakeRegexp(const WStringView& wstr);
//----------------------------------------------------------------------------
Regexp MakeRegexpI(const StringView& str);
WRegexp MakeRegexpI(const WStringView& wstr);
//----------------------------------------------------------------------------
bool Match(const Regexp& exp, const StringView& str);
bool Match(const WRegexp& exp, const WStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
