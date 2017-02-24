#pragma once

#include "Core/Core.h"

#include "Core/Container/HashMap.h"
#include "Core/IO/String.h"
#include "Core/IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    typename _Value,
    ECase _Sensitive,
    typename _Allocator = ALLOCATOR(Container, TPair<TBasicStringView<_Char> COMMA _Value>)
>
using TBasicStringViewHashMap = THashMap<
    TBasicStringView<_Char>,
    _Value,
    TStringViewHasher<_Char, _Sensitive>,
    TStringViewEqualTo<_Char, _Sensitive>,
    _Allocator
>;
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHMAP(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::TBasicStringViewHashMap<char, _VALUE, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::TPair<::Core::TBasicStringView<char> COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHMAP_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::TBasicStringViewHashMap<char, _VALUE, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::TPair<::Core::TBasicStringView<char> COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHMAP(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::TBasicStringViewHashMap<wchar_t, _VALUE, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::TPair<::Core::TBasicStringView<wchar_t> COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHMAP_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::Core::TBasicStringViewHashMap<wchar_t, _VALUE, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::TPair<::Core::TBasicStringView<wchar_t> COMMA _VALUE>)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHMAP_MEMOIZE(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHMAP(_DOMAIN, ::Core::TBasicStringViewHashMemoizer<char COMMA _CASE_SENSITIVE>, _VALUE)
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHMAP_MEMOIZE_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHMAP_THREAD_LOCAL(_DOMAIN, ::Core::TBasicStringViewHashMemoizer<char COMMA _CASE_SENSITIVE>, _VALUE)
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHMAP_MEMOIZE(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHMAP(_DOMAIN, ::Core::TBasicStringViewHashMemoizer<wchar_t COMMA _CASE_SENSITIVE>, _VALUE)
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHMAP_MEMOIZE_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHMAP_THREAD_LOCAL(_DOMAIN, ::Core::TBasicStringViewHashMemoizer<wchar_t COMMA _CASE_SENSITIVE>, _VALUE)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
