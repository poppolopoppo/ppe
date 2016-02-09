#include "stdafx.h"

#include "String.h"

#include "Memory/HashFunctions.h"
#include "Memory/UniqueView.h"

#include <string.h>

/*extern*/ template class std::basic_string<char, std::char_traits<char>, std::allocator<char>>;
/*extern*/ template class std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<char>>;

/*extern*/ template class std::basic_string<char, std::char_traits<char>, ALLOCATOR(String, char)>;
/*extern*/ template class std::basic_string<wchar_t, std::char_traits<wchar_t>, ALLOCATOR(String, wchar_t)>;

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
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

template <CaseSensitive caseSensitive, typename _Char>
static bool WildMatch_(const _Char *pat, const _Char *str)
{
    CharEqualTo<_Char, caseSensitive> equalto;
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
template <CaseSensitive _Sensitive, typename _Char>
static typename std::enable_if< CaseSensitive::True == _Sensitive, size_t >::type
    hash_string_(const _Char* cstr, size_t length) {
    return hash_mem(cstr, length*sizeof(_Char));
}
//----------------------------------------------------------------------------
template <CaseSensitive _Sensitive, typename _Char>
static typename std::enable_if< CaseSensitive::False == _Sensitive, size_t >::type
    hash_string_(const _Char* cstr, size_t length) {
    STACKLOCAL_POD_ARRAY(_Char, lower_cstr, length);
    forrange(i, 0, length)
        lower_cstr[i] = ToLower(cstr[i]);
    return hash_mem(lower_cstr, length*sizeof(_Char));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_string(const char* cstr, size_t length, CaseSensitive sensitive/* = CaseSensitive::True */) {
    return (CaseSensitive::True == sensitive)
        ? hash_string_<CaseSensitive::True>(cstr, length)
        : hash_string_<CaseSensitive::False>(cstr, length);
}
//----------------------------------------------------------------------------
hash_t hash_string(const wchar_t* wcstr, size_t length, CaseSensitive sensitive/* = CaseSensitive::True */) {
    return (CaseSensitive::True == sensitive)
        ? hash_string_<CaseSensitive::True>(wcstr, length)
        : hash_string_<CaseSensitive::False>(wcstr, length);
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
    STACKLOCAL_POD_ARRAY(char, buffer, (length * 3)/2);

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
    STACKLOCAL_POD_ARRAY(wchar_t, buffer, (length * 3)/2);

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
