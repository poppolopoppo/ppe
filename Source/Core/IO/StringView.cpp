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
static bool SplitIf_(TBasicStringView<_Char>& str, TBasicStringView<_Char>& slice, const _Pred& pred) {
    if (str.empty())
        return false;

    const auto it = str.FindIf(pred);
    if (str.end() == it) {
        slice = str;
        str = TBasicStringView<_Char>();
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
static bool Split_(TBasicStringView<_Char>& str, _Char separator, TBasicStringView<_Char>& slice) {
    return SplitIf_<_Char>(str, slice, [separator](const _Char ch) {
        return separator == ch;
    });
}
//----------------------------------------------------------------------------
template <typename _Char>
static bool SplitMulti_(TBasicStringView<_Char>& str, const TBasicStringView<_Char>& separators, TBasicStringView<_Char>& slice) {
    return SplitIf_<_Char>(str, slice, [separators](const _Char ch) {
        return (separators.end() != std::find(separators.begin(), separators.end(), ch));
    });
}
//----------------------------------------------------------------------------
template <typename _Char> struct TAtoN_traits {};
template <> struct TAtoN_traits<char> {
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
template <> struct TAtoN_traits<wchar_t> {
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
static bool Atoi_(T *dst, const TBasicStringView<_Char>& str, size_t base) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    Assert(1 < base && base <= 16);

    typedef TAtoN_traits<_Char> traits;

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
static bool Atof_(T *dst, const TBasicStringView<_Char>& str) {
    static_assert(std::is_floating_point<T>::value, "T must be a floating point type");

    typedef TAtoN_traits<_Char> traits;

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
struct TWildChars_ {};
template <>
struct TWildChars_< char > {
    enum : char { Dot = '.', Question = '?', Star = '*', };
};
template <>
struct TWildChars_< wchar_t > {
    enum : wchar_t { Dot = L'.', Question = L'?', Star = L'*', };
};
template <ECase _Sensitive, typename _Char>
static bool WildMatch_(const TBasicStringView<_Char>& pat, const TBasicStringView<_Char>& str)
{
    TCharEqualTo<_Char, _Sensitive> equalto;
    typedef TWildChars_<_Char> chars;
    typedef const typename TBasicStringView<_Char>::iterator iterator;

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
struct FCharFunctor_ {
    template <bool (*_Pred)(_Char)>
    bool operator ()(_Char ch) const {
        return _Pred(ch);
    }
};
//----------------------------------------------------------------------------
template <typename _Char>
static bool IsAll_(const TBasicStringView<_Char>& str, bool (*pred)(_Char)) {
    return (str.end() == str.FindIfNot(pred));
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicStringView<_Char> SplitIf_(const TBasicStringView<_Char>& str, bool (*pred)(_Char)) {
    return str.SplitIf(pred);
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicStringView<_Char> SplitInplaceIf_ReturnEaten_(TBasicStringView<_Char>& str, bool (*pred)(_Char)) {
    const auto it = str.FindIfNot(pred);
    const TBasicStringView<_Char> eaten = str.CutBefore(it);
    str = str.CutStartingAt(it);
    return eaten;
}
//----------------------------------------------------------------------------
template <typename _Char>
typename TBasicStringView<_Char>::iterator StrChr_(const TBasicStringView<_Char>& str, _Char ch) {
    return str.FindIf([ch](_Char c) { return (c == ch); });
}
//----------------------------------------------------------------------------
template <typename _Char>
typename TBasicStringView<_Char>::reverse_iterator StrRChr_(const TBasicStringView<_Char>& str, _Char ch) {
    return str.FindIfR([ch](_Char c) { return (c == ch); });
}
//----------------------------------------------------------------------------
template <typename _Char>
typename TBasicStringView<_Char>::iterator StrStr_(const TBasicStringView<_Char>& str, const TBasicStringView<_Char>& firstOccurence) {
    return str.FindSubRange(firstOccurence);
}
//----------------------------------------------------------------------------
template <typename _Char>
bool StartsWith_(const TBasicStringView<_Char>& str, const TBasicStringView<_Char>& prefix) {
    Assert(str.size());
    Assert(prefix.size());
    return (str.size() >= prefix.size() && Equals(str.CutBefore(prefix.size()), prefix) );
}
//----------------------------------------------------------------------------
template <typename _Char>
bool StartsWithI_(const TBasicStringView<_Char>& str, const TBasicStringView<_Char>& prefix) {
    Assert(str.size());
    Assert(prefix.size());
    return (str.size() >= prefix.size() && EqualsI(str.CutBefore(prefix.size()), prefix) );
}
//----------------------------------------------------------------------------
template <typename _Char>
bool EndsWith_(const TBasicStringView<_Char>& str, const TBasicStringView<_Char>& suffix) {
    Assert(str.size());
    Assert(suffix.size());
    return (str.size() >= suffix.size() && Equals(str.LastNElements(suffix.size()), suffix) );
}
//----------------------------------------------------------------------------
template <typename _Char>
bool EndsWithI_(const TBasicStringView<_Char>& str, const TBasicStringView<_Char>& suffix) {
    Assert(str.size());
    Assert(suffix.size());
    return (str.size() >= suffix.size() && EqualsI(str.LastNElements(suffix.size()), suffix) );
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView::iterator StrChr(const FStringView& str, char ch) { return StrChr_(str, ch); }
FStringView::reverse_iterator StrRChr(const FStringView& str, char ch) { return StrRChr_(str, ch); }
//----------------------------------------------------------------------------
FWStringView::iterator StrChr(const FWStringView& wstr, wchar_t wch) { return StrChr_(wstr, wch); }
FWStringView::reverse_iterator StrRChr(const FWStringView& wstr, wchar_t wch) { return StrRChr_(wstr, wch); }
//----------------------------------------------------------------------------
FStringView::iterator StrStr(const FStringView& str, const FStringView& firstOccurence) { return StrStr_(str, firstOccurence); }
FWStringView::iterator StrStr(const FWStringView& wstr, const FWStringView& firstOccurence) { return StrStr_(wstr, firstOccurence); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool IsAlnum(const FStringView& str) { return IsAll_(str, &IsAlnum); }
bool IsAlnum(const FWStringView& wstr) { return IsAll_(wstr, &IsAlnum); }
//----------------------------------------------------------------------------
bool IsAlpha(const FStringView& str) { return IsAll_(str, &IsAlpha); }
bool IsAlpha(const FWStringView& wstr) { return IsAll_(wstr, &IsAlpha); }
//----------------------------------------------------------------------------
bool IsDigit(const FStringView& str) { return IsAll_(str, &IsDigit); }
bool IsDigit(const FWStringView& wstr) { return IsAll_(wstr, &IsDigit); }
//----------------------------------------------------------------------------
bool IsXDigit(const FStringView& str) { return IsAll_(str, &IsXDigit); }
bool IsXDigit(const FWStringView& wstr) { return IsAll_(wstr, &IsXDigit); }
//----------------------------------------------------------------------------
bool IsIdentifier(const FStringView& str) { return IsAll_(str, &IsIdentifier); }
bool IsIdentifier(const FWStringView& wstr) { return IsAll_(wstr, &IsIdentifier); }
//----------------------------------------------------------------------------
bool IsPrint(const FStringView& str) { return IsAll_(str, &IsPrint); }
bool IsPrint(const FWStringView& wstr) { return IsAll_(wstr, &IsPrint); }
//----------------------------------------------------------------------------
bool IsSpace(const FStringView& str) { return IsAll_(str, &IsSpace); }
bool IsSpace(const FWStringView& wstr) { return IsAll_(wstr, &IsSpace); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView EatAlnums(FStringView& str) { return SplitInplaceIf_ReturnEaten_(str, &IsAlnum); }
FWStringView EatAlnums(FWStringView& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsAlnum); }
//----------------------------------------------------------------------------
FStringView EatAlphas(FStringView& str) { return SplitInplaceIf_ReturnEaten_(str, &IsAlpha); }
FWStringView EatAlphas(FWStringView& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsAlpha); }
//----------------------------------------------------------------------------
FStringView EatDigits(FStringView& str) { return SplitInplaceIf_ReturnEaten_(str, &IsDigit); }
FWStringView EatDigits(FWStringView& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsDigit); }
//----------------------------------------------------------------------------
FStringView EatXDigits(FStringView& str) { return SplitInplaceIf_ReturnEaten_(str, &IsXDigit); }
FWStringView EatXDigits(FWStringView& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsXDigit); }
//----------------------------------------------------------------------------
FStringView EatPrints(FStringView& str) { return SplitInplaceIf_ReturnEaten_(str, &IsPrint); }
FWStringView EatPrints(FWStringView& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsPrint); }
//----------------------------------------------------------------------------
FStringView EatSpaces(FStringView& str) { return SplitInplaceIf_ReturnEaten_(str, &IsSpace); }
FWStringView EatSpaces(FWStringView& wstr) { return SplitInplaceIf_ReturnEaten_(wstr, &IsSpace); }
//----------------------------------------------------------------------------
FStringView Chomp(const FStringView& str) { return SplitIf_(str, &IsEndLine); }
FWStringView Chomp(const FWStringView& wstr) { return SplitIf_(wstr, &IsEndLine); }
//----------------------------------------------------------------------------
FStringView Strip(const FStringView& str) {
    size_t first = 0;
    size_t last = str.size();
    for(; first < last && IsSpace(str[first]); ++first);
    for(; first < last && IsSpace(str[last - 1]); --last);
    return str.SubRange(first, last - first);
}
//----------------------------------------------------------------------------
FWStringView Strip(const FWStringView& wstr) {
    size_t first = 0;
    size_t last = wstr.size();
    for(; first < last && IsSpace(wstr[first]); ++first);
    for(; first < last && IsSpace(wstr[last - 1]); --last);
    return wstr.SubRange(first, last - first);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
int Compare(const FStringView& lhs, const FStringView& rhs) {
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
int Compare(const FWStringView& lhs, const FWStringView& rhs) {
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
int CompareI(const FStringView& lhs, const FStringView& rhs) {
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
int CompareI(const FWStringView& lhs, const FWStringView& rhs) {
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
bool StartsWith(const FStringView& str, const FStringView& prefix) {
    return StartsWith_(str, prefix);
}
//----------------------------------------------------------------------------
bool StartsWith(const FWStringView& wstr, const FWStringView& wprefix) {
    return StartsWith_(wstr, wprefix);
}
//----------------------------------------------------------------------------
bool StartsWithI(const FStringView& str, const FStringView& prefix) {
    return StartsWithI_(str, prefix);
}
//----------------------------------------------------------------------------
bool StartsWithI(const FWStringView& wstr, const FWStringView& wprefix) {
    return StartsWithI_(wstr, wprefix);
}
//----------------------------------------------------------------------------
bool EndsWith(const FStringView& str, const FStringView& suffix) {
    return EndsWith_(str, suffix);
}
//----------------------------------------------------------------------------
bool EndsWith(const FWStringView& wstr, const FWStringView& wsuffix) {
    return EndsWith_(wstr, wsuffix);
}
//----------------------------------------------------------------------------
bool EndsWithI(const FStringView& str, const FStringView& suffix) {
    return EndsWithI_(str, suffix);
}
//----------------------------------------------------------------------------
bool EndsWithI(const FWStringView& wstr, const FWStringView& wsuffix) {
    return EndsWithI_(wstr, wsuffix);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Split(FStringView& str, char separator, FStringView& slice) {
    return Split_<char>(str, separator, slice);
}
//----------------------------------------------------------------------------
bool Split(FWStringView& wstr, wchar_t separator, FWStringView& slice) {
    return Split_<wchar_t>(wstr, separator, slice);
}
//----------------------------------------------------------------------------
bool Split(FStringView& str, const FStringView& separators, FStringView& slice) {
    return SplitMulti_<char>(str, separators, slice);
}
//----------------------------------------------------------------------------
bool Split(FWStringView& wstr, const FWStringView& separators, FWStringView& slice) {
    return SplitMulti_<wchar_t>(wstr, separators, slice);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Atoi32(i32* dst, const FStringView& str, size_t base) {
    return Atoi_(dst, str, base);
}
//----------------------------------------------------------------------------
bool Atoi64(i64* dst, const FStringView& str, size_t base) {
    return Atoi_(dst, str, base);
}
//----------------------------------------------------------------------------
bool Atoi32(i32* dst, const FWStringView& wstr, size_t base) {
    return Atoi_(dst, wstr, base);
}
//----------------------------------------------------------------------------
bool Atoi64(i64* dst, const FWStringView& wstr, size_t base) {
    return Atoi_(dst, wstr, base);
}
//----------------------------------------------------------------------------
bool Atof(float* dst, const FStringView& str) {
    return Atof_(dst, str);
}
//----------------------------------------------------------------------------
bool Atod(double* dst, const FStringView& str) {
    return Atof_(dst, str);
}
//----------------------------------------------------------------------------
bool Atof(float* dst, const FWStringView& wstr) {
    return Atof_(dst, wstr);
}
//----------------------------------------------------------------------------
bool Atod(double* dst, const FWStringView& wstr) {
    return Atof_(dst, wstr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool WildMatch(const FStringView& pattern, const FStringView& str) {
    return WildMatch_<ECase::Sensitive, char>(pattern, str);
}
//----------------------------------------------------------------------------
bool WildMatch(const FWStringView& pattern, const FWStringView& wstr) {
    return WildMatch_<ECase::Sensitive, wchar_t>(pattern, wstr);
}
//----------------------------------------------------------------------------
bool WildMatchI(const FStringView& pattern, const FStringView& str) {
    return WildMatch_<ECase::Insensitive, char>(pattern, str);
}
//----------------------------------------------------------------------------
bool WildMatchI(const FWStringView& pattern, const FWStringView& wstr) {
    return WildMatch_<ECase::Insensitive, wchar_t>(pattern, wstr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t Copy(const TMemoryView<char>& dst, const FStringView& src) {
    const size_t n = std::min(dst.size(), src.size());
    std::copy(src.begin(), src.begin() + n, dst.begin());
    return n;
}
//----------------------------------------------------------------------------
size_t Copy(const TMemoryView<wchar_t>& dst, const FWStringView& src) {
    const size_t n = std::min(dst.size(), src.size());
    std::copy(src.begin(), src.begin() + n, dst.begin());
    return n;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_string(const FStringView& str) {
    return hash_mem(str.data(), str.SizeInBytes());
}
//----------------------------------------------------------------------------
hash_t hash_string(const FWStringView& wstr) {
    return hash_mem(wstr.data(), wstr.SizeInBytes());
}
//----------------------------------------------------------------------------
hash_t hash_stringI(const FStringView& str) {
    char (*transform)(char) = &ToLower;
    return hash_fnv1a(MakeOutputIterator(str.begin(), transform), MakeOutputIterator(str.end(), transform));
}
//----------------------------------------------------------------------------
hash_t hash_stringI(const FWStringView& wstr) {
    wchar_t (*transform)(wchar_t) = &ToLower;
    return hash_fnv1a(MakeOutputIterator(wstr.begin(), transform), MakeOutputIterator(wstr.end(), transform));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const FStringView& slice) {
    return oss.write(slice.data(), slice.size());
}
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const FWStringView& wslice) {
    return oss.write(wslice.data(), wslice.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
