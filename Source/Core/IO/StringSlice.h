#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/IO/String.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
using BasicStringSlice = MemoryView<typename std::add_const<_Char>::type>;
//----------------------------------------------------------------------------
typedef BasicStringSlice<char>      StringSlice;
typedef BasicStringSlice<wchar_t>   WStringSlice;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char>, typename _Allocator >
BasicStringSlice<_Char> MakeStringSlice(const std::basic_string<_Char, _Traits, _Allocator>& str) {
    return BasicStringSlice<_Char>(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim>
BasicStringSlice<_Char> MakeStringSlice(const _Char(&cstr)[_Dim]) {
    static_assert(_Dim, "invalid string");
    Assert(!cstr[_Dim - 1]);
    return BasicStringSlice<_Char>(cstr, _Dim - 1 /* assume null terminated string */);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Split(const char **reentrantCstr, char separator, BasicStringSlice<char>& slice);
bool Split(const wchar_t **reentrantCstr, wchar_t separator, BasicStringSlice<wchar_t>& slice);
//----------------------------------------------------------------------------
bool Split(const char **reentrantCstr, const char* separators, BasicStringSlice<char>& slice);
bool Split(const wchar_t **reentrantCstr, const wchar_t* separators, BasicStringSlice<wchar_t>& slice);
//----------------------------------------------------------------------------
bool Split(const char **reentrantCstr, size_t *reentrantLength, char separator, BasicStringSlice<char>& slice);
bool Split(const wchar_t **reentrantCstr, size_t *reentrantLength, wchar_t separator, BasicStringSlice<wchar_t>& slice);
//----------------------------------------------------------------------------
bool Split(const char **reentrantCstr, size_t *reentrantLength, const char* separators, BasicStringSlice<char>& slice);
bool Split(const wchar_t **reentrantCstr, size_t *reentrantLength, const wchar_t* separators, BasicStringSlice<wchar_t>& slice);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive>
struct StringSliceEqualTo {
    bool operator ()(const BasicStringSlice<_Char>& lhs, const BasicStringSlice<_Char>& rhs) const {
        return (lhs == rhs) || ((lhs.size() == rhs.size()) && (0 == CompareN(lhs.begin(), rhs.begin(), lhs.size())));
    }
};
//----------------------------------------------------------------------------
template <typename _Char>
struct StringSliceEqualTo<_Char, CaseSensitive::False> {
    bool operator ()(const BasicStringSlice<_Char>& lhs, const BasicStringSlice<_Char>& rhs) const {
        return (lhs == rhs) || ((lhs.size() == rhs.size()) && (0 == CompareNI(lhs.begin(), rhs.begin(), lhs.size())));
    }
};
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive>
struct StringSliceLess {
    bool operator ()(const BasicStringSlice<_Char>& lhs, const BasicStringSlice<_Char>& rhs) const {
        if (lhs == rhs)
            return false;
        const size_t l = lhs.size() < rhs.size() ? lhs.size() : rhs.size();
        int cmp;
        if (0 == (cmp = CompareN(lhs.begin(), rhs.begin(), l)))
            return lhs.size() < rhs.size();
        else
            return cmp < 0;
    }
};
//----------------------------------------------------------------------------
template <typename _Char>
struct StringSliceLess<_Char, CaseSensitive::False> {
    bool operator ()(const BasicStringSlice<_Char>& lhs, const BasicStringSlice<_Char>& rhs) const {
        const size_t l = lhs.size() < rhs.size() ? lhs.size() : rhs.size();
        int cmp;
        if (0 == (cmp = CompareNI(lhs.begin(), rhs.begin(), l)))
            return lhs.size() < rhs.size();
        else
            return cmp < 0;
    }
};
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive>
struct StringSliceHasher {
    size_t operator ()(const BasicStringSlice<_Char>& cstr) const {
        return hash_string(cstr.begin(), cstr.size(), CaseSensitive::False);
    }
};
//----------------------------------------------------------------------------
template <typename _Char>
struct StringSliceHasher<_Char, CaseSensitive::False> {
    size_t operator ()(const BasicStringSlice<_Char>& cstr) const {
        return hash_string(cstr.begin(), cstr.size(), CaseSensitive::False);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const StringSlice& slice);
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const WStringSlice& wslice);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
