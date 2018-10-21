#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformString.h"

#ifdef PLATFORM_WINDOWS

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "IO/StringView.h"

#include "HAL/Windows/LastError.h"
#include "HAL/Windows/WindowsPlatformIncludes.h"
#include "HAL/Windows/WindowsPlatformMaths.h"

#include <emmintrin.h>
#include <intrin.h>

#define USE_PPE_SIMD_STRINGOPS 1 // turn to 0 to disable SIMD optimizations %_NOCOMMIT%

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FWindowsPlatformString::Equals(const char* lhs, const char* rhs, size_t len) {
    Assert(lhs);
    Assert(rhs);

#if USE_PPE_SIMD_STRINGOPS // 2x-3x faster than ::strncmp() == 0
    size_t fast = len / sizeof(__m128i);
    size_t offset = fast * sizeof(__m128i);
    size_t current_block = 0;

    for (; current_block < fast; ++current_block) {
        __m128i lhs_epi8 = ::_mm_lddqu_si128((const __m128i*)lhs + current_block);
        __m128i rhs_epi8 = ::_mm_lddqu_si128((const __m128i*)rhs + current_block);

        if (::_mm_movemask_epi8(::_mm_cmpeq_epi8(lhs_epi8, rhs_epi8)) != 0xFFFF)
            return false;
    }

    ::_mm_lfence();

    for (; len > offset; ++offset) {
        if (lhs[offset] ^ rhs[offset])
            return false;
    }

    return true;
#else
    return (0 == ::strncmp(lhs, rhs, len));
#endif
}
//----------------------------------------------------------------------------
bool FWindowsPlatformString::Equals(const wchar_t* lhs, const wchar_t* rhs, size_t len) {
    Assert(lhs);
    Assert(rhs);

#if USE_PPE_SIMD_STRINGOPS && 0 // some ops are not available for epi16
    size_t fast = (len * sizeof(wchar_t)) / sizeof(__m128i);
    size_t offset = fast * sizeof(__m128i);
    size_t current_block = 0;

    for (; current_block < fast; ++current_block) {
        __m128i lhs_epi16 = ::_mm_lddqu_si128((const __m128i*)lhs + current_block);
        __m128i rhs_epi16 = ::_mm_lddqu_si128((const __m128i*)rhs + current_block);

        if (::_mm_movemask_epi16(::_mm_cmpeq_epi16(lhs_epi16, rhs_epi16)) != 0xFF)
            return false;
    }

    ::_mm_lfence();

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
bool FWindowsPlatformString::EqualsI(const char* lhs, const char* rhs, size_t len) {
    Assert(lhs);
    Assert(rhs);

#if USE_PPE_SIMD_STRINGOPS // 2x-3x faster than ::strncmp() == 0
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

    ::_mm_lfence();

    for (; len > offset; ++offset) {
        if (PPE::ToLower(lhs[offset]) ^ PPE::ToLower(rhs[offset]))
            return false;
    }

    return true;
#else
    return (0 == ::_strnicmp(lhs, rhs, len));
#endif
}
//----------------------------------------------------------------------------
bool FWindowsPlatformString::EqualsI(const wchar_t* lhs, const wchar_t* rhs, size_t len) {
    Assert(lhs);
    Assert(rhs);

#if USE_PPE_SIMD_STRINGOPS && 0 // some ops are not available for epi16
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

    ::_mm_lfence();

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
int FWindowsPlatformString::NCmp(const char* lhs, const char* rhs, size_t len) {
    Assert(lhs);
    Assert(rhs);

#if defined(ARCH_X64) && USE_PPE_SIMD_STRINGOPS // 1x-2x faster with SIMD on x64 :
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
            return ((i8*)&cmpsg)[FWindowsPlatformMaths::tzcnt(mask)];
    }

    ::_mm_lfence();

    for (; len > offset; ++offset) {
        if (lhs[offset] ^ rhs[offset])
            return (int)((unsigned char)lhs[offset] - (unsigned char)rhs[offset]);
    }

    return 0;
#else
    return ::strncmp(lhs, rhs, len);
#endif
}
//----------------------------------------------------------------------------
int FWindowsPlatformString::NCmp(const wchar_t* lhs, const wchar_t* rhs, size_t len) {
    Assert(lhs);
    Assert(rhs);

#if defined(ARCH_X64) && USE_PPE_SIMD_STRINGOPS && 0 // some ops are not available for epi16
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
            return cmpsg.m128i_i16[FWindowsPlatformMaths::tzcnt(mask)];
    }

    ::_mm_lfence();

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
int FWindowsPlatformString::NCmpI(const char* lhs, const char* rhs, size_t len) {
    Assert(lhs);
    Assert(rhs);

    return ::_strnicmp(lhs, rhs, len); // faster than SIMD
}
//----------------------------------------------------------------------------
int FWindowsPlatformString::NCmpI(const wchar_t* lhs, const wchar_t* rhs, size_t len) {
    Assert(lhs);
    Assert(rhs);

    return ::_wcsnicmp(lhs, rhs, len); // faster than SIMD
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformString::CHAR_to_WCHAR(ECodePage codePage, wchar_t* dst, size_t capacity, const char* cstr, size_t length) {
    Assert(dst);
    Assert(capacity);
    Assert(capacity > length);

    // https://www.chilkatsoft.com/p/p_348.asp

    if (0 == length) {
        dst[0] = L'\0';
        return 0;
    }

    Assert(cstr);

    const size_t written = ::MultiByteToWideChar(
        int(codePage),
        0,
        cstr, checked_cast<int>(length),
        dst, checked_cast<int>(capacity));
    CLOG(0 == written, HAL, Fatal, L"CHAR_to_WCHAR failed : {0}\n{1}", FLastError{}, Fmt::HexDump(cstr, length));

    Assert(written >= length);
    Assert(written < capacity);

    dst[written] = L'\0'; // MultiByteToWideChar() won't use a null terminator since we specified the length of input

    return (written + 1);
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformString::WCHAR_to_CHAR(ECodePage codePage, char* dst, size_t capacity, const wchar_t* wcstr, size_t length) {
    Assert(dst);
    Assert(capacity);
    Assert(capacity > length);

    // https://www.chilkatsoft.com/p/p_348.asp

    if (0 == length) {
        dst[0] = '\0';
        return 0;
    }

    Assert(wcstr);

    const size_t written = ::WideCharToMultiByte(
        int(codePage),
        0,
        wcstr, checked_cast<int>(length),
        dst, checked_cast<int>(capacity),
        0, 0);
    CLOG(0 == written, HAL, Fatal, L"WCHAR_to_CHAR failed : {0}\n{1}", FLastError{}, Fmt::HexDump(wcstr, length));

    Assert(written >= length);
    Assert(written < capacity);

    dst[written] = '\0'; // WideCharToMultiByte() won't use a null terminator since we specified the length of input

    return (written + 1);
}
//----------------------------------------------------------------------------
void FWindowsPlatformString::ToLower(char* dst, const char* src, size_t len) {
    Assert(dst);
    Assert(src);

    const char* sbegin = src;
    const char* send = (sbegin + len);
    char* dbegin = dst;

#if USE_PPE_SIMD_STRINGOPS // 6x-8x faster with SIMD
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

    ::_mm_sfence();

    sbegin += offset;
    dbegin += offset;
#endif

    for (; sbegin != send; ++sbegin, ++dbegin)
        *dbegin = PPE::ToLower(*sbegin);
}
//----------------------------------------------------------------------------
void FWindowsPlatformString::ToLower(wchar_t* dst, const wchar_t* src, size_t len) {
    Assert(dst);
    Assert(src);

    const wchar_t* sbegin = src;
    const wchar_t* send = (sbegin + len);
    wchar_t* dbegin = dst;

#if USE_PPE_SIMD_STRINGOPS && 0 // some ops are not available for epi16
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

    ::_mm_sfence();

    sbegin += offset;
    dbegin += offset;
#endif

    for (; sbegin != send; ++sbegin, ++dbegin)
        *dbegin = PPE::ToLower(*sbegin);
}
//----------------------------------------------------------------------------
void FWindowsPlatformString::ToUpper(char* dst, const char* src, size_t len) {
    Assert(dst);
    Assert(src);

    const char* sbegin = src;
    const char* send = (sbegin + len);
    char* dbegin = dst;

#if USE_PPE_SIMD_STRINGOPS // 6x-8x faster with SIMD
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

    ::_mm_sfence();

    sbegin += offset;
    dbegin += offset;
#endif

    for (; sbegin != send; ++sbegin, ++dbegin)
        *dbegin = PPE::ToUpper(*sbegin);
}
//----------------------------------------------------------------------------
void FWindowsPlatformString::ToUpper(wchar_t* dst, const wchar_t* src, size_t len) {
    Assert(dst);
    Assert(src);

    const wchar_t* sbegin = src;
    const wchar_t* send = (sbegin + len);
    wchar_t* dbegin = dst;

#if USE_PPE_SIMD_STRINGOPS && 0 // some ops are not available for epi16
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

    ::_mm_sfence();

    sbegin += offset;
    dbegin += offset;
#endif

    for (; sbegin != send; ++sbegin, ++dbegin)
        *dbegin = PPE::ToUpper(*sbegin);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
