#pragma once

#include "StringSlice.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
bool Split(const _Char **reentrantCstr, _Char separator, BasicStringSlice<_Char>& slice) {
    Assert(reentrantCstr);

    const _Char* begin = *reentrantCstr;
    if (!*begin) {
        return false;
    }
    else if (separator == *begin && !begin[1]) {
        slice = BasicStringSlice<_Char>(begin + 1, 0);
        *reentrantCstr = slice.begin();

        return true;
    }
    else {
        const _Char* end = begin;
        while (*end && separator != *end)
            ++end;

        slice = BasicStringSlice<_Char>(begin, std::distance(begin, end));

        if (*end && end[1]) {
            Assert(separator == *end);
            ++end;
        }
        *reentrantCstr = end;

        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
bool Split(const _Char **reentrantCstr, const _Char* separators, BasicStringSlice<_Char>& slice) {
    Assert(reentrantCstr);
    Assert(separators);

    const _Char* begin = *reentrantCstr;
    if (!*begin) {
        return false;
    }
    else if (StrChr(separators, *begin) && !begin[1]) {
        slice = BasicStringSlice<_Char>(begin + 1, 0);
        *reentrantCstr = slice.begin();

        return true;
    }
    else {
        const _Char* end = begin;
        while (*end && !StrChr(separators, *end))
            ++end;

        slice = BasicStringSlice<_Char>(begin, std::distance(begin, end));

        if (*end && end[1]) {
            Assert(StrChr(separators, *end));
            ++end;
        }
        *reentrantCstr = end;

        return true;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
bool Split(const _Char **reentrantCstr, size_t *reentrantLength, _Char separator, BasicStringSlice<_Char>& slice) {
    Assert(reentrantCstr);
    Assert(reentrantLength);

    const _Char* begin = *reentrantCstr;
    if (0 == *reentrantLength) {
        return false;
    }
    else if (separator == *begin && 1 == *reentrantLength) {
        slice = BasicStringSlice<_Char>(begin + 1, 0);

        *reentrantCstr = slice.begin();
        *reentrantLength = 0;

        return true;
    }
    else {
        size_t length = *reentrantLength;

        const _Char* end = begin;
        for (; length && separator != *end; --length, ++end);

        slice = BasicStringSlice<_Char>(begin, std::distance(begin, end));

        if (length > 1) {
            Assert(separator == *end);
            ++end;
            --length;
        }

        *reentrantCstr = end;
        *reentrantLength = length;

        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
bool Split(const _Char **reentrantCstr, size_t *reentrantLength, const _Char* separators, BasicStringSlice<_Char>& slice) {
    Assert(reentrantCstr);
    Assert(reentrantLength);

    const _Char* begin = *reentrantCstr;
    if (0 == *reentrantLength) {
        return false;
    }
    else if (StrChr(separators, *begin) && 1 == *reentrantLength) {
        slice = BasicStringSlice<_Char>(begin + 1, 0);

        *reentrantCstr = slice.begin();
        *reentrantLength = 0;

        return true;
    }
    else {
        size_t length = *reentrantLength;

        const _Char* end = begin;
        for (; length && !StrChr(separators, *end); --length, ++end);

        slice = BasicStringSlice<_Char>(begin, std::distance(begin, end));

        if (length > 1) {
            Assert(StrChr(separators, *end) );
            ++end;
            --length;
        }

        *reentrantCstr = end;
        *reentrantLength = length;

        return true;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const BasicStringSlice<_Char>& slice) {
    return oss.write(slice.begin(), slice.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
