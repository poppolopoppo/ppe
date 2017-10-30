#include "stdafx.h"

#include "String.h"

#include "StringView.h"

#include <string.h>

/*extern*/ template class std::basic_string<char, std::char_traits<char>, std::allocator<char>>;
/*extern*/ template class std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>;

/*extern*/ template class std::basic_string<char, std::char_traits<char>, ALLOCATOR(String, char)>;
/*extern*/ template class std::basic_string<wchar_t, std::char_traits<wchar_t>, ALLOCATOR(String, wchar_t)>;

/*extern*/ template class std::basic_string<char, std::char_traits<char>, THREAD_LOCAL_ALLOCATOR(String, char)>;
/*extern*/ template class std::basic_string<wchar_t, std::char_traits<wchar_t>, THREAD_LOCAL_ALLOCATOR(String, wchar_t)>;

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const FString& str) {
    return hash_string(MakeStringView(str));
}
//----------------------------------------------------------------------------
hash_t hash_value(const FWString& wstr) {
    return hash_string(MakeStringView(wstr));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
int Compare(const FString& lhs, const FString& rhs) {
    return Compare(MakeStringView(lhs), MakeStringView(rhs));
}
//----------------------------------------------------------------------------
int Compare(const FWString& lhs, const FWString& rhs) {
    return Compare(MakeStringView(lhs), MakeStringView(rhs));
}
//----------------------------------------------------------------------------
int CompareI(const FString& lhs, const FString& rhs) {
    return CompareI(MakeStringView(lhs), MakeStringView(rhs));
}
//----------------------------------------------------------------------------
int CompareI(const FWString& lhs, const FWString& rhs) {
    return CompareI(MakeStringView(lhs), MakeStringView(rhs));
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
size_t ToCStr(char *dst, size_t capacity, const FWString& wstr) {
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
size_t ToWCStr(wchar_t *dst, size_t capacity, const FString& str) {
    return ToWCStr(dst, capacity, str.c_str(), str.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString ToString(const wchar_t *wcstr, size_t length) {
    if (0 == length)
        return FString();

    Assert(wcstr);
    STACKLOCAL_POD_ARRAY(char, buffer, (length * 3)/2);

    size_t written;
    if (0 != ::wcstombs_s(&written, buffer.Pointer(), buffer.SizeInBytes(), wcstr, buffer.SizeInBytes() - sizeof(char)) )
        AssertNotReached();

    Assert(written >= length);
    return FString(buffer.Pointer());
}
//----------------------------------------------------------------------------
FString ToString(const wchar_t *wcstr) {
    Assert(wcstr);
    return ToString(wcstr, ::wcslen(wcstr));
}
//----------------------------------------------------------------------------
FString ToString(const FWString& wstr) {
    return ToString(wstr.c_str(), wstr.size());
}
//----------------------------------------------------------------------------
FWString ToWString(const char *cstr, size_t length) {
    if (0 == length)
        return FWString();

    Assert(cstr);
    STACKLOCAL_POD_ARRAY(wchar_t, buffer, (length * 3)/2);

    size_t written;
    if (0 != ::mbstowcs_s(&written, buffer.Pointer(), buffer.size(), cstr, buffer.size() - 1) )
        AssertNotReached();

    Assert(written >= length);
    return FWString(buffer.Pointer());
}
//----------------------------------------------------------------------------
FWString ToWString(const char *cstr) {
    Assert(cstr);
    return ToWString(cstr, ::strlen(cstr));
}
//----------------------------------------------------------------------------
FWString ToWString(const FString& str) {
    return ToWString(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
