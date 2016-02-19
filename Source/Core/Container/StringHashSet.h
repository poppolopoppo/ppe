#pragma once

#include "Core/Core.h"

#include "Core/Container/HashSet.h"
#include "Core/IO/String.h"
#include "Core/IO/StringSlice.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    CaseSensitive _CaseSensitive,
    typename _Allocator = ALLOCATOR(Container, BasicStringSlice<_Char>)
>
using BasicStringSliceHashSet = HashSet<
    BasicStringSlice<_Char>,
    StringSliceHasher<_Char, _CaseSensitive>,
    StringSliceEqualTo<_Char, _CaseSensitive>,
    _Allocator
>;
//----------------------------------------------------------------------------
#define STRINGSLICE_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::BasicStringSliceHashSet<char, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::BasicStringSlice<char>)>
//----------------------------------------------------------------------------
#define STRINGSLICE_HASHSET_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::BasicStringSliceHashSet<char, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::BasicStringSlice<char>)>
//----------------------------------------------------------------------------
#define WSTRINGSLICE_HASHSET(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::BasicStringSliceHashSet<wchar_t, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::BasicStringSlice<wchar_t>)>
//----------------------------------------------------------------------------
#define WSTRINGSLICE_HASHSET_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::BasicStringSliceHashSet<wchar_t, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::BasicStringSlice<wchar_t>)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
