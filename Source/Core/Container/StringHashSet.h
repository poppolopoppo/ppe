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
    ECase _Sensitive,
    typename _Allocator = ALLOCATOR(Container, TBasicStringView<_Char>)
>
using BasicStringViewHashSet = THashSet<
    TBasicStringView<_Char>,
    TStringViewHasher<_Char, _Sensitive>,
    TStringViewEqualTo<_Char, _Sensitive>,
    _Allocator
>;
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::BasicStringViewHashSet<char, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::TBasicStringView<char>)>
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHSET_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::BasicStringViewHashSet<char, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::TBasicStringView<char>)>
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHSET(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::BasicStringViewHashSet<wchar_t, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::TBasicStringView<wchar_t>)>
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHSET_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::BasicStringViewHashSet<wchar_t, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::TBasicStringView<wchar_t>)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
