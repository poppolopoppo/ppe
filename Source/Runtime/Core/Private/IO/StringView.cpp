#include "stdafx.h"

#include "IO/StringView.h"

#include "Allocator/Alloca.h"
#include "HAL/PlatformMemory.h"
#include "IO/StaticString.h"
#include "IO/String.h"
#include "IO/TextWriter.h"
#include "HAL/PlatformString.h"
#include "Memory/HashFunctions.h"

#include "double-conversion-external.h"

namespace PPE {
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
        Assert_NoAssume(pred(*it));
        slice = str.CutBefore(it);
        str = str.CutStartingAt(it + 1);
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Pred>
static bool SplitIfR_(TBasicStringView<_Char>& str, TBasicStringView<_Char>& slice, _Pred&& pred) {
    if (str.empty())
        return false;

    const auto it = str.FindIfR(pred);

    if (str.rend() == it) {
        slice = str;
        str = TBasicStringView<_Char>();
    }
    else {
        Assert_NoAssume(pred(*it));
        slice = str.CutStartingAt(it - 1);
        str = str.CutBefore(it);
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename _Char, bool _FindFirst>
static bool Split_(TBasicStringView<_Char>& str, _Char separator, TBasicStringView<_Char>& slice) {
    auto pred = [separator](const _Char ch) {
        return separator == ch;
    };
    IF_CONSTEXPR(_FindFirst)
        return SplitIf_<_Char>(str, slice, std::move(pred));
    else
        return SplitIfR_<_Char>(str, slice, std::move(pred));
}
//----------------------------------------------------------------------------
template <typename _Char, bool _FindFirst>
static bool SplitMulti_(TBasicStringView<_Char>& str, const TBasicStringView<_Char>& separators, TBasicStringView<_Char>& slice) {
    auto pred = [separators](const _Char ch) {
        return (separators.end() != std::find(separators.begin(), separators.end(), ch));
    };
    IF_CONSTEXPR(_FindFirst)
        return SplitIf_<_Char>(str, slice, std::move(pred));
    else
        return SplitIfR_<_Char>(str, slice, std::move(pred));
}
//----------------------------------------------------------------------------
template <typename _Char, bool _FindFirst, typename _Separator>
static bool SplitNth_(TBasicStringView<_Char>& str, _Separator separator, TBasicStringView<_Char>& slice, size_t nth) {
    for (++nth; nth; --nth) {
        IF_CONSTEXPR(_FindFirst) {
            if (not Split(str, separator, slice))
                return false;
        }
        else {
            if (not SplitR(str, separator, slice))
                return false;
        }
    }
    Assert(0 == nth);
    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Char>
static bool Atoi_(T *dst, const TBasicStringView<_Char>& str, size_t base) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    Assert(1 < base && base <= 16);

    Assert(dst);

    if (str.empty())
        return false;

#define STR(X) STRING_LITERAL(_Char, X)
    STATIC_ASSERT(STR('9') - STR('0') == 9);
    STATIC_ASSERT(STR('f') - STR('a') == 5);
    STATIC_ASSERT(STR('F') - STR('A') == 5);

    const bool neg = (STR('-') == str[0]);
    Assert(not neg || not std::is_unsigned_v<T>);

    i64 v = 0;
    for (size_t i = size_t(neg); i < str.size(); ++i) {
        const _Char ch = str[i];

        int d;
        if (ch >= STR('0') && ch <= STR('9'))
            d = ch - STR('0');
        else if (base > 10 && ch >= STR('a') && ch <= STR('f'))
            d = ch - STR('a') + 10;
        else if (base > 10 && ch >= STR('A') && ch <= STR('F'))
            d = ch - STR('A') + 10;
        else
            return false;

        Assert(d < checked_cast<int>(base));
        v = T(v * base + d);
    }

#undef STR

    *dst = checked_cast<T>(neg ? -v : v);
    return true;
}
//----------------------------------------------------------------------------
#if 0 // old custom method
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

    size_t scientific = INDEX_NONE;
    i64 fractional = 0;
    i64 unit = 1;
    for (size_t i = dot + 1; i < str.size(); ++i, unit *= 10) {
        const _Char ch = str[i];

        int d = 0;
        if (ch >= traits::_0 && ch <= traits::_9)
            d = ch - traits::_0;
        else if (ch == traits::E || ch == traits::e) {
            scientific = (i + 1);
            break;
        }
        else {
            return false;
        }

        fractional = fractional * 10 + d;
    }

    double result = integral + fractional/double(unit);
    if (neg)
        result = -result;

    if (INDEX_NONE != scientific) {
        Assert(traits::e == ToLower(str[scientific]));

        if (str[scientific] == traits::Pos)
            ++scientific; // skip positive sign

        i64 exponent = 0;
        const TBasicStringView<_Char> substr(str.CutStartingAt(scientific));
        if (not Atoi_(&exponent, substr, 10))
            return false;

        result *= std::pow(10., double(exponent));
    }

    *dst = T(result);

    return true;
}
#else // but now prefer using double-conversion since it's already integrated for inverse operation

static const double_conversion::StringToDoubleConverter& DefaultStringToDoubleConverter_() {
    ONE_TIME_INITIALIZE(
        const double_conversion::StringToDoubleConverter,
        GStringToDoubleConverter_,
        double_conversion::StringToDoubleConverter::NO_FLAGS,
        double(NAN), double(NAN), "Inf", "NaN" );
    return GStringToDoubleConverter_;
}

const char* StringToDoubleStr_(const char* str) { return str; }
auto StringToDoubleStr_(const wchar_t* wstr) {
    IF_CONSTEXPR(sizeof(wchar_t) == sizeof(u16))
        return reinterpret_cast<const u16*>(wstr);
    else
        return WCHAR_TO_UTF_8<128>(wstr); // for platforms were wchar_t is 32 bits
}

template <typename _Char>
static bool Atof_(float *dst, const TBasicStringView<_Char>& str) {
    int len;
    *dst = DefaultStringToDoubleConverter_().StringToFloat(
        StringToDoubleStr_(str.data()),
        checked_cast<int>(str.size()),
        &len );

    return (checked_cast<size_t>(len) == str.size());
}

template <typename _Char>
static bool Atof_(double *dst, const TBasicStringView<_Char>& str) {
    int len;
    *dst = DefaultStringToDoubleConverter_().StringToDouble(
        StringToDoubleStr_(str.data()),
        checked_cast<int>(str.size()),
        &len );

    return (checked_cast<size_t>(len) == str.size());
}

#endif
//----------------------------------------------------------------------------
template <ECase _Sensitive, typename _Char>
static bool WildMatch_(const TBasicStringView<_Char>& pat, const TBasicStringView<_Char>& str)
{
    typedef TCharEqualTo<_Char, _Sensitive> equalto;

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
        case STRING_LITERAL(_Char, '?'):
            if (*s == STRING_LITERAL(_Char, '.')) goto starCheck;
            break;
        case STRING_LITERAL(_Char, '*'):
            star = true;
            sfirst = s, pfirst = p;
            do { ++pfirst; } while (*pfirst == STRING_LITERAL(_Char, '*') && pfirst != pend);
            if (pfirst == pend) return true;
            goto loopStart;
        default:
            if (equalto()(*s, *p) == false)
                goto starCheck;
            break;
        }
    }
    while (*p == STRING_LITERAL(_Char, '*') && p != pend) ++p;
    return (p == pend);

starCheck:
   if (!star) return false;
   sfirst++;
   goto loopStart;
}
//----------------------------------------------------------------------------
/*
 * This function implements the Damerau-Levenshtein algorithm to
 * calculate a distance between strings.
 *
 * Basically, it says how many letters need to be swapped, substituted,
 * deleted from, or added to string1, at least, to get string2.
 *
 * The idea is to build a distance matrix for the substrings of both
 * strings.  To avoid a large space complexity, only the last three rows
 * are kept in memory (if swaps had the same or higher cost as one deletion
 * plus one insertion, only two rows would be needed).
 *
 * At any stage, "i + 1" denotes the length of the current substring of
 * string1 that the distance is calculated for.
 *
 * row2 holds the current row, row1 the previous row (i.e. for the substring
 * of string1 of length "i"), and row0 the row before that.
 *
 * In other words, at the start of the big loop, row2[j + 1] contains the
 * Damerau-Levenshtein distance between the substring of string1 of length
 * "i" and the substring of string2 of length "j + 1".
 *
 * All the big loop does is determine the partial minimum-cost paths.
 *
 * It does so by calculating the costs of the path ending in characters
 * i (in string1) and j (in string2), respectively, given that the last
 * operation is a substitution, a swap, a deletion, or an insertion.
 *
 * This implementation allows the costs to be weighted:
 *
 * - w (as in "sWap")
 * - s (as in "Substitution")
 * - a (for insertion, AKA "Add")
 * - d (as in "Deletion")
 *
 * Note that this algorithm calculates a distance _iff_ d == a.
 */
