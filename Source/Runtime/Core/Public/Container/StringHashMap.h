#pragma once

#include "Core.h"

#include "Container/HashMap.h"
#include "IO/String.h"
#include "IO/StringView.h"

namespace PPE {
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
    ::PPE::TBasicStringViewHashMap<char, _VALUE, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::PPE::TPair<::PPE::TBasicStringView<char> COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHMAP(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::PPE::TBasicStringViewHashMap<wchar_t, _VALUE, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::PPE::TPair<::PPE::TBasicStringView<wchar_t> COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHMAP_MEMOIZE(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHMAP(_DOMAIN, ::PPE::TBasicStringViewHashMemoizer<char COMMA _CASE_SENSITIVE>, _VALUE)
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHMAP_MEMOIZE(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHMAP(_DOMAIN, ::PPE::TBasicStringViewHashMemoizer<wchar_t COMMA _CASE_SENSITIVE>, _VALUE)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    typename _Value,
    ECase _Sensitive,
    typename _Allocator = ALLOCATOR(Container, TPair<TBasicString<_Char> COMMA _Value>)
>
using TBasicStringHashMap = THashMap<
    TBasicString<_Char>,
    _Value,
    TStringHasher<_Char, _Sensitive>,
    TStringEqualTo<_Char, _Sensitive>,
    _Allocator
>;
//----------------------------------------------------------------------------
#define STRING_HASHMAP(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::PPE::TBasicStringHashMap<char, _VALUE, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::PPE::TPair<::PPE::TBasicString<char> COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define WSTRING_HASHMAP(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::PPE::TBasicStringHashMap<wchar_t, _VALUE, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::PPE::TPair<::PPE::TBasicString<wchar_t> COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define STRING_HASHMAP_MEMOIZE(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHMAP(_DOMAIN, ::PPE::TBasicStringHashMemoizer<char COMMA _CASE_SENSITIVE>, _VALUE)
//----------------------------------------------------------------------------
#define WSTRING_HASHMAP_MEMOIZE(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHMAP(_DOMAIN, ::PPE::TBasicStringHashMemoizer<wchar_t COMMA _CASE_SENSITIVE>, _VALUE)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
