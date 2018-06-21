#include "stdafx.h"

#include "String.h"

#include "StringBuilder.h"
#include "StringView.h"

#include "Diagnostic/LastError.h"
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"

#if not EXPORT_CORE_STRING
#   include "String-inl.h"

#   define CORE_INSTANTIATE_BASICSTRING(_CHAR) \
    template class Core::TBasicString<_CHAR>;

    CORE_INSTANTIATE_BASICSTRING(char)
    CORE_INSTANTIATE_BASICSTRING(wchar_t)

#   undef CORE_INSTANTIATE_BASICSTRING

#endif

namespace Core {
LOG_CATEGORY(CORE_API, String)
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
FWStringView ToWCStr(const TMemoryView<wchar_t>& dst, const FStringView& str) {
    size_t len = ToWCStr(dst.data(), dst.size(), str.data(), str.size());
    Assert(0 == len || len - 1 == str.size());
    len = (len ? len - 1/* null char */ : 0);
    return FWStringView(dst.data(), len);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// https://www.chilkatsoft.com/p/p_348.asp
//----------------------------------------------------------------------------
size_t WCHAR_to_CHAR(ECodePage codePage, char *dst, size_t capacity, const wchar_t *wcstr, size_t length) {
    Assert(dst);
    Assert(capacity);
    Assert(capacity > length);
    if (0 == length) {
        dst[0] = '\0';
        return 0;
    }

    Assert(wcstr);

    const size_t written = ::WideCharToMultiByte(
        int(codePage),
        0,
        wcstr, checked_cast<int>(length),
        dst, checked_cast<int>(capacity),
        0, 0 );
    CLOG(0 == written, String, Fatal, L"WCHAR_to_CHAR failed : {0}\n{1}", FLastError{}, Fmt::HexDump(wcstr, length));

    Assert(written >= length);
    Assert(written < capacity);

    dst[written] = '\0'; // WideCharToMultiByte(() won't use a null terminator since we specified the length of input

    return (written + 1);
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
    Assert(dst);
    Assert(capacity);
    Assert(capacity > length);
    if (0 == length) {
        dst[0] = L'\0';
        return 0;
    }

    Assert(cstr);

    const size_t written = ::MultiByteToWideChar(
        int(codePage),
        0,
        cstr, checked_cast<int>(length),
        dst, checked_cast<int>(capacity) );
    CLOG(0 == written, String, Fatal, L"CHAR_to_WCHAR failed : {0}\n{1}", FLastError{}, Fmt::HexDump(cstr, length));

    Assert(written >= length);
    Assert(written < capacity);

    dst[written] = L'\0'; // MultiByteToWideChar(() won't use a null terminator since we specified the length of input

    return (written + 1);
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
