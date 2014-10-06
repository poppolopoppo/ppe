#pragma once

#include "String.h"

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

    bool neg = false;

    T v = 0;
    T unit = 1;

    for (size_t i = length; i; --i, unit *= _Base) {
        const char ch = cstr[i - 1];

        T d = 0;
        if (ch >= '0' && ch <= '9')
            d = ch - '0';
        else if (_Base > 10 && ch >= 'a' && ch <= 'f')
            d = ch - 'a' + 10;
        else if (_Base > 10 && ch >= 'A' && ch <= 'F')
            d = ch - 'A' + 10;
        else if (ch == '-') {
            if (i != 1)
                return false;

            neg = true;
        }
        else {
            return false;
        }

        Assert(d < _Base);
        v += d * unit;
    }

    *dst = (neg) ? T(-i64(v)) : v;
    return true;
}
//----------------------------------------------------------------------------
template <size_t _Base, typename T, size_t _Capacity>
bool Atoi(T *dst, const char (&cstr)[_Capacity]) {
    return Atoi<_Base, T>(dst, cstr, _Capacity);
}
//----------------------------------------------------------------------------
template <size_t _Base, typename T, size_t _Capacity>
bool Atoi(T *dst, const String& str) {
    return Atoi<_Base, T>(dst, str.c_str(), str.size());
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
