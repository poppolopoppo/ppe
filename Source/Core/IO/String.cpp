#include "stdafx.h"

#include "String.h"

#include "Memory/UniqueView.h"

#include <string.h>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// thank you M$ STL ...
template <CaseSensitive caseSensitive, typename _Char>
static size_t hash_value_Cstr_(const _Char *cstr, size_t length, const std::locale& locale) {
    // FNV-1a hash function for bytes in [_First, _First+_Count)
#if defined(_M_X64) || defined(_LP64) || defined(__x86_64) || defined(_WIN64)
    static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
    const size_t _FNV_offset_basis = 14695981039346656037ULL;
    const size_t _FNV_prime = 1099511628211ULL;

#else /* defined(_M_X64), etc. */
    static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
    const size_t _FNV_offset_basis = 2166136261U;
    const size_t _FNV_prime = 16777619U;
#endif /* defined(_M_X64), etc. */

    size_t _Val = _FNV_offset_basis;
    for (size_t _Next = 0; _Next < length; ++_Next)
    {    // fold in another byte
        _Val ^= (size_t)((CaseSensitive::False == caseSensitive)
            ? std::tolower(cstr[_Next], locale)
            : cstr[_Next]
            );
        _Val *= _FNV_prime;
    }

#if defined(_M_X64) || defined(_LP64) || defined(__x86_64) || defined(_WIN64)
    static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
    _Val ^= _Val >> 32;

#else /* defined(_M_X64), etc. */
    static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
#endif /* defined(_M_X64), etc. */

    return (_Val);
}
//----------------------------------------------------------------------------
template <typename _Char>
struct WildChars_ {};

template <>
struct WildChars_< char > {
    enum : char {
        Dot         = '.',
        Question    = '?',
        Star        = '*',
    };
};

template <>
struct WildChars_< wchar_t > {
    enum : wchar_t {
        Dot         = L'.',
        Question    = L'?',
        Star        = L'*',
    };
};

