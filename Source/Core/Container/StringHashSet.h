#pragma once

#include "Core/Core.h"

#include "Core/Container/HashSet.h"
#include "Core/IO/String.h"
#include "Core/IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    Case _Sensitive,
    typename _Allocator = ALLOCATOR(Container, BasicStringView<_Char>)
>
using BasicStringViewHashSet = HashSet<
    BasicStringView<_Char>,
    StringViewHasher<_Char, _Sensitive>,
    StringViewEqualTo<_Char, _Sensitive>,
    _Allocator
>;
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::BasicStringViewHashSet<char, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::BasicStringView<char>)>
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHSET_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::BasicStringViewHashSet<char, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::BasicStringView<char>)>
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHSET(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::BasicStringViewHashSet<wchar_t, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::BasicStringView<wchar_t>)>
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHSET_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::BasicStringViewHashSet<wchar_t, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::BasicStringView<wchar_t>)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
