#pragma once

#include "Core.h"

#include "Container/HashSet.h"
#include "IO/ConstChar.h"
#include "IO/String.h"
#include "IO/StringView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    ECase _Sensitive,
    typename _Allocator = ALLOCATOR(Container)
>
using TBasicStringViewHashSet = THashSet<
    TBasicStringView<_Char>,
    TStringViewHasher<_Char, _Sensitive>,
    TStringViewEqualTo<_Char, _Sensitive>,
    _Allocator
>;
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::PPE::TBasicStringViewHashSet<char, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN)>
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::PPE::TBasicStringViewHashSet<wchar_t, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN)>
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHSET_MEMOIZE(_DOMAIN, _CASE_SENSITIVE) \
    HASHSET(_DOMAIN, ::PPE::TBasicStringViewHashMemoizer<char COMMA _CASE_SENSITIVE>)
//----------------------------------------------------------------------------
#define WSTRINGVIEW_HASHSET_MEMOIZE(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHSET(_DOMAIN, ::PPE::TBasicStringViewHashMemoizer<wchar_t COMMA _CASE_SENSITIVE>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    ECase _Sensitive,
    typename _Allocator = ALLOCATOR(Container)
>
using TBasicConstCharHashSet = THashSet<
    TBasicConstChar<_Char>,
    TConstCharHasher<_Char, _Sensitive>,
    TConstCharEqualTo<_Char, _Sensitive>,
    _Allocator
>;
//----------------------------------------------------------------------------
#define CONSTCHAR_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::PPE::TBasicConstCharHashSet<char, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define CONSTWCHAR_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::PPE::TBasicConstCharHashSet<wchar_t, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define CONSTCHAR_HASHSET_MEMOIZE(_DOMAIN, _CASE_SENSITIVE) \
    HASHSET(_DOMAIN, ::PPE::TBasicConstCharHashMemoizer<char COMMA _CASE_SENSITIVE>)
//----------------------------------------------------------------------------
#define CONSTWCHAR_HASHSET_MEMOIZE(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHSET(_DOMAIN, ::PPE::TBasicConstCharHashMemoizer<wchar_t COMMA _CASE_SENSITIVE>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
