#pragma once

#include "HAL/TargetPlatform.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ECodePage : int {
    ACP             = 0,
    UTF_8           = 65001,
    Windows_1252    = 1252,
    ANSI            = Windows_1252,
};
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformString {
public: // must be defined for every platform

    static bool Equals(const char* lhs, const char* rhs, size_t len) = delete;
    static bool Equals(const wchar_t* lhs, const wchar_t* rhs, size_t len) = delete;

    static bool EqualsI(const char* lhs, const char* rhs, size_t len) = delete;
    static bool EqualsI(const wchar_t* lhs, const wchar_t* rhs, size_t len) = delete;

    static int NCmp(const char* lhs, const char* rhs, size_t len) = delete;
    static int NCmp(const wchar_t* lhs, const wchar_t* rhs, size_t len) = delete;

    static int NCmpI(const char* lhs, const char* rhs, size_t len) = delete;
    static int NCmpI(const wchar_t* lhs, const wchar_t* rhs, size_t len) = delete;

    static size_t CHAR_to_WCHAR(ECodePage codePage, wchar_t* dst, size_t capacity, const char* cstr, size_t length) = delete;
    static size_t WCHAR_to_CHAR(ECodePage codePage, char* dst, size_t capacity, const wchar_t* wcstr, size_t length) = delete;

    static void ToLower(char* dst, const char* src, size_t len) = delete;
    static void ToUpper(char* dst, const char* src, size_t len) = delete;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
