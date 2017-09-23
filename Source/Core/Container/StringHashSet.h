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
using TBasicStringViewHashSet = THashSet<
    TBasicStringView<_Char>,
    TStringViewHasher<_Char, _Sensitive>,
    TStringViewEqualTo<_Char, _Sensitive>,
    _Allocator
>;
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::TBasicStringViewHashSet<char, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::TBasicStringView<char>)>

#define STRINGVIEW_HASHSET_THREAD_LOCAL(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::TBasicStringViewHashSet<char, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::TBasicStringView<char>)>

#define WSTRINGVIEW_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::TBasicStringViewHashSet<wchar_t, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::TBasicStringView<wchar_t>)>

#define WSTRINGVIEW_HASHSET_THREAD_LOCAL(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::TBasicStringViewHashSet<wchar_t, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::TBasicStringView<wchar_t>)>
//----------------------------------------------------------------------------
#define STRINGVIEW_HASHSET_MEMOIZE(_DOMAIN, _CASE_SENSITIVE) \
    HASHSET(_DOMAIN, ::Core::TBasicStringViewHashMemoizer<char COMMA _CASE_SENSITIVE>)

#define STRINGVIEW_HASHSET_MEMOIZE_THREAD_LOCAL(_DOMAIN, _CASE_SENSITIVE) \
    HASHSET_THREAD_LOCAL(_DOMAIN, ::Core::TBasicStringViewHashMemoizer<char COMMA _CASE_SENSITIVE>)

#define WSTRINGVIEW_HASHSET_MEMOIZE(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHSET(_DOMAIN, ::Core::TBasicStringViewHashMemoizer<wchar_t COMMA _CASE_SENSITIVE>)

#define WSTRINGVIEW_HASHSET_MEMOIZE_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHSET_THREAD_LOCAL(_DOMAIN, ::Core::TBasicStringViewHashMemoizer<wchar_t COMMA _CASE_SENSITIVE>)
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

#define CONSTCHAR_HASHSET_THREAD_LOCAL(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::TBasicConstCharHashSet<char, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::TBasicConstChar<char>)>

#define CONSTWCHAR_HASHSET(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::TBasicConstCharHashSet<wchar_t, _CASE_SENSITIVE, ALLOCATOR(_DOMAIN, ::Core::TBasicConstChar<wchar_t>)>

#define CONSTWCHAR_HASHSET_THREAD_LOCAL(_DOMAIN, _CASE_SENSITIVE) \
    ::Core::TBasicConstCharHashSet<wchar_t, _CASE_SENSITIVE, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::TBasicConstChar<wchar_t>)>
//----------------------------------------------------------------------------
#define CONSTCHAR_HASHSET_MEMOIZE(_DOMAIN, _CASE_SENSITIVE) \
    HASHSET(_DOMAIN, ::Core::TBasicConstCharHashMemoizer<char COMMA _CASE_SENSITIVE>)

#define CONSTCHAR_HASHSET_MEMOIZE_THREAD_LOCAL(_DOMAIN, _CASE_SENSITIVE) \
    HASHSET_THREAD_LOCAL(_DOMAIN, ::Core::TBasicConstCharHashMemoizer<char COMMA _CASE_SENSITIVE>)

#define CONSTWCHAR_HASHSET_MEMOIZE(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHSET(_DOMAIN, ::Core::TBasicConstCharHashMemoizer<wchar_t COMMA _CASE_SENSITIVE>)

#define CONSTWCHAR_HASHSET_MEMOIZE_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE) \
    HASHSET_THREAD_LOCAL(_DOMAIN, ::Core::TBasicConstCharHashMemoizer<wchar_t COMMA _CASE_SENSITIVE>)

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
