#include "stdafx.h"

#include "IO/String.h"

#include "HAL/PlatformString.h"

#include "IO/StringBuilder.h"
#include "IO/StringView.h"

#if EXTERN_TEMPLATE_STRING
#   include "IO/String-inl.h"
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) PPE::TBasicString<char>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) PPE::TBasicString<wchar_t>;
#endif

namespace PPE {
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
    const size_t written = FPlatformString::WCHAR_to_CHAR(ECodePage::ACP, dst, capacity, wcstr, length);

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
FStringView ToCStr(const TMemoryView<char>& dst, const FWStringView& wstr) {
    size_t len = ToCStr(dst.data(), dst.size(), wstr.data(), wstr.size());
    Assert(0 == len || len - 1 == wstr.size());
    len = (len ? len - 1/* null char */ : 0);
    return FStringView(dst.data(), len);
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
    const size_t written = FPlatformString::CHAR_to_WCHAR(ECodePage::ANSI, dst, capacity, cstr, length);

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
FWStringView ToWCStr(const TMemoryView<wchar_t>& dst, const FStringView& str) {
    size_t len = ToWCStr(dst.data(), dst.size(), str.data(), str.size());
    Assert(0 == len || len - 1 == str.size());
    len = (len ? len - 1/* null char */ : 0);
    return FWStringView(dst.data(), len);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t WCHAR_to_CHAR(ECodePage codePage, char *dst, size_t capacity, const wchar_t *wcstr, size_t length) {
    return FPlatformString::WCHAR_to_CHAR(codePage, dst, capacity, wcstr, length);
}
//----------------------------------------------------------------------------
size_t WCHAR_to_CHAR(ECodePage codePage, char *dst, size_t capacity, const wchar_t *wcstr) {
    return WCHAR_to_CHAR(codePage, dst, capacity, wcstr, ::wcslen(wcstr));
}
//----------------------------------------------------------------------------
size_t WCHAR_to_CHAR(ECodePage codePage, char *dst, size_t capacity, const FWString& wstr) {
    return WCHAR_to_CHAR(codePage, dst, capacity, wstr.c_str(), wstr.size());
}
//----------------------------------------------------------------------------
FStringView WCHAR_to_CHAR(ECodePage codePage, const TMemoryView<char>& dst, const FWStringView& wstr) {
    size_t len = WCHAR_to_CHAR(codePage, dst.data(), dst.size(), wstr.data(), wstr.size());
    len = (len ? len - 1/* null char */ : 0);
    return FStringView(dst.data(), len);
}
//----------------------------------------------------------------------------
size_t CHAR_to_WCHAR(ECodePage codePage, wchar_t *dst, size_t capacity, const char *cstr, size_t length) {
    return FPlatformString::CHAR_to_WCHAR(codePage, dst, capacity, cstr, length);
}
//----------------------------------------------------------------------------
size_t CHAR_to_WCHAR(ECodePage codePage, wchar_t *dst, size_t capacity, const char *cstr) {
    return CHAR_to_WCHAR(codePage, dst, capacity, cstr, ::strlen(cstr));
}
//----------------------------------------------------------------------------
size_t CHAR_to_WCHAR(ECodePage codePage, wchar_t *dst, size_t capacity, const FString& str) {
    return CHAR_to_WCHAR(codePage, dst, capacity, str.c_str(), str.size());
}
//----------------------------------------------------------------------------
FWStringView CHAR_to_WCHAR(ECodePage codePage, const TMemoryView<wchar_t>& dst, const FStringView& str) {
    size_t len = CHAR_to_WCHAR(codePage, dst.data(), dst.size(), str.data(), str.size());
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
FString ToString(const TMemoryView<const char>& strview) {
    return FString(strview.Pointer(), strview.size());
}
//----------------------------------------------------------------------------
FString ToString(const TMemoryView<const wchar_t>& strview) {
    return ToString(strview.Pointer(), strview.size());
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
FWString ToWString(const TMemoryView<const wchar_t>& strview) {
    return FWString(strview.Pointer(), strview.size());
}
//----------------------------------------------------------------------------
FWString ToWString(const TMemoryView<const char>& strview) {
    return ToWString(strview.Pointer(), strview.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
