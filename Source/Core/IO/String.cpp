#include "stdafx.h"

#include "String.h"

#include "StringBuilder.h"
#include "StringView.h"

#if not EXPORT_CORE_STRING
#   include "String-inl.h"
/*extern */template class Core::TBasicString<char>;
/*extern */template class Core::TBasicString<wchar_t>;
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t ToCStr(char *dst, size_t capacity, const wchar_t *wcstr, size_t length) {
    Assert(dst);
    Assert(capacity);
    Assert(capacity > length);
    if (0 == length) {
        dst[0] = '\0';
        return 0;
    }

    Assert(wcstr);

    size_t written;
    VerifyRelease(::wcstombs_s(&written, dst, capacity, wcstr, length) == 0);

    Assert(written >= length);
    Assert(written <= capacity);
    Assert(dst[written - 1] == '\0');
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
    Assert(capacity > length);
    if (0 == length) {
        dst[0] = L'\0';
        return 0;
    }

    Assert(cstr);

    size_t written;
    VerifyRelease(::mbstowcs_s(&written, dst, capacity, cstr, length) == 0);

    Assert(written >= length);
    Assert(written <= capacity);
    Assert(dst[written - 1] == L'\0');
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
FStringView ToCStr(const TMemoryView<char>& dst, const FWStringView& wstr) {
    size_t len = ToCStr(dst.data(), dst.size(), wstr.data(), wstr.size());
    Assert(0 == len || len - 1 == wstr.size());
    len = (len ? len - 1/* null char */ : 0);
    return FStringView(dst.data(), len);
}
//----------------------------------------------------------------------------
FWStringView ToWCStr(const TMemoryView<wchar_t>& dst, const FStringView& str) {
    size_t len = ToWCStr(dst.data(), dst.size(), str.data(), str.size());
    Assert(0 == len || len - 1 == str.size());
    len = (len ? len - 1/* null char */ : 0);
    return FWStringView(dst.data(), len);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString ToString(const wchar_t *wcstr, size_t length) {
    if (0 == length)
        return FString();

    Assert(wcstr);

    FStringBuilder sb;
    sb.reserve(length + 1);
    sb << FWStringView(wcstr, length);

    return sb.ToString();
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

    FWStringBuilder sb;
    sb.reserve(length + 1);
    sb << FStringView(cstr, length);

    return sb.ToString();
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
