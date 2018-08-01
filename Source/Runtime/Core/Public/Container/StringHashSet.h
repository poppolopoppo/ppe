#pragma once

#include "Core.h"

#include "Container/HashSet.h"
#include "IO/String.h"
#include "IO/StringView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    ECase _Sensitive,
    typename _Allocator = ALLOCATOR(Container, TBasicStringView<_Char>)
>
using TBasicStringViewHashSet = THashSet<
    TBasicStringView<_Char>,
    TStringViewHasher<_Char, _Sensitive>,
    TStringViewEqualTo<_Char, _Sensitive>,
    _Allocator
>;
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::TBasicStringViewHashSet<char, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::TBasicStringView<char>)>
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::TBasicStringViewHashSet<wchar_t, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::TBasicStringView<wchar_t>)>
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHSET_MEMOIZE(_DOMAIN, _CASE_SENSITIVE) \
    HASHSET(_DOMAIN, ::Core::TBasicStringViewHashMemoizer<char COMMA _CASE_SENSITIVE>)
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHSET_MEMOIZE(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHSET(_DOMAIN, ::Core::TBasicStringViewHashMemoizer<wchar_t COMMA _CASE_SENSITIVE>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    ECase _Sensitive,
    typename _Allocator = ALLOCATOR(Container, TBasicConstChar<_Char>)
>
using TBasicConstCharHashSet = THashSet<
    TBasicConstChar<_Char>,
    TConstCharHasher<_Char, _Sensitive>,
    TConstCharEqualTo<_Char, _Sensitive>,
    _Allocator
>;
//----------------------------------------------------------------------------
#define CONSTCHAR_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::TBasicConstCharHashSet<char, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::TBasicConstChar<char>)>
//----------------------------------------------------------------------------
#define CONSTWCHAR_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::TBasicConstCharHashSet<wchar_t, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::TBasicConstChar<wchar_t>)>
//----------------------------------------------------------------------------
#define CONSTCHAR_HASHSET_MEMOIZE(_DOMAIN, _CASE_SENSITIVE) \
    HASHSET(_DOMAIN, ::Core::TBasicConstCharHashMemoizer<char COMMA _CASE_SENSITIVE>)
//----------------------------------------------------------------------------
#define CONSTWCHAR_HASHSET_MEMOIZE(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHSET(_DOMAIN, ::Core::TBasicConstCharHashMemoizer<wchar_t COMMA _CASE_SENSITIVE>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
