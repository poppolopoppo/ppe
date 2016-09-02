#include "stdafx.h"

#include "Core/IO/StringView.h"

#include "Core/Memory/HashFunctions.h"

#include <ostream>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char, typename _Pred>
static bool SplitIf_(BasicStringView<_Char>& str, BasicStringView<_Char>& slice, const _Pred& pred) {
    if (str.empty())
        return false;

    const auto it = str.FindIf(pred);
    if (str.end() == it) {
        slice = str;
        str = BasicStringView<_Char>();
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
static bool Split_(BasicStringView<_Char>& str, _Char separator, BasicStringView<_Char>& slice) {
    return SplitIf_<_Char>(str, slice, [separator](const _Char ch) {
        return separator == ch;
    });
}
//----------------------------------------------------------------------------
template <typename _Char>
static bool SplitMulti_(BasicStringView<_Char>& str, const BasicStringView<_Char>& separators, BasicStringView<_Char>& slice) {
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
static bool Atoi_(T *dst, const BasicStringView<_Char>& str, size_t base) {
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
static bool Atof_(T *dst, const BasicStringView<_Char>& str) {
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
static bool WildMatch_(const BasicStringView<_Char>& pat, const BasicStringView<_Char>& str)
{
    CharEqualTo<_Char, _Sensitive> equalto;
    typedef WildChars_<_Char> chars;
    typedef const typename BasicStringView<_Char>::iterator iterator;

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
static bool IsAll_(const BasicStringView<_Char>& str, bool (*pred)(_Char)) {
    return (str.end() == str.FindIfNot(pred));
}
//----------------------------------------------------------------------------
template <typename _Char>
static BasicStringView<_Char> SplitIf_(const BasicStringView<_Char>& str, bool (*pred)(_Char)) {
    return str.SplitIf(pred);
}
//----------------------------------------------------------------------------
template <typename _Char>
static BasicStringView<_Char> SplitInplaceIf_ReturnEaten_(BasicStringView<_Char>& str, bool (*pred)(_Char)) {
    const auto it = str.FindIfNot(pred);
    const BasicStringView<_Char> eaten = str.CutBefore(it);
    str = str.CutStartingAt(it);
    return eaten;
}
//----------------------------------------------------------------------------
template <typename _Char>
typename BasicStringView<_Char>::iterator StrChr_(const BasicStringView<_Char>& str, _Char ch) {
    return str.FindIf([ch](_Char c) { return (c == ch); });
}
//----------------------------------------------------------------------------
template <typename _Char>
typename BasicStringView<_Char>::reverse_iterator StrRChr_(const BasicStringView<_Char>& str, _Char ch) {
    return str.FindIfR([ch](_Char c) { return (c == ch); });
}
//----------------------------------------------------------------------------
template <typename _Char>
typename BasicStringView<_Char>::iterator StrStr_(const BasicStringView<_Char>& str, const BasicStringView<_Char>& firstOccurence) {
    return str.FindSubRange(firstOccurence);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView::iterator StrChr(const StringView& str, char ch) { return StrChr_(str, ch); }
StringView::reverse_iterator StrRChr(const StringView& str, char ch) { return StrRChr_(str, ch); }
//----------------------------------------------------------------------------
WStringView::iterator StrChr(const WStringView& wstr, wchar_t wch) { return StrChr_(wstr, wch); }
WStringView::reverse_iterator StrRChr(const WStringView& wstr, wchar_t wch) { return StrRChr_(wstr, wch); }
//----------------------------------------------------------------------------
StringView::iterator StrStr(const StringView& str, const StringView& firstOccurence) { return StrStr_(str, firstOccurence); }
WStringView::iterator StrStr(const WStringView& wstr, const WStringView& firstOccurence) { return StrStr_(wstr, firstOccurence); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool IsAlnum(const StringView& str) { return IsAll_(str, &IsAlnum); }
bool IsAlnum(const WStringView& wstr) { return IsAll_(wstr, &IsAlnum); }
//----------------------------------------------------------------------------
bool IsAlpha(const StringView& str) { return IsAll_(str, &IsAlpha); }
bool IsAlpha(const WStringView& wstr) { return IsAll_(wstr, &IsAlpha); }
//----------------------------------------------------------------------------
bool IsDigit(const StringView& str) { return IsAll_(str, &IsDigit); }
bool IsDigit(const WStringView& wstr) { return IsAll_(wstr, &IsDigit); }
//----------------------------------------------------------------------------
bool IsXDigit(const StringView& str) { return IsAll_(str, &IsXDigit); }
bool IsXDigit(const WStringView& wstr) { return IsAll_(wstr, &IsXDigit); }
//----------------------------------------------------------------------------
bool IsPrint(const StringView& str) { return IsAll_(str, &IsPrint); }
bool IsPrint(const WStringView& wstr) { return IsAll_(wstr, &IsPrint); }
//----------------------------------------------------------------------------
bool IsSpace(const StringView& str) { return IsAll_(str, &IsSpace); }
bool IsSpace(const WStringView& wstr) { return IsAll_(wstr, &IsSpace); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView EatAlnums(StringView& str) { return SplitInplaceIf_ReturnEaten_(str, &IsAlnum); }
WStringView EatAlnums(WStringView& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsAlnum); }
//----------------------------------------------------------------------------
StringView EatAlphas(StringView& str) { return SplitInplaceIf_ReturnEaten_(str, &IsAlpha); }
WStringView EatAlphas(WStringView& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsAlpha); }
//----------------------------------------------------------------------------
StringView EatDigits(StringView& str) { return SplitInplaceIf_ReturnEaten_(str, &IsDigit); }
WStringView EatDigits(WStringView& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsDigit); }
//----------------------------------------------------------------------------
StringView EatXDigits(StringView& str) { return SplitInplaceIf_ReturnEaten_(str, &IsXDigit); }
WStringView EatXDigits(WStringView& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsXDigit); }
//----------------------------------------------------------------------------
StringView EatPrints(StringView& str) { return SplitInplaceIf_ReturnEaten_(str, &IsPrint); }
WStringView EatPrints(WStringView& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsPrint); }
//----------------------------------------------------------------------------
StringView EatSpaces(StringView& str) { return SplitInplaceIf_ReturnEaten_(str, &IsSpace); }
WStringView EatSpaces(WStringView& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsSpace); }
//----------------------------------------------------------------------------
StringView Chomp(const StringView& str) { return SplitIf_(str, &IsEndLine); }
WStringView Chomp(const WStringView& wstr) { return SplitIf_(wstr, &IsEndLine); }
//----------------------------------------------------------------------------
StringView Strip(const StringView& str) {
    size_t first = 0;
    size_t last = str.size();
    for(; first < last && IsSpace(str[first]); ++first);
    for(; first < last && IsSpace(str[last - 1]); --last);
    return str.SubRange(first, last - first);
}
//----------------------------------------------------------------------------
WStringView Strip(const WStringView& wstr) {
    size_t first = 0;
    size_t last = wstr.size();
    for(; first < last && IsSpace(wstr[first]); ++first);
    for(; first < last && IsSpace(wstr[last - 1]); --last);
    return wstr.SubRange(first, last - first);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
int Compare(const StringView& lhs, const StringView& rhs) {
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
int Compare(const WStringView& lhs, const WStringView& rhs) {
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
int CompareI(const StringView& lhs, const StringView& rhs) {
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
int CompareI(const WStringView& lhs, const WStringView& rhs) {
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
bool Split(StringView& str, char separator, StringView& slice) {
    return Split_<char>(str, separator, slice);
}
//----------------------------------------------------------------------------
bool Split(WStringView& wstr, wchar_t separator, WStringView& slice) {
    return Split_<wchar_t>(wstr, separator, slice);
}
//----------------------------------------------------------------------------
bool Split(StringView& str, const StringView& separators, StringView& slice) {
    return SplitMulti_<char>(str, separators, slice);
}
//----------------------------------------------------------------------------
bool Split(WStringView& wstr, const WStringView& separators, WStringView& slice) {
    return SplitMulti_<wchar_t>(wstr, separators, slice);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Atoi32(i32* dst, const StringView& str, size_t base) {
    return Atoi_(dst, str, base);
}
//----------------------------------------------------------------------------
bool Atoi64(i64* dst, const StringView& str, size_t base) {
    return Atoi_(dst, str, base);
}
//----------------------------------------------------------------------------
bool Atoi32(i32* dst, const WStringView& wstr, size_t base) {
    return Atoi_(dst, wstr, base);
}
//----------------------------------------------------------------------------
bool Atoi64(i64* dst, const WStringView& wstr, size_t base) {
    return Atoi_(dst, wstr, base);
}
//----------------------------------------------------------------------------
bool Atof(float* dst, const StringView& str) {
    return Atof_(dst, str);
}
//----------------------------------------------------------------------------
bool Atod(double* dst, const StringView& str) {
    return Atof_(dst, str);
}
//----------------------------------------------------------------------------
bool Atof(float* dst, const WStringView& wstr) {
    return Atof_(dst, wstr);
}
//----------------------------------------------------------------------------
bool Atod(double* dst, const WStringView& wstr) {
    return Atof_(dst, wstr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool WildMatch(const StringView& pattern, const StringView& str) {
    return WildMatch_<Case::Sensitive, char>(pattern, str);
}
//----------------------------------------------------------------------------
bool WildMatch(const WStringView& pattern, const WStringView& wstr) {
    return WildMatch_<Case::Sensitive, wchar_t>(pattern, wstr);
}
//----------------------------------------------------------------------------
bool WildMatchI(const StringView& pattern, const StringView& str) {
    return WildMatch_<Case::Insensitive, char>(pattern, str);
}
//----------------------------------------------------------------------------
bool WildMatchI(const WStringView& pattern, const WStringView& wstr) {
    return WildMatch_<Case::Insensitive, wchar_t>(pattern, wstr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t Copy(const MemoryView<char>& dst, const StringView& src) {
    const size_t n = std::min(dst.size(), src.size());
    std::copy(src.begin(), src.begin() + n, dst.begin());
    return n;
}
//----------------------------------------------------------------------------
size_t Copy(const MemoryView<wchar_t>& dst, const WStringView& src) {
    const size_t n = std::min(dst.size(), src.size());
    std::copy(src.begin(), src.begin() + n, dst.begin());
    return n;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_string(const StringView& str) {
    return hash_mem(str.data(), str.SizeInBytes());
}
//----------------------------------------------------------------------------
hash_t hash_string(const WStringView& wstr) {
    return hash_mem(wstr.data(), wstr.SizeInBytes());
}
//----------------------------------------------------------------------------
hash_t hash_stringI(const StringView& str) {
    char (*transform)(char) = &ToLower;
    return hash_fnv1a(MakeOutputIterator(str.begin(), transform), MakeOutputIterator(str.end(), transform));
}
//----------------------------------------------------------------------------
hash_t hash_stringI(const WStringView& wstr) {
    wchar_t (*transform)(wchar_t) = &ToLower;
    return hash_fnv1a(MakeOutputIterator(wstr.begin(), transform), MakeOutputIterator(wstr.end(), transform));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const StringView& slice) {
    return oss.write(slice.data(), slice.size());
}
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const WStringView& wslice) {
    return oss.write(wslice.data(), wslice.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
