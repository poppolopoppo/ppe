#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/IO/StringSlice.h"

#include <regex>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::regex_traits<_Char> >
using BasicRegexp = std::regex<_Char, _Traits>;
//----------------------------------------------------------------------------
using Regexp = BasicRegexp<char>;
using WRegexp = BasicRegexp<wchar_t>;
//----------------------------------------------------------------------------
bool ReMatch(const Regexp& exp, const StringSlice& str);
bool ReMatch(const WRegexp& exp, const WStringSlice& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
