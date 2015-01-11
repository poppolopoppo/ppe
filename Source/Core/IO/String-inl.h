#pragma once

#include "Core/IO/String.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Base, typename T>
bool Atoi(T *dst, const char *cstr, size_t length) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    static_assert(1 < _Base && _Base <= 16, "invalid _Base");

    Assert(dst);
    Assert(cstr);

    if (0 == length)
        return false;

    const bool neg = ('-' == cstr[0]);

    i64 v = 0;
    for (size_t i = neg ? 1 : 0; i < length; ++i) {
        const char ch = cstr[i];

        int d = 0;
        if (ch >= '0' && ch <= '9')
            d = ch - '0';
        else if (_Base > 10 && ch >= 'a' && ch <= 'f')
            d = ch - 'a' + 10;
        else if (_Base > 10 && ch >= 'A' && ch <= 'F')
            d = ch - 'A' + 10;
        else
            return false;

        Assert(d < _Base);
        v = v * _Base + d;
    }

    *dst = checked_cast<T>(neg ? -v : v);
    return true;
}
//----------------------------------------------------------------------------
template <size_t _Base, typename T, size_t _Capacity>
bool Atoi(T *dst, const char (&cstr)[_Capacity]) {
    return Atoi<_Base, T>(dst, cstr, _Capacity);
}
//----------------------------------------------------------------------------
template <size_t _Base, typename T>
bool Atoi(T *dst, const String& str) {
    return Atoi<_Base, T>(dst, str.c_str(), str.size());
}
//----------------------------------------------------------------------------
template <size_t _Base, typename T>
bool Atoi(T *dst, const MemoryView<const char>& strview) {
    return Atoi<_Base, T>(dst, strview.Pointer(), strview.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool Atof(T *dst, const char *cstr, size_t length) {
    static_assert(std::is_floating_point<T>::value, "T must be a floating point type");

    Assert(dst);
    Assert(cstr);

    if (0 == length)
        return false;

    size_t dot = 0;
    for (; dot < length && cstr[dot] != '.'; ++dot);

    const bool neg = ('-' == cstr[0]);

    i64 integral = 0;
    for (size_t i = neg ? 1 : 0; i < dot; ++i) {
        const char ch = cstr[i];

        i64 d = 0;
        if (ch >= '0' && ch <= '9')
            d = ch - '0';
        else
            return false;

        Assert(d < 10);
        integral = integral * 10 + d;
    }

    i64 fractional = 0;
    i64 unit = 1;
    for (size_t i = dot + 1; i < length; ++i, unit *= 10) {
        const char ch = cstr[i];

        int d = 0;
        if (ch >= '0' && ch <= '9')
            d = ch - '0';
        else
            return false;

        fractional = fractional * 10 + d;
    }

    const double result = integral + double(fractional)/unit;
    *dst = T(neg ? -result : result);

    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
bool Atof(T *dst, const char (&cstr)[_Capacity]) {
    return Atof<T>(dst, cstr, _Capacity);
}
//----------------------------------------------------------------------------
template <typename T>
bool Atof(T *dst, const String& str) {
    return Atof<T>(dst, str.c_str(), str.size());
}
//----------------------------------------------------------------------------
template <typename T>
bool Atof(T *dst, const MemoryView<const char>& strview) {
    return Atof<T>(dst, strview.Pointer(), strview.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Traits >
std::basic_ostream<char, _Traits>& operator <<(
    std::basic_ostream<char, _Traits>& oss,
    const wchar_t wch) {
    char ch[4];
    wcstombs_s(nullptr, ch, sizeof(ch), &wch, 1);
    return oss << ch;
}
//----------------------------------------------------------------------------
template <typename _Traits >
std::basic_ostream<char, _Traits>& operator <<(
    std::basic_ostream<char, _Traits>& oss,
    const wchar_t* wstr) {
    size_t len = 0;
    if (wstr && (len = Length(wstr)))
    {
        MALLOCA(char, str, len + 1);
        wcstombs_s(nullptr, str.get(), len + 1, wstr, len + 1);
        oss << str.get();
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _TraitsW, typename _Allocator >
std::basic_ostream<char, _Traits>& operator <<(
    std::basic_ostream<char, _Traits>& oss,
    const std::basic_string<wchar_t, _TraitsW, _Allocator>& wstr) {
    const size_t len = wstr.size();
    if (len)
    {
        MALLOCA(char, str, len + 1);
        wcstombs_s(nullptr, str.get(), len + 1, wstr.c_str(), len + 1);
        oss << str.get();
    }
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
