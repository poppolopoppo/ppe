#pragma once

#include "Core.h"

#include "Container/HashHelpers.h"
#include "Container/HashMap.h"
#include "IO/String_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    typename _Value,
    ECase _Sensitive,
    typename _Allocator = ALLOCATOR(Container)
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
    ::PPE::TBasicStringViewHashMap<char, _VALUE, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN)>
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHMAP(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::PPE::TBasicStringViewHashMap<wchar_t, _VALUE, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN)>
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
    typename _Allocator = ALLOCATOR(Container)
>
using TBasicStringHashMap = THashMap<
    TBasicString<_Char>,
    _Value,
    TStringHasher<_Char, _Sensitive>,
    TStringEqualTo<_Char, _Sensitive>,
    _Allocator
>;
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
using TBasicStringHashMemoizer = THashMemoizer<
    TBasicString<_Char>,
    TStringHasher<_Char, _Sensitive>,
    TStringEqualTo<_Char, _Sensitive>
>;
//----------------------------------------------------------------------------
#define STRING_HASHMAP(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::PPE::TBasicStringHashMap<char, _VALUE, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define WSTRING_HASHMAP(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    ::PPE::TBasicStringHashMap<wchar_t, _VALUE, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN) >
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
