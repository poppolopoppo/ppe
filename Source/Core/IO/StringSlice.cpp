#include "stdafx.h"

#include "Core/IO/StringSlice.h"

#include "Core/Memory/HashFunctions.h"

#include <ostream>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char, typename _Pred>
static bool SplitIf_(BasicStringSlice<_Char>& str, BasicStringSlice<_Char>& slice, const _Pred& pred) {
    if (str.empty())
        return false;

    const auto it = str.FindIf(pred);
    if (str.end() == it) {
        slice = str;
        str = BasicStringSlice<_Char>();
    }
    else {
        Assert(pred(*it));
        slice = str.CutBefore(it);
        str = str.CutStartingAt(it + 1);
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename _Char>
static bool Split_(BasicStringSlice<_Char>& str, _Char separator, BasicStringSlice<_Char>& slice) {
    return SplitIf_<_Char>(str, slice, [separator](const _Char ch) {
        return separator == ch;
    });
}
//----------------------------------------------------------------------------
template <typename _Char>
static bool SplitMulti_(BasicStringSlice<_Char>& str, const BasicStringSlice<_Char>& separators, BasicStringSlice<_Char>& slice) {
    return SplitIf_<_Char>(str, slice, [separators](const _Char ch) {
        return (separators.end() != std::find(separators.begin(), separators.end(), ch));
    });
}
//----------------------------------------------------------------------------
template <typename _Char> struct AtoN_traits {};
template <> struct AtoN_traits<char> {
    enum : char {
        Neg = '-',
        Dot = '.',
        _0  = '0',
        _9  = '9',
        a   = 'a',
        f   = 'f',
        A   = 'A',
        F   = 'F',
    };
};
template <> struct AtoN_traits<wchar_t> {
    enum : wchar_t {
        Neg = L'-',
        Dot = L'.',
        _0  = L'0',
        _9  = L'9',
        a   = L'a',
        f   = L'f',
        A   = L'A',
        F   = L'F',
    };
};
//----------------------------------------------------------------------------
template <typename T, typename _Char>
static bool Atoi_(T *dst, const BasicStringSlice<_Char>& str, size_t base) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    Assert(1 < base && base <= 16);

    typedef AtoN_traits<_Char> traits;

    Assert(dst);

    if (str.empty())
        return false;

    const bool neg = (traits::Neg == str[0]);

    i64 v = 0;
    for (size_t i = neg ? 1 : 0; i < str.size(); ++i) {
        const _Char ch = str[i];

        int d = 0;
        if (ch >= traits::_0 && ch <= traits::_9)
            d = ch - traits::_0;
        else if (base > 10 && ch >= traits::a && ch <= traits::f)
            d = ch - traits::a + 10;
        else if (base > 10 && ch >= traits::A && ch <= traits::F)
            d = ch - traits::A + 10;
        else
            return false;

        Assert(d < checked_cast<int>(base));
        v = v * base + d;
    }

    *dst = checked_cast<T>(neg ? -v : v);
    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Char>
static bool Atof_(T *dst, const BasicStringSlice<_Char>& str) {
    static_assert(std::is_floating_point<T>::value, "T must be a floating point type");

    typedef AtoN_traits<_Char> traits;

    Assert(dst);

    if (str.empty())
        return false;

    size_t dot = 0;
    for (; dot < str.size() && str[dot] != traits::Dot; ++dot);

    const bool neg = (traits::Neg == str[0]);

    i64 integral = 0;
    for (size_t i = neg ? 1 : 0; i < dot; ++i) {
        const _Char ch = str[i];

        i64 d = 0;
        if (ch >= traits::_0 && ch <= traits::_9)
            d = ch - traits::_0;
        else
            return false;

        Assert(d < 10);
        integral = integral * 10 + d;
    }

    i64 fractional = 0;
    i64 unit = 1;
    for (size_t i = dot + 1; i < str.size(); ++i, unit *= 10) {
        const _Char ch = str[i];

        int d = 0;
        if (ch >= traits::_0 && ch <= traits::_9)
            d = ch - traits::_0;
        else
            return false;

        fractional = fractional * 10 + d;
    }

    const double result = integral + fractional/double(unit);
    *dst = T(neg ? -result : result);

    return true;
}
//----------------------------------------------------------------------------
template <typename _Char>
struct WildChars_ {};
template <>
struct WildChars_< char > {
    enum : char { Dot = '.', Question = '?', Star = '*', };
};
template <>
struct WildChars_< wchar_t > {
    enum : wchar_t { Dot = L'.', Question = L'?', Star = L'*', };
};
template <Case _Sensitive, typename _Char>
static bool WildMatch_(const BasicStringSlice<_Char>& pat, const BasicStringSlice<_Char>& str)
{
    CharEqualTo<_Char, _Sensitive> equalto;
    typedef WildChars_<_Char> chars;
    typedef const typename BasicStringSlice<_Char>::iterator iterator;

    // Wildcard matching algorithms
    // http://xoomer.virgilio.it/acantato/dev/wildcard/wildmatch.html#evolution

    const _Char* sfirst = str.data();
    const _Char* pfirst = pat.data();

    const _Char* const send = str.data() + str.size();
    const _Char* const pend = pat.data() + pat.size();

    const _Char* s;
    const _Char* p;
    bool star = false;

loopStart:
    for (s = sfirst, p = pfirst; s != send; ++s, ++p) {
        switch (*p) {
        case chars::Question:
            if (*s == chars::Dot) goto starCheck;
            break;
        case chars::Star:
            star = true;
            sfirst = s, pfirst = p;
            do { ++pfirst; } while (*pfirst == chars::Star && pfirst != pend);
            if (pfirst == pend) return true;
            goto loopStart;
        default:
            if (equalto(*s, *p) == false)
                goto starCheck;
            break;
        }
    }
    while (*p == chars::Star && p != pend) ++p;
    return (p == pend);

starCheck:
   if (!star) return false;
   sfirst++;
   goto loopStart;
}
//----------------------------------------------------------------------------
template <typename _Char>
struct CharFunctor_ {
    template <bool (*_Pred)(_Char)>
    bool operator ()(_Char ch) const {
        return _Pred(ch);
    }
};
//----------------------------------------------------------------------------
template <typename _Char>
static bool IsAll_(const BasicStringSlice<_Char>& str, bool (*pred)(_Char)) {
    return (str.end() == str.FindIfNot(pred));
}
//----------------------------------------------------------------------------
template <typename _Char>
static BasicStringSlice<_Char> SplitIf_(const BasicStringSlice<_Char>& str, bool (*pred)(_Char)) {
    return str.SplitIf(pred);
}
//----------------------------------------------------------------------------
template <typename _Char>
static BasicStringSlice<_Char> SplitInplaceIf_ReturnEaten_(BasicStringSlice<_Char>& str, bool (*pred)(_Char)) {
    const auto it = str.FindIfNot(pred);
    const BasicStringSlice<_Char> eaten = str.CutBefore(it);
    str = str.CutStartingAt(it);
    return eaten;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool IsAlnum(const StringSlice& str) { return IsAll_(str, &IsAlnum); }
bool IsAlnum(const WStringSlice& wstr) { return IsAll_(wstr, &IsAlnum); }
//----------------------------------------------------------------------------
bool IsAlpha(const StringSlice& str) { return IsAll_(str, &IsAlpha); }
bool IsAlpha(const WStringSlice& wstr) { return IsAll_(wstr, &IsAlpha); }
//----------------------------------------------------------------------------
bool IsDigit(const StringSlice& str) { return IsAll_(str, &IsDigit); }
bool IsDigit(const WStringSlice& wstr) { return IsAll_(wstr, &IsDigit); }
//----------------------------------------------------------------------------
bool IsXDigit(const StringSlice& str) { return IsAll_(str, &IsXDigit); }
bool IsXDigit(const WStringSlice& wstr) { return IsAll_(wstr, &IsXDigit); }
//----------------------------------------------------------------------------
bool IsPrint(const StringSlice& str) { return IsAll_(str, &IsPrint); }
bool IsPrint(const WStringSlice& wstr) { return IsAll_(wstr, &IsPrint); }
//----------------------------------------------------------------------------
bool IsSpace(const StringSlice& str) { return IsAll_(str, &IsSpace); }
bool IsSpace(const WStringSlice& wstr) { return IsAll_(wstr, &IsSpace); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice EatAlnums(StringSlice& str) { return SplitInplaceIf_ReturnEaten_(str, &IsAlnum); }
WStringSlice EatAlnums(WStringSlice& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsAlnum); }
//----------------------------------------------------------------------------
StringSlice EatAlphas(StringSlice& str) { return SplitInplaceIf_ReturnEaten_(str, &IsAlpha); }
WStringSlice EatAlphas(WStringSlice& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsAlpha); }
//----------------------------------------------------------------------------
StringSlice EatDigits(StringSlice& str) { return SplitInplaceIf_ReturnEaten_(str, &IsDigit); }
WStringSlice EatDigits(WStringSlice& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsDigit); }
//----------------------------------------------------------------------------
StringSlice EatXDigits(StringSlice& str) { return SplitInplaceIf_ReturnEaten_(str, &IsXDigit); }
WStringSlice EatXDigits(WStringSlice& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsXDigit); }
//----------------------------------------------------------------------------
StringSlice EatPrints(StringSlice& str) { return SplitInplaceIf_ReturnEaten_(str, &IsPrint); }
WStringSlice EatPrints(WStringSlice& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsPrint); }
//----------------------------------------------------------------------------
StringSlice EatSpaces(StringSlice& str) { return SplitInplaceIf_ReturnEaten_(str, &IsSpace); }
WStringSlice EatSpaces(WStringSlice& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsSpace); }
//----------------------------------------------------------------------------
StringSlice Chomp(const StringSlice& str) { return SplitIf_(str, &IsEndLine); }
WStringSlice Chomp(const WStringSlice& wstr) { return SplitIf_(wstr, &IsEndLine); }
//----------------------------------------------------------------------------
StringSlice Strip(const StringSlice& str) {
    size_t first = 0;
    size_t last = str.size();
    for(; first < last && IsSpace(str[first]); ++first);
    for(; first < last && IsSpace(str[last - 1]); --last);
    return str.SubRange(first, last - first);
}
//----------------------------------------------------------------------------
WStringSlice Strip(const WStringSlice& wstr) {
    size_t first = 0;
    size_t last = wstr.size();
    for(; first < last && IsSpace(wstr[first]); ++first);
    for(; first < last && IsSpace(wstr[last - 1]); --last);
    return wstr.SubRange(first, last - first);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
int Compare(const StringSlice& lhs, const StringSlice& rhs) {
    if (lhs == rhs)
        return 0;

    const int cmp = ::strncmp(lhs.data(), rhs.data(), std::min(lhs.size(), rhs.size()));

    if (cmp)
        return cmp;
    else if (lhs.size() == rhs.size())
        return 0;
    else
        return (lhs.size() < rhs.size() ? -1 : 1);
}
//----------------------------------------------------------------------------
int Compare(const WStringSlice& lhs, const WStringSlice& rhs) {
    if (lhs == rhs)
        return 0;

    const int cmp = ::wcsncmp(lhs.data(), rhs.data(), std::min(lhs.size(), rhs.size()));

    if (cmp)
        return cmp;
    else if (lhs.size() == rhs.size())
        return 0;
    else
        return (lhs.size() < rhs.size() ? -1 : 1);
}
//----------------------------------------------------------------------------
int CompareI(const StringSlice& lhs, const StringSlice& rhs) {
    if (lhs == rhs)
        return 0;

    const int cmp = ::_strnicmp(lhs.data(), rhs.data(), std::min(lhs.size(), rhs.size()));

    if (cmp)
        return cmp;
    else if (lhs.size() == rhs.size())
        return 0;
    else
        return (lhs.size() < rhs.size() ? -1 : 1);
}
//----------------------------------------------------------------------------
int CompareI(const WStringSlice& lhs, const WStringSlice& rhs) {
    if (lhs == rhs)
        return 0;

    const int cmp = ::_wcsnicmp(lhs.data(), rhs.data(), std::min(lhs.size(), rhs.size()));

    if (cmp)
        return cmp;
    else if (lhs.size() == rhs.size())
        return 0;
    else
        return (lhs.size() < rhs.size() ? -1 : 1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Split(StringSlice& str, char separator, StringSlice& slice) {
    return Split_<char>(str, separator, slice);
}
//----------------------------------------------------------------------------
bool Split(WStringSlice& wstr, wchar_t separator, WStringSlice& slice) {
    return Split_<wchar_t>(wstr, separator, slice);
}
//----------------------------------------------------------------------------
bool Split(StringSlice& str, const StringSlice& separators, StringSlice& slice) {
    return SplitMulti_<char>(str, separators, slice);
}
//----------------------------------------------------------------------------
bool Split(WStringSlice& wstr, const WStringSlice& separators, WStringSlice& slice) {
    return SplitMulti_<wchar_t>(wstr, separators, slice);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Atoi32(i32* dst, const StringSlice& str, size_t base) {
    return Atoi_(dst, str, base);
}
//----------------------------------------------------------------------------
bool Atoi64(i64* dst, const StringSlice& str, size_t base) {
    return Atoi_(dst, str, base);
}
//----------------------------------------------------------------------------
bool Atoi32(i32* dst, const WStringSlice& wstr, size_t base) {
    return Atoi_(dst, wstr, base);
}
//----------------------------------------------------------------------------
bool Atoi64(i64* dst, const WStringSlice& wstr, size_t base) {
    return Atoi_(dst, wstr, base);
}
//----------------------------------------------------------------------------
bool Atof(float* dst, const StringSlice& str) {
    return Atof_(dst, str);
}
//----------------------------------------------------------------------------
bool Atod(double* dst, const StringSlice& str) {
    return Atof_(dst, str);
}
//----------------------------------------------------------------------------
bool Atof(float* dst, const WStringSlice& wstr) {
    return Atof_(dst, wstr);
}
//----------------------------------------------------------------------------
bool Atod(double* dst, const WStringSlice& wstr) {
    return Atof_(dst, wstr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool WildMatch(const StringSlice& pattern, const StringSlice& str) {
    return WildMatch_<Case::Sensitive, char>(pattern, str);
}
//----------------------------------------------------------------------------
bool WildMatch(const WStringSlice& pattern, const WStringSlice& wstr) {
    return WildMatch_<Case::Sensitive, wchar_t>(pattern, wstr);
}
//----------------------------------------------------------------------------
bool WildMatchI(const StringSlice& pattern, const StringSlice& str) {
    return WildMatch_<Case::Insensitive, char>(pattern, str);
}
//----------------------------------------------------------------------------
bool WildMatchI(const WStringSlice& pattern, const WStringSlice& wstr) {
    return WildMatch_<Case::Insensitive, wchar_t>(pattern, wstr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t Copy(const MemoryView<char>& dst, const StringSlice& src) {
    const size_t n = std::min(dst.size(), src.size());
    std::copy(src.begin(), src.begin() + n, dst.begin());
    return n;
}
//----------------------------------------------------------------------------
size_t Copy(const MemoryView<wchar_t>& dst, const WStringSlice& src) {
    const size_t n = std::min(dst.size(), src.size());
    std::copy(src.begin(), src.begin() + n, dst.begin());
    return n;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_string(const StringSlice& str) {
    return hash_mem(str.data(), str.SizeInBytes());
}
//----------------------------------------------------------------------------
hash_t hash_string(const WStringSlice& wstr) {
    return hash_mem(wstr.data(), wstr.SizeInBytes());
}
//----------------------------------------------------------------------------
hash_t hash_stringI(const StringSlice& str) {
    char (*transform)(char) = &ToLower;
    return hash_fnv1a(MakeOutputIterator(str.begin(), transform), MakeOutputIterator(str.end(), transform));
}
//----------------------------------------------------------------------------
hash_t hash_stringI(const WStringSlice& wstr) {
    wchar_t (*transform)(wchar_t) = &ToLower;
    return hash_fnv1a(MakeOutputIterator(wstr.begin(), transform), MakeOutputIterator(wstr.end(), transform));
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
