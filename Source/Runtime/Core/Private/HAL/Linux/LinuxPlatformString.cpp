#include "stdafx.h"

#include "HAL/Linux/LinuxPlatformString.h"

#ifdef PLATFORM_LINUX

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "IO/StringView.h"

#include "HAL/Linux/Errno.h"
#include "HAL/Linux/LinuxPlatformIncludes.h"
#include "HAL/Linux/LinuxPlatformMaths.h"

#include <emmintrin.h>

#define USE_PPE_SIMD_STRINGOPS 1 // turn to 0 to disable SIMD optimizations %_NOCOMMIT%

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FLinuxPlatformString::Equals(const char* lhs, const char* rhs, size_t len) NOEXCEPT {
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
bool FLinuxPlatformString::Equals(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT {
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
bool FLinuxPlatformString::EqualsI(const char* lhs, const char* rhs, size_t len) NOEXCEPT {
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
bool FLinuxPlatformString::EqualsI(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT {
    Assert(lhs);
    Assert(rhs);
    return (0 == ::wcsncasecmp(lhs, rhs, len));
}
//----------------------------------------------------------------------------
int FLinuxPlatformString::NCmp(const char* lhs, const char* rhs, size_t len) NOEXCEPT {
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
            return ((i8*)&cmpsg)[FLinuxPlatformMaths::tzcnt(mask)];
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
int FLinuxPlatformString::NCmp(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT {
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
            return cmpsg.m128i_i16[FLinuxPlatformMaths::tzcnt(mask)];
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
int FLinuxPlatformString::NCmpI(const char* lhs, const char* rhs, size_t len) NOEXCEPT {
    Assert(lhs);
    Assert(rhs);

    return ::strncasecmp(lhs, rhs, len); // faster than SIMD
}
//----------------------------------------------------------------------------
int FLinuxPlatformString::NCmpI(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT {
    Assert(lhs);
    Assert(rhs);

    return ::wcsncasecmp(lhs, rhs, len); // faster than SIMD
}
//----------------------------------------------------------------------------
void FLinuxPlatformString::ToLower(char* dst, const char* src, size_t len) NOEXCEPT {
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
void FLinuxPlatformString::ToLower(wchar_t* dst, const wchar_t* src, size_t len) NOEXCEPT {
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
void FLinuxPlatformString::ToUpper(char* dst, const char* src, size_t len) NOEXCEPT {
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
void FLinuxPlatformString::ToUpper(wchar_t* dst, const wchar_t* src, size_t len) NOEXCEPT {
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
size_t FLinuxPlatformString::CHAR_to_WCHAR(ECodePage codePage, wchar_t* dst, size_t capacity, const char* cstr, size_t length) {
    Assert(dst);
    Assert(capacity);
    Assert(capacity > length);

    if (0 == length) {
        dst[0] = L'\0';
        return 1;
    }

    Assert(cstr);
    Unused(capacity);
    Unused(codePage);

    size_t od = 0;
    for (size_t os = 0; os != length && od < capacity; ++od) {
        const int written = ::mbtowc(dst + od, cstr + os, length - os);
        CLOG(-1 == written, HAL, Fatal, L"mbtowc failed with errno: {0}\n{1}", FErrno{}, Fmt::HexDump(cstr + os, length - os));
        os += checked_cast<size_t>(written);
    }

    Assert(od >= length);
    Assert(od < capacity);
    if (od < capacity)
        dst[od] = L'\0';

    return checked_cast<size_t>(od + 1);
}
//----------------------------------------------------------------------------
size_t FLinuxPlatformString::WCHAR_to_CHAR(ECodePage codePage, char* dst, size_t capacity, const wchar_t* wcstr, size_t length) {
    Assert(dst);
    Assert(capacity);
    Assert(capacity > length);

    if (0 == length) {
        dst[0] = '\0';
        return 1;
    }

    Assert(wcstr);
    Unused(codePage);

    size_t od = 0;
    for (size_t os = 0; os != length && od < capacity; ++os) {
        const int written = ::wctomb(dst + od, wcstr[os]);
        CLOG(-1 == written, HAL, Fatal, L"wctomb failed with errno: {0}\n{1}", FErrno{}, Fmt::HexDump(wcstr + os, length - os));
        od += checked_cast<size_t>(written);
    }

    Assert(od >= length);
    Assert(od < capacity);
    if (od < capacity)
        dst[od] = '\0';

    return checked_cast<size_t>(od + 1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
