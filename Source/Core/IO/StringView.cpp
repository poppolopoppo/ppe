#include "stdafx.h"

#include "StringView.h"

#include "Allocator/Alloca.h"
#include "Memory/HashFunctions.h"

#include <emmintrin.h>
#include <intrin.h>
#include <ostream>

#define USE_CORE_SIMD_STRINGOPS 1 // turn to 0 to disable SIMD optimizations %_NOCOMMIT%

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
        Pos = '+',
        Neg = '-',
        Dot = '.',
        _0  = '0',
        _9  = '9',
        a   = 'a',
        f   = 'f',
        A   = 'A',
        F   = 'F',
        e   = 'e',
        E   = 'E',
    };
};
template <> struct TAtoN_traits<wchar_t> {
    enum : wchar_t {
        Pos = L'+',
        Neg = L'-',
        Dot = L'.',
        _0  = L'0',
        _9  = L'9',
        a   = L'a',
        f   = L'f',
        A   = L'A',
        F   = L'F',
        e   = L'e',
        E   = L'E',
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

        int d;
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

    const u32 len1 = checked_cast<u32>(str1.size());
    const u32 len2 = checked_cast<u32>(str2.size());

    STACKLOCAL_POD_ARRAY(u32, rows_alloc, (len2 + 1) * 3);

    TMemoryView<u32> row0 = rows_alloc.SubRange(0 * (len2 + 1), len2 + 1);
    TMemoryView<u32> row1 = rows_alloc.SubRange(1 * (len2 + 1), len2 + 1);
    TMemoryView<u32> row2 = rows_alloc.SubRange(2 * (len2 + 1), len2 + 1);

    forrange(j, 0, len2 + 1) row1[j] = j * a;

    forrange(i, 0, len1) {
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

        auto dummy = row0;
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
// returns < 0 if less (not necessarly -1), > 0 if greater (not necessarly +1) and 0 if equals
// 1x-2x faster with SIMD on x64 :
static int StrNCmp_(const char* lhs, const char* rhs, size_t len) {
#if defined(ARCH_X64) && USE_CORE_SIMD_STRINGOPS
    size_t fast = len / sizeof(__m128i);
    size_t offset = fast * sizeof(__m128i);
    size_t current_block = 0;

    const __m128i* lhs_128i = (const __m128i*)lhs;
    const __m128i* rhs_128i = (const __m128i*)rhs;

    for (; current_block < fast; ++current_block) {
        __m128i cmpsg = ::_mm_sub_epi8(
            ::_mm_lddqu_si128(lhs_128i + current_block),
            ::_mm_lddqu_si128(rhs_128i + current_block)
        );

        size_t mask = size_t(::_mm_movemask_epi8(::_mm_cmpgt_epi8(::_mm_abs_epi8(cmpsg), ::_mm_setzero_si128())));
        if (mask)
            return cmpsg.m128i_i8[Meta::tzcnt(mask)];
    }

    for (; len > offset; ++offset) {
        if (lhs[offset] ^ rhs[offset])
            return (int)((unsigned char)lhs[offset] - (unsigned char)rhs[offset]);
    }

    return 0;
#else
    return ::strncmp(lhs, rhs, len);
#endif
}
static int StrNCmp_(const wchar_t* lhs, const wchar_t* rhs, size_t len) {
#if defined(ARCH_X64) && USE_CORE_SIMD_STRINGOPS && 0 // some ops are not available for epi16
    size_t fast = (len * sizeof(wchar_t)) / sizeof(__m128i);
    size_t offset = fast * sizeof(__m128i);
    size_t current_block = 0;

    const __m128i* lhs_128i = (const __m128i*)lhs;
    const __m128i* rhs_128i = (const __m128i*)rhs;

    for (; current_block < fast; ++current_block) {
        __m128i cmpsg = ::_mm_sub_epi16(
            ::_mm_lddqu_si128(lhs_128i + current_block),
            ::_mm_lddqu_si128(rhs_128i + current_block)
        );

        size_t mask = size_t(::_mm_movemask_epi16(::_mm_cmpgt_epi16(::_mm_abs_epi16(cmpsg), ::_mm_setzero_si128())));
        if (mask)
            return cmpsg.m128i_i16[Meta::tzcnt(mask)];
    }

    for (; len > offset; ++offset) {
        if (lhs[offset] ^ rhs[offset])
            return (int)(lhs[offset] - rhs[offset]);
    }

    return 0;
#else
    return ::wcsncmp(lhs, rhs, len);
#endif
}
//----------------------------------------------------------------------------
// 2x-3x faster than ::strncmp() == 0
static bool StrEquals_(const char* lhs, const char* rhs, size_t len) {
#if USE_CORE_SIMD_STRINGOPS
    size_t fast = len / sizeof(__m128i);
    size_t offset = fast * sizeof(__m128i);
    size_t current_block = 0;

    for (; current_block < fast; ++current_block) {
        __m128i lhs_epi8 = ::_mm_lddqu_si128((const __m128i*)lhs + current_block);
        __m128i rhs_epi8 = ::_mm_lddqu_si128((const __m128i*)rhs + current_block);

        if (::_mm_movemask_epi8(::_mm_cmpeq_epi8(lhs_epi8, rhs_epi8)) != 0xFFFF)
            return false;
    }

    for (; len > offset; ++offset) {
        if (lhs[offset] ^ rhs[offset])
            return false;
    }

    return true;
#else
    return (0 == ::strncmp(lhs, rhs, len));
#endif
}
static bool StrEquals_(const wchar_t* lhs, const wchar_t* rhs, size_t len) {
#if USE_CORE_SIMD_STRINGOPS && 0 // some ops are not available for epi16
    size_t fast = (len * sizeof(wchar_t)) / sizeof(__m128i);
    size_t offset = fast * sizeof(__m128i);
    size_t current_block = 0;

    for (; current_block < fast; ++current_block) {
        __m128i lhs_epi16 = ::_mm_lddqu_si128((const __m128i*)lhs + current_block);
        __m128i rhs_epi16 = ::_mm_lddqu_si128((const __m128i*)rhs + current_block);

        if (::_mm_movemask_epi16(::_mm_cmpeq_epi16(lhs_epi16, rhs_epi16)) != 0xFF)
            return false;
    }

    for (; len > offset; ++offset) {
        if (lhs[offset] ^ rhs[offset])
            return false;
    }

    return true;
#else
    return (0 == ::wcsncmp(lhs, rhs, len));
#endif
}
//----------------------------------------------------------------------------
// 2x-3x faster than ::strncmp() == 0
static bool StrEqualsI_(const char* lhs, const char* rhs, size_t len) {
#if USE_CORE_SIMD_STRINGOPS
    size_t fast = len / sizeof(__m128i);
    size_t offset = fast * sizeof(__m128i);
    size_t current_block = 0;

    for (; current_block < fast; ++current_block) {
        __m128i lhs_epi8 = ::_mm_lddqu_si128((const __m128i*)lhs + current_block);
        __m128i rhs_epi8 = ::_mm_lddqu_si128((const __m128i*)rhs + current_block);

        __m128i lower_lhs_epi8 = ::_mm_blendv_epi8(
            lhs_epi8,
            ::_mm_add_epi8(_mm_sub_epi8(lhs_epi8, ::_mm_set1_epi8('A')), ::_mm_set1_epi8('a')),
            ::_mm_and_si128(
                ::_mm_cmpgt_epi8(lhs_epi8, ::_mm_set1_epi8('A' - 1)),
                ::_mm_cmplt_epi8(lhs_epi8, ::_mm_set1_epi8('Z' + 1))
            )
        );
        __m128i lower_rhs_epi8 = _mm_blendv_epi8(
            rhs_epi8,
            ::_mm_add_epi8(_mm_sub_epi8(rhs_epi8, ::_mm_set1_epi8('A')), ::_mm_set1_epi8('a')),
            ::_mm_and_si128(
                ::_mm_cmpgt_epi8(rhs_epi8, ::_mm_set1_epi8('A' - 1)),
                ::_mm_cmplt_epi8(rhs_epi8, ::_mm_set1_epi8('Z' + 1))
            )
        );

        if (::_mm_movemask_epi8(::_mm_cmpeq_epi8(lower_lhs_epi8, lower_rhs_epi8)) != 0xFFFF)
            return false;
    }

    for (; len > offset; ++offset) {
        if (ToLower(lhs[offset]) ^ ToLower(rhs[offset]))
            return false;
    }

    return true;
#else
    return (0 == ::_strnicmp(lhs, rhs, len));
#endif
}
static bool StrEqualsI_(const wchar_t* lhs, const wchar_t* rhs, size_t len) {
#if USE_CORE_SIMD_STRINGOPS && 0 // some ops are not available for epi16
    size_t fast = (len * sizeof(wchar_t)) / sizeof(__m128i);
    size_t offset = fast * sizeof(__m128i);
    size_t current_block = 0;

    for (; current_block < fast; ++current_block) {
        __m128i lhs_epi16 = ::_mm_lddqu_si128((const __m128i*)lhs + current_block);
        __m128i rhs_epi16 = ::_mm_lddqu_si128((const __m128i*)rhs + current_block);

        __m128i lower_lhs_epi16 = ::_mm_blend_epi16(
            lhs_epi16,
            ::_mm_add_epi16(_mm_sub_epi16(lhs_epi16, ::_mm_set1_epi16(L'A')), ::_mm_set1_epi16(L'a')),
            ::_mm_and_si128(
                ::_mm_cmpgt_epi16(lhs_epi16, ::_mm_set1_epi16(L'A' - 1)),
                ::_mm_cmplt_epi16(lhs_epi16, ::_mm_set1_epi16(L'Z' + 1))
            )
        );
        __m128i lower_rhs_epi16 = ::_mm_blend_epi16(
            rhs_epi16,
            ::_mm_add_epi16(_mm_sub_epi16(rhs_epi16, ::_mm_set1_epi16(L'A')), ::_mm_set1_epi16(L'a')),
            ::_mm_and_si128(
                ::_mm_cmpgt_epi16(rhs_epi16, ::_mm_set1_epi16(L'A' - 1)),
                ::_mm_cmplt_epi16(rhs_epi16, ::_mm_set1_epi16(L'Z' + 1))
            )
        );

        if (::_mm_movemask_epi8(::_mm_cmpeq_epi16(lower_lhs_epi16, lower_rhs_epi16)) != 0xFF)
            return false;
    }

    for (; len > offset; ++offset) {
        if (ToLower(lhs[offset]) ^ ToLower(rhs[offset]))
            return false;
    }

    return true;
#else
    return (0 == ::_wcsnicmp(lhs, rhs, len));
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// 6x-8x faster with SIMD :
//----------------------------------------------------------------------------
void ToLower(const TMemoryView<char>& dst, const TMemoryView<const char>& src) {
    Assert(dst.size() == src.size());

    const char* sbegin = src.data();
    const char* send = (sbegin + src.size());
    char* dbegin = dst.data();

#if USE_CORE_SIMD_STRINGOPS
    size_t fast = (send - sbegin) / sizeof(__m128i);
    size_t offset = fast * sizeof(__m128i);
    size_t current_block = 0;

    for (; current_block < fast; ++current_block) {
        __m128i src_epi8 = ::_mm_lddqu_si128((const __m128i*)sbegin + current_block);

        __m128i lower_epi8 = ::_mm_blendv_epi8(
            src_epi8,
            ::_mm_add_epi8(_mm_sub_epi8(src_epi8, ::_mm_set1_epi8('A')), ::_mm_set1_epi8('a')),
            ::_mm_and_si128(
                ::_mm_cmpgt_epi8(src_epi8, ::_mm_set1_epi8('A' - 1)),
                ::_mm_cmplt_epi8(src_epi8, ::_mm_set1_epi8('Z' + 1))
            )
        );

        ::_mm_storeu_si128((__m128i*)dbegin + current_block, lower_epi8);
    }

    sbegin += offset;
    dbegin += offset;
#endif

    for (; sbegin != send; ++sbegin, ++dbegin)
        *dbegin = ToLower(*sbegin);
}
//----------------------------------------------------------------------------
void ToLower(const TMemoryView<wchar_t>& dst, const TMemoryView<const wchar_t>& src) {
    Assert(dst.size() == src.size());

    const wchar_t* sbegin = src.data();
    const wchar_t* send = (sbegin + src.size());
    wchar_t* dbegin = dst.data();

#if USE_CORE_SIMD_STRINGOPS && 0 // some ops are not available for epi16
    size_t fast = ((send - sbegin) * sizeof(wchar_t)) / sizeof(__m128i);
    size_t offset = fast * sizeof(__m128i);
    size_t current_block = 0;

    for (; current_block < fast; ++current_block) {
        __m128i src_epi16 = ::_mm_lddqu_si128((const __m128i*)sbegin + current_block);

        __m128i lower_epi16 = ::_mm_blendv_epi16(
            src_epi16,
            ::_mm_add_epi16(_mm_sub_epi16(src_epi16, ::_mm_set1_epi16(L'A')), ::_mm_set1_epi16(L'a')),
            ::_mm_and_si128(
                ::_mm_cmpgt_epi16(src_epi16, ::_mm_set1_epi16(L'A' - 1)),
                ::_mm_cmplt_epi16(src_epi16, ::_mm_set1_epi16(L'Z' + 1))
            )
        );

        ::_mm_storeu_si128((__m128i*)dbegin + current_block, lower_epi8);
    }

    sbegin += offset;
    dbegin += offset;
#endif

    for (; sbegin != send; ++sbegin, ++dbegin)
        *dbegin = ToLower(*sbegin);
}
//----------------------------------------------------------------------------
void ToUpper(const TMemoryView<char>& dst, const TMemoryView<const char>& src) {
    Assert(dst.size() == src.size());

    const char* sbegin = src.data();
    const char* send = (sbegin + src.size());
    char* dbegin = dst.data();

#if USE_CORE_SIMD_STRINGOPS
    size_t fast = (send - sbegin) / sizeof(__m128i);
    size_t offset = fast * sizeof(__m128i);
    size_t current_block = 0;

    for (; current_block < fast; ++current_block) {
        __m128i src_epi8 = ::_mm_lddqu_si128((const __m128i*)sbegin + current_block);

        __m128i lower_epi8 = ::_mm_blendv_epi8(
            src_epi8,
            ::_mm_add_epi8(_mm_sub_epi8(src_epi8, ::_mm_set1_epi8('a')), ::_mm_set1_epi8('A')),
            ::_mm_and_si128(
                ::_mm_cmpgt_epi8(src_epi8, ::_mm_set1_epi8('a' - 1)),
                ::_mm_cmplt_epi8(src_epi8, ::_mm_set1_epi8('z' + 1))
            )
        );

        ::_mm_storeu_si128((__m128i*)dbegin + current_block, lower_epi8);
    }

    sbegin += offset;
    dbegin += offset;
#endif

    for (; sbegin != send; ++sbegin, ++dbegin)
        *dbegin = ToUpper(*sbegin);
}
//----------------------------------------------------------------------------
void ToUpper(const TMemoryView<wchar_t>& dst, const TMemoryView<const wchar_t>& src) {
    Assert(dst.size() == src.size());

    const wchar_t* sbegin = src.data();
    const wchar_t* send = (sbegin + src.size());
    wchar_t* dbegin = dst.data();

#if USE_CORE_SIMD_STRINGOPS && 0 // some ops are not available for epi16
    size_t fast = ((send - sbegin) * sizeof(wchar_t)) / sizeof(__m128i);
    size_t offset = fast * sizeof(__m128i);
    size_t current_block = 0;

    for (; current_block < fast; ++current_block) {
        __m128i src_epi16 = ::_mm_lddqu_si128((const __m128i*)sbegin + current_block);

        __m128i lower_epi16 = ::_mm_blendv_epi16(
            src_epi16,
            ::_mm_add_epi16(_mm_sub_epi16(src_epi16, ::_mm_set1_epi16(L'a')), ::_mm_set1_epi16(L'A')),
            ::_mm_and_si128(
                ::_mm_cmpgt_epi16(src_epi16, ::_mm_set1_epi16(L'a' - 1)),
                ::_mm_cmplt_epi16(src_epi16, ::_mm_set1_epi16(L'z' + 1))
            )
        );

        ::_mm_storeu_si128((__m128i*)dbegin + current_block, lower_epi8);
    }

    sbegin += offset;
    dbegin += offset;
#endif

    for (; sbegin != send; ++sbegin, ++dbegin)
        *dbegin = ToUpper(*sbegin);
}
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

    const int cmp = StrNCmp_(lhs.data(), rhs.data(), std::min(lhs.size(), rhs.size()));

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

    const int cmp = StrNCmp_(lhs.data(), rhs.data(), std::min(lhs.size(), rhs.size()));

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

    // Faster than SIMD on MSVC
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

    // Faster than SIMD on MSVC
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
bool EqualsN(const char* lhs, const char* rhs, size_t len) {
    return StrEquals_(lhs, rhs, len);
}
//----------------------------------------------------------------------------
bool EqualsNI(const char* lhs, const char* rhs, size_t len) {
    return StrEqualsI_(lhs, rhs, len);
}
//----------------------------------------------------------------------------
bool EqualsN(const wchar_t* lhs, const wchar_t* rhs, size_t len) {
    return StrEquals_(lhs, rhs, len);
}
//----------------------------------------------------------------------------
bool EqualsNI(const wchar_t* lhs, const wchar_t* rhs, size_t len) {
    return StrEqualsI_(lhs, rhs, len);
}
//----------------------------------------------------------------------------
bool Equals(const FStringView& lhs, const FStringView& rhs) {
    return ((lhs.size() == rhs.size()) &&
            (lhs.data() == rhs.data() || StrEquals_(lhs.data(), rhs.data(), lhs.size())) );
}
//----------------------------------------------------------------------------
bool Equals(const FWStringView& lhs, const FWStringView& rhs) {
    return ((lhs.size() == rhs.size()) &&
            (lhs.data() == rhs.data() || StrEquals_(lhs.data(), rhs.data(), lhs.size())) );
}
//----------------------------------------------------------------------------
bool EqualsI(const FStringView& lhs, const FStringView& rhs) {
    return ((lhs.size() == rhs.size()) &&
            (lhs.data() == rhs.data() || StrEqualsI_(lhs.data(), rhs.data(), lhs.size())) );
}
//----------------------------------------------------------------------------
bool EqualsI(const FWStringView& lhs, const FWStringView& rhs) {
    return ((lhs.size() == rhs.size()) &&
            (lhs.data() == rhs.data() || StrEqualsI_(lhs.data(), rhs.data(), lhs.size())) );
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
size_t EditDistance(const FStringView& lhs, const FStringView& rhs) {
    return LevenshteinDistance_<char, ECase::Sensitive>(lhs, rhs);
}
//----------------------------------------------------------------------------
size_t EditDistance(const FWStringView& lhs, const FWStringView& rhs) {
    return LevenshteinDistance_<wchar_t, ECase::Sensitive>(lhs, rhs);
}
//----------------------------------------------------------------------------
size_t EditDistanceI(const FStringView& lhs, const FStringView& rhs) {
    return LevenshteinDistance_<char, ECase::Insensitive>(lhs, rhs);
}
//----------------------------------------------------------------------------
size_t EditDistanceI(const FWStringView& lhs, const FWStringView& rhs) {
    return LevenshteinDistance_<wchar_t, ECase::Insensitive>(lhs, rhs);
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
#if 0 // don't want to use another hash function :/
    char (*transform)(char) = &ToLower;
    return hash_fnv1a(MakeOutputIterator(str.begin(), transform), MakeOutputIterator(str.end(), transform));
#else
    const size_t sz = str.size();
    STACKLOCAL_POD_ARRAY(u8, istr, sz);
    ::memcpy(istr.data(), str.data(), sz);
    return hash_mem(istr.data(), sz);
#endif
}
//----------------------------------------------------------------------------
hash_t hash_stringI(const FWStringView& wstr) {
#if 0 // don't want to use another hash function :/
    wchar_t (*transform)(wchar_t) = &ToLower;
    return hash_fnv1a(MakeOutputIterator(wstr.begin(), transform), MakeOutputIterator(wstr.end(), transform));
#else
    const size_t sz = wstr.size();
    STACKLOCAL_POD_ARRAY(wchar_t, iwstr, sz);
    ToLower(iwstr, wstr);
    return hash_mem(iwstr.data(), iwstr.SizeInBytes());
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Escape(std::basic_ostream<char>& oss, const FStringView& str, EEscape escape) {
    for (char ch : str) {
        switch (ch) {
        case '\\':
        case '"':
            oss << '\\' << ch;
            break;
        case '\b':
            oss << "\\b";
            break;
        case '\t':
            oss << "\\t";
            break;
        case '\n':
            oss << "\\n";
            break;
        case '\f':
            oss << "\\f";
            break;
        case '\r':
            oss << "\\r";
            break;
        default:
            if (IsPrint(ch))
                oss << ch;
            else {
                static constexpr char hexdig[] = "0123456789ABCDEF";

                switch (escape) {
                case Core::EEscape::Octal:
                    oss.put('\\');
                    oss << std::oct << (u32(ch) & 0xFF) << std::dec;
                    break;
                case Core::EEscape::Hexadecimal:
                    oss << "\\x";
                    oss.put(hexdig[u8(ch) >> 4]);
                    oss.put(hexdig[u8(ch) & 0xF]);
                    break;
                case Core::EEscape::Unicode:
                    oss << "\\u00";
                    oss.put(hexdig[u8(ch) >> 4]);
                    oss.put(hexdig[u8(ch) & 0xF]);
                    break;
                default:
                    AssertNotImplemented();
                    break;
                }
            }
        }
    }
}
//----------------------------------------------------------------------------
void Escape(std::basic_ostream<wchar_t>& oss, const FWStringView& wstr, EEscape escape) {
    for (wchar_t ch : wstr) {
        switch (ch) {
        case L'\\':
        case L'"':
            oss << L'\\' << ch;
            break;
        case '\b':
            oss << L"\\b";
            break;
        case '\t':
            oss << L"\\t";
            break;
        case '\n':
            oss << L"\\n";
            break;
        case '\f':
            oss << L"\\f";
            break;
        case '\r':
            oss << L"\\r";
            break;
        default:
            if (IsPrint(ch))
                oss << ch;
            else {
                static constexpr wchar_t hexdig[] = L"0123456789ABCDEF";

                switch (escape) {
                case Core::EEscape::Octal:
                    oss.put(L'\\');
                    oss << std::oct << (u32(ch) & 0xFF) << std::dec;
                    break;
                case Core::EEscape::Hexadecimal:
                    oss << L"\\x";
                    oss.put(hexdig[u8(ch) >> 4]);
                    oss.put(hexdig[u8(ch) & 0xF]);
                    break;
                case Core::EEscape::Unicode:
                    oss << L"\\u00";
                    oss.put(hexdig[u8(ch) >> 4]);
                    oss.put(hexdig[u8(ch) & 0xF]);
                    break;
                default:
                    AssertNotImplemented();
                    break;
                }
            }
        }
    }
}
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
