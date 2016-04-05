#include "stdafx.h"

#include "String.h"

#include "StringSlice.h"

#include <string.h>

/*extern*/ template class std::basic_string<char, std::char_traits<char>, std::allocator<char>>;
/*extern*/ template class std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<char>>;

/*extern*/ template class std::basic_string<char, std::char_traits<char>, ALLOCATOR(String, char)>;
/*extern*/ template class std::basic_string<wchar_t, std::char_traits<wchar_t>, ALLOCATOR(String, wchar_t)>;

/*extern*/ template class std::basic_string<char, std::char_traits<char>, THREAD_LOCAL_ALLOCATOR(String, char)>;
/*extern*/ template class std::basic_string<wchar_t, std::char_traits<wchar_t>, THREAD_LOCAL_ALLOCATOR(String, wchar_t)>;

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const String& str) {
    return hash_string(MakeStringSlice(str));
}
//----------------------------------------------------------------------------
hash_t hash_value(const WString& wstr) {
    return hash_string(MakeStringSlice(wstr));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
int Compare(const String& lhs, const String& rhs) {
    return Compare(MakeStringSlice(lhs), MakeStringSlice(rhs));
}
//----------------------------------------------------------------------------
int Compare(const WString& lhs, const WString& rhs) {
    return Compare(MakeStringSlice(lhs), MakeStringSlice(rhs));
}
//----------------------------------------------------------------------------
int CompareI(const String& lhs, const String& rhs) {
    return CompareI(MakeStringSlice(lhs), MakeStringSlice(rhs));
}
//----------------------------------------------------------------------------
int CompareI(const WString& lhs, const WString& rhs) {
    return CompareI(MakeStringSlice(lhs), MakeStringSlice(rhs));
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
} //!namespace Core
