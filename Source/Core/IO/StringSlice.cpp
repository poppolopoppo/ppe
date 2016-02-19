#include "stdafx.h"

#include "Core/IO/StringSlice.h"

#include <ostream>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
bool Split_(const _Char **reentrantCstr, _Char separator, BasicStringSlice<_Char>& slice) {
    Assert(reentrantCstr);

    const _Char* begin = *reentrantCstr;
    if (!*begin) {
        return false;
    }
    else if (separator == *begin && !begin[1]) {
        slice = BasicStringSlice<_Char>(begin + 1, 0);
        *reentrantCstr = slice.data();

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
bool Split_(const _Char **reentrantCstr, const _Char* separators, BasicStringSlice<_Char>& slice) {
    Assert(reentrantCstr);
    Assert(separators);

    const _Char* begin = *reentrantCstr;
    if (!*begin) {
        return false;
    }
    else if (StrChr(separators, *begin) && !begin[1]) {
        slice = BasicStringSlice<_Char>(begin + 1, 0);
        *reentrantCstr = slice.data();

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
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
bool Split_(const _Char **reentrantCstr, size_t *reentrantLength, _Char separator, BasicStringSlice<_Char>& slice) {
    Assert(reentrantCstr);
    Assert(reentrantLength);

    const _Char* begin = *reentrantCstr;
    if (0 == *reentrantLength) {
        return false;
    }
    else if (separator == *begin && 1 == *reentrantLength) {
        slice = BasicStringSlice<_Char>(begin + 1, 0);

        *reentrantCstr = slice.data();
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
bool Split_(const _Char **reentrantCstr, size_t *reentrantLength, const _Char* separators, BasicStringSlice<_Char>& slice) {
    Assert(reentrantCstr);
    Assert(reentrantLength);

    const _Char* begin = *reentrantCstr;
    if (0 == *reentrantLength) {
        return false;
    }
    else if (StrChr(separators, *begin) && 1 == *reentrantLength) {
        slice = BasicStringSlice<_Char>(begin + 1, 0);

        *reentrantCstr = slice.data();
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
template <typename _Char>
typename MemoryView<const _Char>::iterator FindCharIf_(
    const MemoryView<const _Char>& str,
    bool (*pred)(_Char) ) {
    return str.find_if(pred);
}
//----------------------------------------------------------------------------
template <typename _Char>
bool FindCharIf_ReturnIfNot_(const MemoryView<const _Char>& str, bool (*pred)(_Char) ) {
    return (str.end() == FindCharIf_(str, pred));
}
//----------------------------------------------------------------------------
template <typename _Char>
size_t FindCharIf_ReturnIndex_(const MemoryView<const _Char>& str, bool (*pred)(_Char) ) {
    return std::distance(str.begin(), FindCharIf_(str, pred));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Split(const char **reentrantCstr, char separator, BasicStringSlice<char>& slice) {
    return Split_(reentrantCstr, separator, slice);
}
//----------------------------------------------------------------------------
bool Split(const wchar_t **reentrantCstr, wchar_t separator, BasicStringSlice<wchar_t>& slice) {
    return Split_(reentrantCstr, separator, slice);
}
//----------------------------------------------------------------------------
bool Split(const char **reentrantCstr, const char* separators, BasicStringSlice<char>& slice) {
    return Split_(reentrantCstr, separators, slice);
}
//----------------------------------------------------------------------------
bool Split(const wchar_t **reentrantCstr, const wchar_t* separators, BasicStringSlice<wchar_t>& slice) {
    return Split_(reentrantCstr, separators, slice);
}
//----------------------------------------------------------------------------
bool Split(const char **reentrantCstr, size_t *reentrantLength, char separator, BasicStringSlice<char>& slice) {
    return Split_(reentrantCstr, reentrantLength, separator, slice);
}
//----------------------------------------------------------------------------
bool Split(const wchar_t **reentrantCstr, size_t *reentrantLength, wchar_t separator, BasicStringSlice<wchar_t>& slice) {
    return Split_(reentrantCstr, reentrantLength, separator, slice);
}
//----------------------------------------------------------------------------
bool Split(const char **reentrantCstr, size_t *reentrantLength, const char* separators, BasicStringSlice<char>& slice) {
    return Split_(reentrantCstr, reentrantLength, separators, slice);
}
//----------------------------------------------------------------------------
bool Split(const wchar_t **reentrantCstr, size_t *reentrantLength, const wchar_t* separators, BasicStringSlice<wchar_t>& slice) {
    return Split_(reentrantCstr, reentrantLength, separators, slice);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice Chomp(const StringSlice& line) {
    return StringSlice(line.data(), FindCharIf_ReturnIndex_(line, &IsEndLine));
}
//----------------------------------------------------------------------------
WStringSlice Chomp(const WStringSlice& line) {
    return WStringSlice(line.data(), FindCharIf_ReturnIndex_(line, &IsEndLine));
}
//----------------------------------------------------------------------------
bool IsAlnum(const StringSlice& str) {
    return FindCharIf_ReturnIfNot_(str, &IsAlnum);
}
//----------------------------------------------------------------------------
bool IsAlnum(const WStringSlice& wstr) {
    return FindCharIf_ReturnIfNot_(wstr, &IsAlnum);
}
//----------------------------------------------------------------------------
bool IsAlpha(const StringSlice& str) {
    return FindCharIf_ReturnIfNot_(str, &IsAlpha);
}
//----------------------------------------------------------------------------
bool IsAlpha(const WStringSlice& wstr) {
    return FindCharIf_ReturnIfNot_(wstr, &IsAlpha);
}
//----------------------------------------------------------------------------
bool IsDigit(const StringSlice& str) {
    return FindCharIf_ReturnIfNot_(str, &IsDigit);
}
//----------------------------------------------------------------------------
bool IsDigit(const WStringSlice& wstr) {
    return FindCharIf_ReturnIfNot_(wstr, &IsDigit);
}
//----------------------------------------------------------------------------
bool IsXDigit(const StringSlice& str) {
    return FindCharIf_ReturnIfNot_(str, &IsXDigit);
}
//----------------------------------------------------------------------------
bool IsXDigit(const WStringSlice& wstr) {
    return FindCharIf_ReturnIfNot_(wstr, &IsXDigit);
}
//----------------------------------------------------------------------------
bool IsPrint(const StringSlice& str) {
    return FindCharIf_ReturnIfNot_(str, &IsPrint);
}
//----------------------------------------------------------------------------
bool IsPrint(const WStringSlice& wstr) {
    return FindCharIf_ReturnIfNot_(wstr, &IsPrint);
}
//----------------------------------------------------------------------------
bool IsSpace(const StringSlice& str) {
    return FindCharIf_ReturnIfNot_(str, &IsSpace);
}
//----------------------------------------------------------------------------
bool IsSpace(const WStringSlice& wstr) {
    return FindCharIf_ReturnIfNot_(wstr, &IsSpace);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const StringSlice& slice) {
    return oss.write(slice.data(), slice.size());
}
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const WStringSlice& wslice) {
    return oss.write(wslice.data(), wslice.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
