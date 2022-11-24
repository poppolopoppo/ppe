#pragma once

#include "HAL/Generic/GenericPlatformHash.h"

#ifdef PLATFORM_WINDOWS

#include <intrin.h>

#define USE_PPE_MM_CRC32_MIXER_ON_WINDOWS (1) //%_NOCOMMIT%

#ifndef __clang__
#   pragma intrinsic(_mm_crc32_u8)
#   pragma intrinsic(_mm_crc32_u16)
#   pragma intrinsic(_mm_crc32_u32)
#   ifdef ARCH_X64
#       pragma intrinsic(_mm_crc32_u64)
#   endif
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformHash : FGenericPlatformHash {
public:

    static FORCE_INLINE size_t CRC32(u32 v, size_t seed = 0) NOEXCEPT {
        return CODE3264(::_mm_crc32_u32, ::_mm_crc32_u64)(seed, v);
    }
    static FORCE_INLINE size_t CRC32(u64 v, size_t seed = 0) NOEXCEPT {
#ifdef ARCH_X64
        return ::_mm_crc32_u64(seed, v);
#else
        return ::_mm_crc32_u32(::_mm_crc32_u32(seed, u32(v)), u32(v >> 32));
#endif
    }
    static FORCE_INLINE size_t CRC32(u96 v) NOEXCEPT {
        STATIC_ASSERT(sizeof(v.hi) == sizeof(u32));
#ifdef ARCH_X64
        return ::_mm_crc32_u64(v.hi, v.lo);
#else
        return ::_mm_crc32_u32(::_mm_crc32_u32(v.lo >> 32, u32(v.lo)), v.hi);
#endif
    }
    static FORCE_INLINE size_t CRC32(u128 v) NOEXCEPT {
#ifdef ARCH_X64
        return ::_mm_crc32_u64(::_mm_crc32_u64(0, v.lo), v.hi);
#else
        return ::_mm_crc32_u32(
            ::_mm_crc32_u32(v.lo >> 32, u32(v.lo)),
            ::_mm_crc32_u32(v.hi >> 32, u32(v.hi)) );
#endif
    }
    static FORCE_INLINE size_t CRC32(u160 v) NOEXCEPT {
        STATIC_ASSERT(sizeof(v.hi) == sizeof(u32));
        return ::_mm_crc32_u32(u32(CRC32(v.lo)), v.hi);
    }
    static FORCE_INLINE size_t CRC32(u192 v) NOEXCEPT {
        STATIC_ASSERT(sizeof(v.hi) == sizeof(u64));
#ifdef ARCH_X64
        return ::_mm_crc32_u64(CRC32(v.lo), v.hi);
#else
        return ::_mm_crc32_u32(CRC32(v.lo), CRC32(v.hi));
#endif
    }
    static FORCE_INLINE size_t CRC32(u224 v) NOEXCEPT {
        STATIC_ASSERT(sizeof(v.hi) == sizeof(u96));
        return ::_mm_crc32_u32(u32(CRC32(v.lo)), u32(CRC32(v.hi)));
    }
    static FORCE_INLINE size_t CRC32(u256 v) NOEXCEPT {
        return ::_mm_crc32_u32(u32(CRC32(v.lo)), u32(CRC32(v.hi)));
    }

    static FORCE_INLINE size_t CRC32(size_t crc32, const void* p, size_t size) NOEXCEPT {
        Assert_NoAssume(Meta::IsAlignedPow2(sizeof(intptr_t), p));

        const u32 fast = u32(size / sizeof(intptr_t));
        u32 offset = fast * sizeof(intptr_t);
        u32 current_block = 0;

        crc32 ^= 0xffffffff;

        for (; current_block < fast; ++current_block)
            crc32 = CODE3264(::_mm_crc32_u32, ::_mm_crc32_u64)(crc32,
                ((const CODE3264(u32, u64)*)p)[current_block] );

        for (; offset < size; ++offset)
            crc32 = ::_mm_crc32_u8(u32(crc32), ((const u8*)p)[offset]);

        return (crc32 ^ 0xffffffff);
    }

    using FGenericPlatformHash::HashMem32;
    using FGenericPlatformHash::HashMem64;
    using FGenericPlatformHash::HashMem;

    using FGenericPlatformHash::FNV1a32;
    using FGenericPlatformHash::FNV1a64;

    using FGenericPlatformHash::GoodOAAT;
    using FGenericPlatformHash::MicroOAAT;

    using FGenericPlatformHash::HashUInt;
    using FGenericPlatformHash::HashUInt32;
    using FGenericPlatformHash::HashUInt64;

    using FGenericPlatformHash::FMix32;
    using FGenericPlatformHash::FMix64;

    using FGenericPlatformHash::HashCombine;
    using FGenericPlatformHash::HashCombine32;
    using FGenericPlatformHash::HashCombine64;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
