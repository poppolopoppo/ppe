#pragma once

#include "HAL/Generic/GenericPlatformString.h"

#ifdef PLATFORM_WINDOWS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformString : FGenericPlatformString {
public:

    static bool Equals(const char* lhs, const char* rhs, size_t len) NOEXCEPT;
    static bool Equals(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT;

    static bool EqualsI(const char* lhs, const char* rhs, size_t len) NOEXCEPT;
    static bool EqualsI(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT;

    static int NCmp(const char* lhs, const char* rhs, size_t len) NOEXCEPT;
    static int NCmp(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT;

    static int NCmpI(const char* lhs, const char* rhs, size_t len) NOEXCEPT;
    static int NCmpI(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT;

    static size_t CHAR_to_WCHAR(ECodePage codePage, wchar_t* dst, size_t capacity, const char* cstr, size_t length);
    static size_t WCHAR_to_CHAR(ECodePage codePage, char* dst, size_t capacity, const wchar_t* wcstr, size_t length);

    static void ToLower(char* dst, const char* src, size_t len) NOEXCEPT;
    static void ToLower(wchar_t* dst, const wchar_t* src, size_t len) NOEXCEPT;

    static void ToUpper(char* dst, const char* src, size_t len) NOEXCEPT;
    static void ToUpper(wchar_t* dst, const wchar_t* src, size_t len) NOEXCEPT;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