template <typename CaseSensitive caseSensitive, typename _Char>
static bool WildMatch_(const _Char *pat, const _Char *str)
{
    const CharEqualTo<_Char, caseSensitive> equalto;
    typedef WildChars_<_Char> chars;

    // Wildcard matching algorithms
    // http://xoomer.virgilio.it/acantato/dev/wildcard/wildmatch.html#evolution

    const _Char * s;
    const _Char * p;
    bool star = false;

loopStart:
    for (s = str, p = pat; *s; ++s, ++p) {
        switch (*p) {
        case chars::Question:
            if (*s == chars::Dot) goto starCheck;
            break;
        case chars::Star:
            star = true;
            str = s, pat = p;
            do { ++pat; } while (*pat == chars::Star);
            if (!*pat) return true;
            goto loopStart;
        default:
            if (equalto(*s, *p) == false)
                goto starCheck;
            break;
        }
    }
    while (*p == chars::Star) ++p;
    return (!*p);

starCheck:
   if (!star) return false;
   str++;
   goto loopStart;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template std::basic_string<char, std::char_traits<char>, ALLOCATOR(String, char)>;
template std::basic_string<wchar_t, std::char_traits<wchar_t>, ALLOCATOR(String, wchar_t)>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t hash_value(const char* cstr, size_t length) {
    return hash_value_Cstr_<CaseSensitive::True>(cstr, length, std::locale::classic());
}
//----------------------------------------------------------------------------
size_t hash_value(const wchar_t* wcstr, size_t length) {
    return hash_value_Cstr_<CaseSensitive::True>(wcstr, length, std::locale::classic());
}
//----------------------------------------------------------------------------
size_t hash_valueI(const char* cstr, size_t length) {
    return hash_value_Cstr_<CaseSensitive::False>(cstr, length, std::locale::classic());
}
//----------------------------------------------------------------------------
size_t hash_valueI(const wchar_t* wcstr, size_t length) {
    return hash_value_Cstr_<CaseSensitive::False>(wcstr, length, std::locale::classic());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool WildMatch(const char *pattern, const char *cstr) {
    return WildMatch_<CaseSensitive::True>(pattern, cstr);
}
//----------------------------------------------------------------------------
bool WildMatch(const wchar_t *wpattern, const wchar_t *wcstr) {
    return WildMatch_<CaseSensitive::True>(wpattern, wcstr);
}
//----------------------------------------------------------------------------
bool WildMatchI(const char *pattern, const char *cstr) {
    return WildMatch_<CaseSensitive::False>(pattern, cstr);
}
//----------------------------------------------------------------------------
bool WildMatchI(const wchar_t *wpattern, const wchar_t *wcstr) {
    return WildMatch_<CaseSensitive::False>(wpattern, wcstr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t ToCStr(char *dst, size_t capacity, const wchar_t *wcstr, size_t length) {
    Assert(dst);
    Assert(capacity);
    if (0 == length) {
        dst[0] = '\0';
        return 0;
    }

    Assert(wcstr);

    size_t written;
    if (0 != ::wcstombs_s(&written, dst, capacity, wcstr, capacity - 1) )
        AssertNotReached();

    Assert(written >= length);
    return written;
}
//----------------------------------------------------------------------------
size_t ToCStr(char *dst, size_t capacity, const wchar_t *wcstr) {
    return ToCStr(dst, capacity, wcstr, ::wcslen(wcstr));
}
//----------------------------------------------------------------------------
size_t ToCStr(char *dst, size_t capacity, const WString& wstr) {
    return ToCStr(dst, capacity, wstr.c_str(), wstr.size());
}
//----------------------------------------------------------------------------
size_t ToWCStr(wchar_t *dst, size_t capacity, const char *cstr, size_t length) {
Assert(dst);
    Assert(capacity);
    if (0 == length) {
        dst[0] = L'\0';
        return 0;
    }

    Assert(cstr);

    size_t written;
    if (0 != ::mbstowcs_s(&written, dst, capacity, cstr, capacity - 1) )
        AssertNotReached();

    Assert(written >= length);
    return written;
}
//----------------------------------------------------------------------------
size_t ToWCStr(wchar_t *dst, size_t capacity, const char *cstr) {
    return ToWCStr(dst, capacity, cstr, ::strlen(cstr));
}
//----------------------------------------------------------------------------
size_t ToWCStr(wchar_t *dst, size_t capacity, const String& str) {
    return ToWCStr(dst, capacity, str.c_str(), str.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
String ToString(const wchar_t *wcstr, size_t length) {
    if (0 == length)
        return String();

    Assert(wcstr);
    const auto buffer = MALLOCA_VIEW(char, length * 2);

    size_t written;
    if (0 != ::wcstombs_s(&written, buffer.Pointer(), buffer.SizeInBytes(), wcstr, buffer.SizeInBytes() - sizeof(char)) )
        AssertNotReached();

    Assert(written >= length);
    return String(buffer.Pointer());
}
//----------------------------------------------------------------------------
String ToString(const wchar_t *wcstr) {
    Assert(wcstr);
    return ToString(wcstr, ::wcslen(wcstr));
}
//----------------------------------------------------------------------------
String ToString(const WString& wstr) {
    return ToString(wstr.c_str(), wstr.size());
}
//----------------------------------------------------------------------------
WString ToWString(const char *cstr, size_t length) {
    if (0 == length)
        return WString();

    Assert(cstr);
    const auto buffer = MALLOCA_VIEW(wchar_t, length * 2);

    size_t written;
    if (0 != ::mbstowcs_s(&written, buffer.Pointer(), buffer.size(), cstr, buffer.size() - 1) )
        AssertNotReached();

    Assert(written >= length);
    return WString(buffer.Pointer());
}
//----------------------------------------------------------------------------
WString ToWString(const char *cstr) {
    Assert(cstr);
    return ToWString(cstr, ::strlen(cstr));
}
//----------------------------------------------------------------------------
WString ToWString(const String& str) {
    return ToWString(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define CORE_ATOIBASE_DEF(_Base) \
    template bool Atoi<_Base, int>(int *, const char *, size_t ); \
    template bool Atoi<_Base, unsigned>(unsigned *, const char *, size_t ); \
    template bool Atoi<_Base, i8> (i8  *, const char *, size_t ); \
    template bool Atoi<_Base, i16>(i16 *, const char *, size_t ); \
    template bool Atoi<_Base, i32>(i32 *, const char *, size_t ); \
    template bool Atoi<_Base, i64>(i64 *, const char *, size_t ); \
    template bool Atoi<_Base, u8> (u8  *, const char *, size_t ); \
    template bool Atoi<_Base, u16>(u16 *, const char *, size_t ); \
    template bool Atoi<_Base, u32>(u32 *, const char *, size_t ); \
    template bool Atoi<_Base, u64>(u64 *, const char *, size_t )
//----------------------------------------------------------------------------
CORE_ATOIBASE_DEF(2);
CORE_ATOIBASE_DEF(8);
CORE_ATOIBASE_DEF(10);
CORE_ATOIBASE_DEF(16);
//----------------------------------------------------------------------------
#undef CORE_ATOIBASE_DEF
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template bool Atof<float>(float *, const char *, size_t );
template bool Atof<double>(double *, const char *, size_t );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
