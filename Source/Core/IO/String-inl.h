#pragma once

#include "Core/IO/String.h"

namespace Core {
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
    if (wstr && 0 != (len = Length(wstr)))
    {
        STACKLOCAL_POD_ARRAY(char, str, len + 1 );
        wcstombs_s(nullptr, str.Pointer(), len + 1, wstr, len + 1);
        oss << str.Pointer();
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
        STACKLOCAL_POD_ARRAY(char, str, len + 1 );
        wcstombs_s(nullptr, str.Pointer(), len + 1, wstr.c_str(), len + 1);
        oss << str.Pointer();
    }
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