// https://github.com/gitster/git/blob/master/levenshtein.c
template <typename _Char, ECase _Sensitive>
static size_t LevenshteinDistance_(
    TBasicStringView<_Char> str1,
    TBasicStringView<_Char> str2,
    const u32 w, const u32 s, const u32 a, const u32 d) {
    typedef TCharEqualTo<_Char, _Sensitive> equalto;

    if (str1.empty()) return (a * str2.size());
    if (str2.empty()) return (d * str1.size());

    const size_t len1 = str1.size();
    const size_t len2 = str2.size();

    STACKLOCAL_POD_ARRAY(u32, rows_alloc, (len2 + 1) * 3);

    TMemoryView<u32> row0 = rows_alloc.SubRange(0 * (len2 + 1), len2 + 1);
    TMemoryView<u32> row1 = rows_alloc.SubRange(1 * (len2 + 1), len2 + 1);
    TMemoryView<u32> row2 = rows_alloc.SubRange(2 * (len2 + 1), len2 + 1);

    forrange(j, 0, u32(len2 + 1)) row1[j] = j * a;

    forrange(i, 0, u32(len1)) {
        row2[0] = (i + 1) * d;

        forrange(j, 0, len2) {
            /* substitution */
            row2[j + 1] = row1[j] + (equalto()(str1[i], str2[j]) ? 0 : s);
            /* swap */
            if (i > 0 && j > 0 && equalto()(str1[i - 1], str2[j]) && equalto()(str1[i], str2[j - 1]) && row2[j + 1] > row0[j - 1] + w)
                row2[j + 1] = row0[j - 1] + w;
            /* deletion */
            if (row2[j + 1] > row1[j + 1] + d)
                row2[j + 1] = row1[j + 1] + d;
            /* insertion */
            if (row2[j + 1] > row2[j] + a)
                row2[j + 1] = row2[j] + a;
        }

        const auto dummy = row0;
        row0 = row1;
        row1 = row2;
        row2 = dummy;
    }

    return row1[len2];
}
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sentitive>
static size_t LevenshteinDistance_(const TBasicStringView<_Char>& str1, const TBasicStringView<_Char>& str2) {
    return LevenshteinDistance_<_Char, _Sentitive>(str1, str2, 0, 2, 1, 3); //taken from https://github.com/gitster/git/blob/master/help.c
}
//----------------------------------------------------------------------------
template <typename _Char>
static bool IsAll_(const TBasicStringView<_Char>& str, bool (*pred)(_Char)) {
    return (str.end() == str.FindIfNot(pred));
}
//----------------------------------------------------------------------------
template <typename _Char>
static bool Has_(const TBasicStringView<_Char>& str, bool (*pred)(_Char)) {
    return (str.end() != str.FindIf(pred));
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicStringView<_Char> SplitIf_(const TBasicStringView<_Char>& str, bool (*pred)(_Char)) {
    return str.SplitIf(pred);
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicStringView<_Char> SplitInplaceIf_ReturnEaten_(TBasicStringView<_Char>& str, bool (*pred)(_Char ch)) {
    const auto it = str.FindIfNot(std::move(pred));
    const TBasicStringView<_Char> eaten = str.CutBefore(it);
    str = str.CutStartingAt(it);
    return eaten;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Pred>
static TBasicStringView<_Char> SplitInplaceIf_ReturnEaten_(TBasicStringView<_Char>& str, _Pred&& pred) {
    const auto it = str.FindIfNot(std::move(pred));
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
    Assert(not firstOccurence.empty());
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
template <typename _Char>
typename TBasicStringView<_Char>::iterator StrStrI_(const TBasicStringView<_Char>& str, const TBasicStringView<_Char>& firstOccurence) {
    Assert(not firstOccurence.empty());
    if (firstOccurence.size() <= str.size()) {
        forrange(i, 0, str.size() - firstOccurence.size() + 1) {
            if (EqualsI(str.SubRange(i, firstOccurence.size()), firstOccurence))
                return str.begin() + i;
        }
    }
    return str.end();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// 6x-8x faster with SIMD :
//----------------------------------------------------------------------------
void ToLower(const TMemoryView<char>& dst, const TBasicStringView<char>& src) NOEXCEPT {
    Assert(dst.size() == src.size());
    if (not dst.empty())
        FPlatformString::ToLower(dst.data(), src.data(), src.size());
}
//----------------------------------------------------------------------------
void ToLower(const TMemoryView<wchar_t>& dst, const TBasicStringView<wchar_t>& src) NOEXCEPT {
    Assert(dst.size() == src.size());
    if (not dst.empty())
        FPlatformString::ToLower(dst.data(), src.data(), src.size());
}
//----------------------------------------------------------------------------
void ToUpper(const TMemoryView<char>& dst, const TBasicStringView<char>& src) NOEXCEPT {
    Assert(dst.size() == src.size());
    if (not dst.empty())
        FPlatformString::ToUpper(dst.data(), src.data(), src.size());
}
//----------------------------------------------------------------------------
void ToUpper(const TMemoryView<wchar_t>& dst, const TBasicStringView<wchar_t>& src) NOEXCEPT {
    Assert(dst.size() == src.size());
    if (not dst.empty())
        FPlatformString::ToUpper(dst.data(), src.data(), src.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView::iterator CONSTF StrChr(const FStringView& str, char ch) NOEXCEPT { return StrChr_(str, ch); }
FStringView::reverse_iterator CONSTF StrRChr(const FStringView& str, char ch) NOEXCEPT { return StrRChr_(str, ch); }
//----------------------------------------------------------------------------
FWStringView::iterator CONSTF StrChr(const FWStringView& wstr, wchar_t wch) NOEXCEPT { return StrChr_(wstr, wch); }
FWStringView::reverse_iterator CONSTF StrRChr(const FWStringView& wstr, wchar_t wch) NOEXCEPT { return StrRChr_(wstr, wch); }
//----------------------------------------------------------------------------
FStringView::iterator CONSTF StrStr(const FStringView& str, const FStringView& firstOccurence) NOEXCEPT { return StrStr_(str, firstOccurence); }
FWStringView::iterator CONSTF StrStr(const FWStringView& wstr, const FWStringView& firstOccurence) NOEXCEPT { return StrStr_(wstr, firstOccurence); }
//----------------------------------------------------------------------------
FStringView::iterator CONSTF StrStrI(const FStringView& str, const FStringView& firstOccurence) NOEXCEPT { return StrStrI_(str, firstOccurence); }
FWStringView::iterator CONSTF StrStrI(const FWStringView& wstr, const FWStringView& firstOccurence) NOEXCEPT { return StrStrI_(wstr, firstOccurence); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool CONSTF IsAlnum(const FStringView& str) NOEXCEPT { return IsAll_(str, &IsAlnum); }
bool CONSTF IsAlnum(const FWStringView& wstr) NOEXCEPT { return IsAll_(wstr, &IsAlnum); }
//----------------------------------------------------------------------------
bool CONSTF IsAlpha(const FStringView& str) NOEXCEPT { return IsAll_(str, &IsAlpha); }
bool CONSTF IsAlpha(const FWStringView& wstr) NOEXCEPT { return IsAll_(wstr, &IsAlpha); }
//----------------------------------------------------------------------------
bool CONSTF IsDigit(const FStringView& str) NOEXCEPT { return IsAll_(str, &IsDigit); }
bool CONSTF IsDigit(const FWStringView& wstr) NOEXCEPT { return IsAll_(wstr, &IsDigit); }
//----------------------------------------------------------------------------
bool CONSTF IsXDigit(const FStringView& str) NOEXCEPT { return IsAll_(str, &IsXDigit); }
bool CONSTF IsXDigit(const FWStringView& wstr) NOEXCEPT { return IsAll_(wstr, &IsXDigit); }
//----------------------------------------------------------------------------
bool CONSTF IsIdentifier(const FStringView& str) NOEXCEPT { return IsAll_(str, &IsIdentifier); }
bool CONSTF IsIdentifier(const FWStringView& wstr) NOEXCEPT { return IsAll_(wstr, &IsIdentifier); }
//----------------------------------------------------------------------------
bool CONSTF IsOctal(const FStringView& str) NOEXCEPT { return IsAll_(str, &IsOctal); }
bool CONSTF IsOctal(const FWStringView& wstr) NOEXCEPT { return IsAll_(wstr, &IsOctal); }
//----------------------------------------------------------------------------
bool CONSTF IsPrint(const FStringView& str) NOEXCEPT { return IsAll_(str, &IsPrint); }
bool CONSTF IsPrint(const FWStringView& wstr) NOEXCEPT { return IsAll_(wstr, &IsPrint); }
//----------------------------------------------------------------------------
bool CONSTF IsSpace(const FStringView& str) NOEXCEPT { return IsAll_(str, &IsSpace); }
bool CONSTF IsSpace(const FWStringView& wstr) NOEXCEPT { return IsAll_(wstr, &IsSpace); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool CONSTF HasAlnum(const FStringView& str) NOEXCEPT { return Has_(str, &IsAlnum); }
bool CONSTF HasAlnum(const FWStringView& wstr) NOEXCEPT { return Has_(wstr, &IsAlnum); }
//----------------------------------------------------------------------------
bool CONSTF HasAlpha(const FStringView& str) NOEXCEPT { return Has_(str, &IsAlpha); }
bool CONSTF HasAlpha(const FWStringView& wstr) NOEXCEPT { return Has_(wstr, &IsAlpha); }
//----------------------------------------------------------------------------
bool CONSTF HasDigit(const FStringView& str) NOEXCEPT { return Has_(str, &IsDigit); }
bool CONSTF HasDigit(const FWStringView& wstr) NOEXCEPT { return Has_(wstr, &IsDigit); }
//----------------------------------------------------------------------------
bool CONSTF HasXDigit(const FStringView& str) NOEXCEPT { return Has_(str, &IsXDigit); }
bool CONSTF HasXDigit(const FWStringView& wstr) NOEXCEPT { return Has_(wstr, &IsXDigit); }
//----------------------------------------------------------------------------
bool CONSTF HasPrint(const FStringView& str) NOEXCEPT { return Has_(str, &IsPrint); }
bool CONSTF HasPrint(const FWStringView& wstr) NOEXCEPT { return Has_(wstr, &IsPrint); }
//----------------------------------------------------------------------------
bool CONSTF HasSpace(const FStringView& str) NOEXCEPT { return Has_(str, &IsSpace); }
bool CONSTF HasSpace(const FWStringView& wstr) NOEXCEPT { return Has_(wstr, &IsSpace); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView EatAlnums(FStringView& str) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, &IsAlnum); }
FWStringView EatAlnums(FWStringView& wstr) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, &IsAlnum); }
//----------------------------------------------------------------------------
FStringView EatAlphas(FStringView& str) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, &IsAlpha); }
FWStringView EatAlphas(FWStringView& wstr) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, &IsAlpha); }
//----------------------------------------------------------------------------
FStringView EatDigits(FStringView& str) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, &IsDigit); }
FWStringView EatDigits(FWStringView& wstr) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, &IsDigit); }
//----------------------------------------------------------------------------
FStringView EatXDigits(FStringView& str) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, &IsXDigit); }
FWStringView EatXDigits(FWStringView& wstr) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, &IsXDigit); }
//----------------------------------------------------------------------------
FStringView EatIdentifier(FStringView& str) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, &IsIdentifier); }
FWStringView EatIdentifier(FWStringView& wstr) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, &IsIdentifier); }
//----------------------------------------------------------------------------
FStringView EatOctals(FStringView& str) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, &IsOctal); }
FWStringView EatOctals(FWStringView& wstr) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, &IsOctal); }
//----------------------------------------------------------------------------
FStringView EatPrints(FStringView& str) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, &IsPrint); }
FWStringView EatPrints(FWStringView& wstr) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, &IsPrint); }
//----------------------------------------------------------------------------
FStringView EatSpaces(FStringView& str) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, &IsSpace); }
FWStringView EatSpaces(FWStringView& wstr) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, &IsSpace); }
//----------------------------------------------------------------------------
FStringView EatWhile(FStringView& str, char ch) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, [ch](char x) { return (x == ch); }); }
FWStringView EatWhile(FWStringView& wstr, wchar_t wch) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, [wch](wchar_t x) { return (x == wch); }); }
//----------------------------------------------------------------------------
FStringView EatWhile(FStringView& str, const FStringView& multiple) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, [multiple](char x) { return (multiple.Contains(x)); }); }
FWStringView EatWhile(FWStringView& wstr, const FWStringView& wmultiple) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, [wmultiple](wchar_t x) { return (wmultiple.Contains(x)); }); }
//----------------------------------------------------------------------------
FStringView EatWhile(FStringView& str, bool (*is_a)(char)) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, is_a); }
FWStringView EatWhile(FWStringView& wstr, bool (*is_a)(wchar_t)) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, is_a); }
//----------------------------------------------------------------------------
FStringView EatUntil(FStringView& str, char ch) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, [ch](char x) { return (x != ch); }); }
FWStringView EatUntil(FWStringView& wstr, wchar_t wch) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, [wch](wchar_t x) { return (x != wch); }); }
//----------------------------------------------------------------------------
FStringView EatUntil(FStringView& str, const FStringView& multiple) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, [multiple](char x) { return (not multiple.Contains(x)); }); }
FWStringView EatUntil(FWStringView& wstr, const FWStringView& wmultiple) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, [wmultiple](wchar_t x) { return (not wmultiple.Contains(x)); }); }
//----------------------------------------------------------------------------
FStringView EatUntil(FStringView& str, bool (*is_not_a)(char)) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(str, is_not_a); }
FWStringView EatUntil(FWStringView& wstr, bool (*is_not_a)(wchar_t)) NOEXCEPT { return SplitInplaceIf_ReturnEaten_(wstr, is_not_a); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicStringView<_Char> TrimStart_(const TBasicStringView<_Char>& str, _Char ch) {
    size_t begin = 0;
    for (; begin < str.size() && str[begin] == ch; ++begin);
    return str.CutStartingAt(begin);
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicStringView<_Char> TrimStart_(const TBasicStringView<_Char>& str, const TBasicStringView<_Char>& chars) {
    Assert(not chars.empty());
    size_t begin = 0;
    for (; begin < str.size() && chars.Contains(str[begin]); ++begin);
    return str.CutStartingAt(begin);
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicStringView<_Char> TrimEnd_(const TBasicStringView<_Char>& str, _Char ch) {
    size_t end = str.size();
    for (; end > 0 && str[end - 1] == ch; --end);
    return str.CutBefore(end);
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicStringView<_Char> TrimEnd_(const TBasicStringView<_Char>& str, const TBasicStringView<_Char>& chars) {
    Assert(not chars.empty());
    size_t end = str.size();
    for (; end > 0 && chars.Contains(str[end - 1]); --end);
    return str.CutBefore(end);
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicStringView<_Char> Trim_(const TBasicStringView<_Char>& str, _Char ch) {
    return TrimEnd_(TrimStart_(str, ch), ch);
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicStringView<_Char> Trim_(const TBasicStringView<_Char>& str, const TBasicStringView<_Char>& chars) {
    return TrimEnd_(TrimStart_(str, chars), chars);
}
//----------------------------------------------------------------------------
FStringView TrimStart(const FStringView& str, char ch) NOEXCEPT { return TrimStart_(str, ch); }
FWStringView TrimStart(const FWStringView& wstr, wchar_t wch) NOEXCEPT { return TrimStart_(wstr, wch); }
//----------------------------------------------------------------------------
FStringView TrimStart(const FStringView& str, const FStringView& chars) NOEXCEPT { return TrimStart_(str, chars); }
FWStringView TrimStart(const FWStringView& wstr, const FWStringView& wchars) NOEXCEPT { return TrimStart_(wstr, wchars); }
//----------------------------------------------------------------------------
FStringView TrimEnd(const FStringView& str, char ch) NOEXCEPT { return TrimEnd_(str, ch); }
FWStringView TrimEnd(const FWStringView& wstr, wchar_t wch) NOEXCEPT { return TrimEnd_(wstr, wch); }
//----------------------------------------------------------------------------
FStringView TrimEnd(const FStringView& str, const FStringView& chars) NOEXCEPT { return TrimEnd_(str, chars); }
FWStringView TrimEnd(const FWStringView& wstr, const FWStringView& wchars) NOEXCEPT { return TrimEnd_(wstr, wchars); }
//----------------------------------------------------------------------------
FStringView Trim(const FStringView& str, char ch) NOEXCEPT { return Trim_(str, ch); }
FWStringView Trim(const FWStringView& wstr, wchar_t wch) NOEXCEPT { return Trim_(wstr, wch); }
//----------------------------------------------------------------------------
FStringView Trim(const FStringView& str, const FStringView& chars) NOEXCEPT { return Trim_(str, chars); }
FWStringView Trim(const FWStringView& wstr, const FWStringView& wchars) NOEXCEPT { return Trim_(wstr, wchars); }
//----------------------------------------------------------------------------
FStringView Chomp(const FStringView& str) NOEXCEPT { return TrimEnd_(str, MakeStringView("\n\r")); }
FWStringView Chomp(const FWStringView& wstr) NOEXCEPT { return TrimEnd_(wstr, MakeStringView(L"\n\r")); }
//----------------------------------------------------------------------------
FStringView Strip(const FStringView& str) NOEXCEPT { return Trim_(str, MakeStringView(" \t\r")); }
FWStringView Strip(const FWStringView& wstr) NOEXCEPT { return Trim_(wstr, MakeStringView(L" \t\r")); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Cmp>
FORCE_INLINE static int Compare_(
    const TBasicStringView<_Char>& lhs,
    const TBasicStringView<_Char>& rhs,
    _Cmp compare ) NOEXCEPT {
    if (lhs == rhs)
        return 0;

    int cmp;

    const size_t sz = Min(lhs.size(), rhs.size());
    if (0 == sz)
        goto CMP_SIZE;

    cmp = compare(lhs.data(), rhs.data(), sz);
    if (cmp)
        return cmp;

CMP_SIZE:
    if (lhs.size() == rhs.size())
        return 0;
    else
        return (lhs.size() < rhs.size() ? -1 : 1);
}
//----------------------------------------------------------------------------
template <typename _Char>
FORCE_INLINE static int Compare_(
    const TBasicStringView<_Char>& lhs,
    const TBasicStringView<_Char>& rhs) {
    return Compare_(lhs, rhs, [](const _Char* a, const _Char* b, size_t n) NOEXCEPT {
        return FPlatformString::NCmp(a, b, n);
    });
}
//----------------------------------------------------------------------------
template <typename _Char>
FORCE_INLINE static int CompareI_(
    const TBasicStringView<_Char>& lhs,
    const TBasicStringView<_Char>& rhs) {
    return Compare_(lhs, rhs, [](const _Char* a, const _Char* b, size_t n) NOEXCEPT {
        return FPlatformString::NCmpI(a, b, n);
    });
}
//----------------------------------------------------------------------------
int CONSTF Compare(const FStringView& lhs, const FStringView& rhs) NOEXCEPT {
    return Compare_(lhs, rhs);
}
//----------------------------------------------------------------------------
int CONSTF Compare(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT {
    return Compare_(lhs, rhs);
}
//----------------------------------------------------------------------------
int CONSTF CompareI(const FStringView& lhs, const FStringView& rhs) NOEXCEPT {
    return CompareI_(lhs, rhs);
}
//----------------------------------------------------------------------------
int CONSTF CompareI(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT {
    return CompareI_(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool CONSTF EqualsN(const char* lhs, const char* rhs, size_t len) NOEXCEPT {
    return (lhs == rhs || 0 == len || FPlatformString::Equals(lhs, rhs, len));
}
//----------------------------------------------------------------------------
bool CONSTF EqualsNI(const char* lhs, const char* rhs, size_t len) NOEXCEPT {
    return (lhs == rhs || 0 == len || FPlatformString::EqualsI(lhs, rhs, len));
}
//----------------------------------------------------------------------------
bool CONSTF EqualsN(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT {
    return (lhs == rhs || 0 == len || FPlatformString::Equals(lhs, rhs, len));
}
//----------------------------------------------------------------------------
bool CONSTF EqualsNI(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT {
    return (lhs == rhs || 0 == len || FPlatformString::EqualsI(lhs, rhs, len));
}
//----------------------------------------------------------------------------
bool CONSTF Equals(const FStringView& lhs, const FStringView& rhs) NOEXCEPT {
    return ((lhs.size() == rhs.size()) &&
            (lhs.size() == 0 || lhs.data() == rhs.data() || FPlatformString::Equals(lhs.data(), rhs.data(), lhs.size())) );
}
//----------------------------------------------------------------------------
bool CONSTF Equals(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT {
    return ((lhs.size() == rhs.size()) &&
            (lhs.size() == 0 || lhs.data() == rhs.data() || FPlatformString::Equals(lhs.data(), rhs.data(), lhs.size())) );
}
//----------------------------------------------------------------------------
bool CONSTF EqualsI(const FStringView& lhs, const FStringView& rhs) NOEXCEPT {
    return ((lhs.size() == rhs.size()) &&
            (lhs.size() == 0 || lhs.data() == rhs.data() || FPlatformString::EqualsI(lhs.data(), rhs.data(), lhs.size())) );
}
//----------------------------------------------------------------------------
bool CONSTF EqualsI(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT {
    return ((lhs.size() == rhs.size()) &&
            (lhs.size() == 0 || lhs.data() == rhs.data() || FPlatformString::EqualsI(lhs.data(), rhs.data(), lhs.size())) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool CONSTF StartsWith(const FStringView& str, const FStringView& prefix) NOEXCEPT {
    return StartsWith_(str, prefix);
}
//----------------------------------------------------------------------------
bool CONSTF StartsWith(const FWStringView& wstr, const FWStringView& wprefix) NOEXCEPT {
    return StartsWith_(wstr, wprefix);
}
//----------------------------------------------------------------------------
bool CONSTF StartsWithI(const FStringView& str, const FStringView& prefix) NOEXCEPT {
    return StartsWithI_(str, prefix);
}
//----------------------------------------------------------------------------
bool CONSTF StartsWithI(const FWStringView& wstr, const FWStringView& wprefix) NOEXCEPT {
    return StartsWithI_(wstr, wprefix);
}
//----------------------------------------------------------------------------
bool CONSTF EndsWith(const FStringView& str, const FStringView& suffix) NOEXCEPT {
    return EndsWith_(str, suffix);
}
//----------------------------------------------------------------------------
bool CONSTF EndsWith(const FWStringView& wstr, const FWStringView& wsuffix) NOEXCEPT {
    return EndsWith_(wstr, wsuffix);
}
//----------------------------------------------------------------------------
bool CONSTF EndsWithI(const FStringView& str, const FStringView& suffix) NOEXCEPT {
    return EndsWithI_(str, suffix);
}
//----------------------------------------------------------------------------
bool CONSTF EndsWithI(const FWStringView& wstr, const FWStringView& wsuffix) NOEXCEPT {
    return EndsWithI_(wstr, wsuffix);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Split(FStringView& str, char separator, FStringView& slice) NOEXCEPT {
    return Split_<char, true>(str, separator, slice);
}
//----------------------------------------------------------------------------
bool Split(FWStringView& wstr, wchar_t separator, FWStringView& slice) NOEXCEPT {
    return Split_<wchar_t, true>(wstr, separator, slice);
}
//----------------------------------------------------------------------------
bool Split(FStringView& str, const FStringView& separators, FStringView& slice) NOEXCEPT {
    return SplitMulti_<char, true>(str, separators, slice);
}
//----------------------------------------------------------------------------
bool Split(FWStringView& wstr, const FWStringView& separators, FWStringView& slice) NOEXCEPT {
    return SplitMulti_<wchar_t, true>(wstr, separators, slice);
}
//----------------------------------------------------------------------------
bool SplitR(FStringView& str, char separator, FStringView& slice) NOEXCEPT {
    return Split_<char, false>(str, separator, slice);
}
//----------------------------------------------------------------------------
bool SplitR(FWStringView& wstr, wchar_t separator, FWStringView& slice) NOEXCEPT {
    return Split_<wchar_t, false>(wstr, separator, slice);
}
//----------------------------------------------------------------------------
bool SplitR(FStringView& str, const FStringView& separators, FStringView& slice) NOEXCEPT {
    return SplitMulti_<char, false>(str, separators, slice);
}
//----------------------------------------------------------------------------
bool SplitR(FWStringView& wstr, const FWStringView& separators, FWStringView& slice) NOEXCEPT {
    return SplitMulti_<wchar_t, false>(wstr, separators, slice);
}
//----------------------------------------------------------------------------
bool SplitNth(FStringView& str, char separator, FStringView& slice, size_t nth) NOEXCEPT {
    return SplitNth_<char, true>(str, separator, slice, nth);
}
//----------------------------------------------------------------------------
bool SplitNth(FWStringView& wstr, wchar_t separator, FWStringView& slice, size_t nth) NOEXCEPT {
    return SplitNth_<wchar_t, true>(wstr, separator, slice, nth);
}
//----------------------------------------------------------------------------
bool SplitNth(FStringView& str, const FStringView& separators, FStringView& slice, size_t nth) NOEXCEPT {
    return SplitNth_<char, false>(str, separators, slice, nth);
}
//----------------------------------------------------------------------------
bool SplitNth(FWStringView& wstr, const FWStringView& separators, FWStringView& slice, size_t nth) NOEXCEPT {
    return SplitNth_<wchar_t, false>(wstr, separators, slice, nth);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Atoi(i32* dst, const FStringView& str, size_t base) NOEXCEPT {
    return Atoi_(dst, str, base);
}
//----------------------------------------------------------------------------
bool Atoi(u32* dst, const FStringView& str, size_t base) NOEXCEPT {
    return Atoi_(dst, str, base);
}
//----------------------------------------------------------------------------
bool Atoi(i64* dst, const FStringView& str, size_t base) NOEXCEPT {
    return Atoi_(dst, str, base);
}
//----------------------------------------------------------------------------
bool Atoi(u64* dst, const FStringView& str, size_t base) NOEXCEPT {
    return Atoi_(dst, str, base);
}
//----------------------------------------------------------------------------
bool Atoi(i32* dst, const FWStringView& wstr, size_t base) NOEXCEPT {
    return Atoi_(dst, wstr, base);
}
//----------------------------------------------------------------------------
bool Atoi(u32* dst, const FWStringView& wstr, size_t base) NOEXCEPT {
    return Atoi_(dst, wstr, base);
}
//----------------------------------------------------------------------------
bool Atoi(i64* dst, const FWStringView& wstr, size_t base) NOEXCEPT {
    return Atoi_(dst, wstr, base);
}
//----------------------------------------------------------------------------
bool Atoi(u64* dst, const FWStringView& wstr, size_t base) NOEXCEPT {
    return Atoi_(dst, wstr, base);
}
//----------------------------------------------------------------------------
bool Atof(float* dst, const FStringView& str) NOEXCEPT {
    return Atof_(dst, str);
}
//----------------------------------------------------------------------------
bool Atod(double* dst, const FStringView& str) NOEXCEPT {
    return Atof_(dst, str);
}
//----------------------------------------------------------------------------
bool Atof(float* dst, const FWStringView& wstr) NOEXCEPT {
    return Atof_(dst, wstr);
}
//----------------------------------------------------------------------------
bool Atod(double* dst, const FWStringView& wstr) NOEXCEPT {
    return Atof_(dst, wstr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool WildMatch(const FStringView& pattern, const FStringView& str) NOEXCEPT {
    return WildMatch_<ECase::Sensitive, char>(pattern, str);
}
//----------------------------------------------------------------------------
bool WildMatch(const FWStringView& pattern, const FWStringView& wstr) NOEXCEPT {
    return WildMatch_<ECase::Sensitive, wchar_t>(pattern, wstr);
}
//----------------------------------------------------------------------------
bool WildMatchI(const FStringView& pattern, const FStringView& str) NOEXCEPT {
    return WildMatch_<ECase::Insensitive, char>(pattern, str);
}
//----------------------------------------------------------------------------
bool WildMatchI(const FWStringView& pattern, const FWStringView& wstr) NOEXCEPT {
    return WildMatch_<ECase::Insensitive, wchar_t>(pattern, wstr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t LevenshteinDistance(const FStringView& lhs, const FStringView& rhs) NOEXCEPT {
    return LevenshteinDistance_<char, ECase::Sensitive>(lhs, rhs);
}
//----------------------------------------------------------------------------
size_t LevenshteinDistance(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT {
    return LevenshteinDistance_<wchar_t, ECase::Sensitive>(lhs, rhs);
}
//----------------------------------------------------------------------------
size_t LevenshteinDistanceI(const FStringView& lhs, const FStringView& rhs) NOEXCEPT {
    return LevenshteinDistance_<char, ECase::Insensitive>(lhs, rhs);
}
//----------------------------------------------------------------------------
size_t LevenshteinDistanceI(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT {
    return LevenshteinDistance_<wchar_t, ECase::Insensitive>(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t Copy(const TMemoryView<char>& dst, const FStringView& src) {
    const size_t n = Min(dst.size(), src.size());
    std::copy(src.begin(), src.begin() + n, dst.begin());
    return n;
}
//----------------------------------------------------------------------------
size_t Copy(const TMemoryView<wchar_t>& dst, const FWStringView& src) {
    const size_t n = Min(dst.size(), src.size());
    std::copy(src.begin(), src.begin() + n, dst.begin());
    return n;
}
//----------------------------------------------------------------------------
const char* NullTerminated(const TMemoryView<char>& dst, const FStringView& src) {
    const size_t n = Copy(dst, src);
    dst[n] = '\0';
    return dst.data();
}
//----------------------------------------------------------------------------
const wchar_t* NullTerminated(const TMemoryView<wchar_t>& dst, const FWStringView& src) {
    const size_t n = Copy(dst, src);
    dst[n] = L'\0';
    return dst.data();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_string(const FStringView& str) NOEXCEPT {
    return hash_mem(str.data(), str.SizeInBytes(), hash_value(str.size()));
}
//----------------------------------------------------------------------------
hash_t hash_string(const FWStringView& wstr) NOEXCEPT {
    return hash_mem(wstr.data(), wstr.SizeInBytes(), hash_value(wstr.size()));
}
//----------------------------------------------------------------------------
hash_t hash_stringI(const FStringView& str) NOEXCEPT {
#if 0 // don't want to use another hash function :/
    char (*transform)(char) = &ToLower;
    return hash_fnv1a(MakeOutputIterator(str.begin(), transform), MakeOutputIterator(str.end(), transform));
#else
    // NOPE ---> refactor to skip copy, avoid using case insensitive ?
    // SEEMS TO BE FASTER TO COPY : https://github.com/Cyan4973/xxHash/issues/124
    const size_t sz = str.size();
    MALLOCA_POD(char, istr, sz);
    ToLower(istr.MakeView(), str);
    return hash_mem(istr.data(), sz, hash_value(sz));
#endif
}
//----------------------------------------------------------------------------
hash_t hash_stringI(const FWStringView& wstr) NOEXCEPT {
#if 0 // don't want to use another hash function :/
    wchar_t (*transform)(wchar_t) = &ToLower;
    return hash_fnv1a(MakeOutputIterator(wstr.begin(), transform), MakeOutputIterator(wstr.end(), transform));
#else
    // NOPE ---> refactor to skip copy, avoid using case insensitive ?
    // SEEMS TO BE FASTER TO COPY : https://github.com/Cyan4973/xxHash/issues/124
    const size_t sz = wstr.size();
    MALLOCA_POD(wchar_t, iwstr, sz);
    ToLower(iwstr, wstr);
    return hash_mem(iwstr.data(), iwstr.SizeInBytes(), hash_value(sz));
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Escape(FTextWriter& oss, const FStringView& str, EEscape escape) {
    const FTextFormat fmt = oss.ResetFormat();

    for (char ch : str) {
        switch (ch) {
        case '\\':
        case '"':
            oss.Put('\\');
            oss.Put(ch);
            break;
        case '\b':
            oss.Put("\\b");
            break;
        case '\t':
            oss.Put("\\t");
            break;
        case '\n':
            oss.Put("\\n");
            break;
        case '\f':
            oss.Put("\\f");
            break;
        case '\r':
            oss.Put("\\r");
            break;
        default:
            if (IsPrint(ch))
                oss.Put(ch);
            else {
                const size_t ord = (size_t(ch) & 0xFF);
                switch (escape) {
                case PPE::EEscape::Octal:
                    oss.Put('\\');
                    oss << FTextFormat::Octal << ord;
                    break;
                case PPE::EEscape::Hexadecimal:
                    oss.Put("\\x");
                    oss << FTextFormat::Hexadecimal << FTextFormat::PadLeft(2, '0') << ord;
                    break;
                case PPE::EEscape::Unicode:
                    oss.Put("\\u00");
                    oss << FTextFormat::Hexadecimal << FTextFormat::PadLeft(2, '0') << ord;
                    break;
                default:
                    AssertNotImplemented();
                }
            }
        }
    }

    oss.SetFormat(fmt);
}
//----------------------------------------------------------------------------
void Escape(FTextWriter& oss, const FWStringView& wstr, EEscape escape) {
    Assert(EEscape::Unicode == escape);
    const FTextFormat fmt = oss.ResetFormat();

    for (wchar_t wch : wstr) {
        const size_t ord = (size_t(wch) & 0xFFFF);
        oss.Put("\\u");
        oss << FTextFormat::Hexadecimal << FTextFormat::PadLeft(4, '0') << ord;
    }

    oss.SetFormat(fmt);
}
//----------------------------------------------------------------------------
void Escape(FWTextWriter& oss, const FWStringView& wstr, EEscape escape) {
    const FTextFormat fmt = oss.ResetFormat();

    for (wchar_t wch : wstr) {
        switch (wch) {
        case L'\\':
        case L'"':
            oss.Put(L'\\');
            oss.Put(wch);
            break;
        case '\b':
            oss.Put(L"\\b");
            break;
        case '\t':
            oss.Put(L"\\t");
            break;
        case '\n':
            oss.Put(L"\\n");
            break;
        case '\f':
            oss.Put(L"\\f");
            break;
        case '\r':
            oss.Put(L"\\r");
            break;
        default:
            if (IsPrint(wch))
                oss.Put(wch);
            else {
                const size_t ord = (size_t(wch) & 0xFFFF);
                switch (escape) {
                case PPE::EEscape::Octal:
                    oss.Put(L'\\');
                    oss << FTextFormat::Octal << ord;
                    break;
                case PPE::EEscape::Hexadecimal:
                    if (ord <= 0xFF) {
						oss.Put(L"\\x");
						oss << FTextFormat::Hexadecimal << FTextFormat::PadLeft(2, L'0') << ord;
                    }
                    else {
                        oss.Put(L"\\x");
                        oss << FTextFormat::Hexadecimal << FTextFormat::PadLeft(2, L'0') << ((ord >> 8) & 0xFF);
                        oss.Put(L"\\x");
                        oss << FTextFormat::Hexadecimal << FTextFormat::PadLeft(2, L'0') << (ord & 0xFF);
                    }
                    break;
                case PPE::EEscape::Unicode:
                    oss.Put(L"\\u");
                    oss << FTextFormat::Hexadecimal << FTextFormat::PadLeft(4, L'0') << ord;
                    break;
                default:
                    AssertNotImplemented();
                }
            }
        }
    }

    oss.SetFormat(fmt);
}
//----------------------------------------------------------------------------
void Escape(FWTextWriter& oss, const FStringView& str, EEscape escape) {
    const FTextFormat fmt = oss.ResetFormat();

    for (char ch : str) {
        const size_t ord = (size_t(ch) & 0xFF);
        switch (escape) {
        case PPE::EEscape::Octal:
            oss.Put(L'\\');
            oss << FTextFormat::Octal << ord;
            break;
        case PPE::EEscape::Hexadecimal:
            oss.Put(L"\\x");
            oss << FTextFormat::Hexadecimal << FTextFormat::PadLeft(2, L'0') << ord;
            break;
        case PPE::EEscape::Unicode:
            oss.Put(L"\\u00");
            oss << FTextFormat::Hexadecimal << FTextFormat::PadLeft(4, L'0') << ord;
            break;
        default:
            AssertNotImplemented();
        }
    }

    oss.SetFormat(fmt);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _DstChar, typename _SrcChar>
static size_t Unescape_Hexadecimal_(TBasicTextWriter<_DstChar>& oss, TBasicStringView<_SrcChar>&& str) {
    const TBasicStringView<_SrcChar> xdigits = EatXDigits(str);
    if (xdigits.size()) {// don't have character limit, as per cpp standard
        u32 ch;
        if (Atoi(&ch, xdigits, 16)) {
            oss.Put(static_cast<_DstChar>(ch));
            return (xdigits.size() + 1/* leading 'x'/'u' */);
        }
    }
    return 0; // this is undefined behavior in cpp standard
}
//----------------------------------------------------------------------------
template <typename _DstChar, typename _SrcChar>
static size_t Unescape_Octal_(TBasicTextWriter<_DstChar>& oss, TBasicStringView<_SrcChar>&& str) {
    TBasicStringView<_SrcChar> octal = EatOctals(str);
    if (octal.size()) {
        if (octal.size() > 3)// limit to 3 chars, as per cpp standard
            octal = octal.CutBefore(3);

        u32 ch;
        if (Atoi(&ch, octal, 8)) {
            oss.Put(static_cast<_DstChar>(ch));
            return octal.size();
        }
    }
    return 0; // this is undefined behavior in cpp standard
}
//----------------------------------------------------------------------------
template <typename _DstChar, typename _SrcChar>
static void Unescape_(TBasicTextWriter<_DstChar>& oss, const TBasicStringView<_SrcChar>& str) {
#define STR(X) STRING_LITERAL(_DstChar, X)
    for (size_t i = 0; i < str.size(); ++i) {
        if ((str[i] == STR('\\')) && (i + 1 < str.size())) {
            size_t eaten = 1;
            switch (str[i + 1]) {
            // special characters
            case STR('t'):case STR('T'):
                oss.Put(STR('\t'));
                break;
            case STR('r'):case STR('R'):
                oss.Put(STR('\r'));
                break;
            case STR('n'):case STR('N'):
                oss.Put(STR('\n'));
                break;
            case STR('\\'):
            case STR('"'):
                oss.Put(str[i + 1]);
                break;
            case STR('b'):case STR('B'):
                oss.Put(STR('\b'));
                break;
            case STR('f'):case STR('F'):
                oss.Put(STR('\f'));
                break;
            // hexadecimal + unicode
            case STR('x'):case STR('X'):
            case STR('u'):case STR('U'):
                eaten = Unescape_Hexadecimal_<_DstChar, _SrcChar>(oss, str.CutStartingAt(i + 2));
                break;
            // octal
            case STR('0'):case STR('1'):case STR('2'):case STR('3'):
            case STR('4'):case STR('5'):case STR('6'):case STR('7'):
                eaten = Unescape_Octal_<_DstChar, _SrcChar>(oss, str.CutStartingAt(i + 1));
                break;
            // unknown escape sequence
            default:
                eaten = 0;// don't escape the sequence
                break;
            }

            if (Likely(eaten))
                i += eaten;
            else
                oss.Put('\\'); // print escaping '\' to hint at error in the result
        }
        else {
            oss.Put(str[i]);
        }
    }
#undef STR
}
//----------------------------------------------------------------------------
void Unescape(FTextWriter& oss, const FStringView& str) { Unescape_(oss, str); }
void Unescape(FWTextWriter& oss, const FWStringView& str) { Unescape_(oss, str); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FWStringView& wslice) {
    return oss << WCHAR_to_CHAR(ECodePage::ACP, INLINE_MALLOCA(char, wslice.size() * 2/* for multi bytes chars */ + 1), wslice);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FStringView& slice) {
    return oss << CHAR_to_WCHAR(ECodePage::ACP, INLINE_MALLOCA(wchar_t, slice.size() + 1), slice);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
