#pragma once

#include "Core/Core.h"

#include "Core/Container/HashMap.h"
#include "Core/IO/String.h"
#include "Core/IO/StringSlice.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    typename _Value,
    CaseSensitive _CaseSensitive,
    typename _Allocator = ALLOCATOR(Container, Pair<BasicStringSlice<_Char> COMMA _Value>)
>
using BasicStringSliceHashMap = HashMap<
    BasicStringSlice<_Char>,
    _Value,
    StringSliceHasher<_Char, _CaseSensitive>,
    StringSliceEqualTo<_Char, _CaseSensitive>,
    _Allocator
>;
//----------------------------------------------------------------------------
#define STRINGSLICE_HASHMAP(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::BasicStringSliceHashMap<char, _VALUE, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::Pair<::Core::BasicStringSlice<char> COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define STRINGSLICE_HASHMAP_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::BasicStringSliceHashMap<char, _VALUE, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::Pair<::Core::BasicStringSlice<char> COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define WSTRINGSLICE_HASHMAP(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::BasicStringSliceHashMap<wchar_t, _VALUE, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::Pair<::Core::BasicStringSlice<wchar_t> COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define WSTRINGSLICE_HASHMAP_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::BasicStringSliceHashMap<wchar_t, _VALUE, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::Pair<::Core::BasicStringSlice<wchar_t> COMMA _VALUE>)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
