#pragma once

#include "HAL/Generic/GenericPlatformHash.h"

#ifdef PLATFORM_WINDOWS

#include <intrin.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformHash : FGenericPlatformHash {
public:

    static FORCE_INLINE size_t CRC32(size_t crc32, const void* p, size_t size) NOEXCEPT {
        Assert_NoAssume(Meta::IsAligned(sizeof(intptr_t), p));

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

    static u32 HashMem32(u32 seed, const void* p, size_t size) NOEXCEPT;
    static u64 HashMem64(u64 seed, const void* p, size_t size) NOEXCEPT;

    static FORCE_INLINE size_t HashMem(size_t seed, const void* p, size_t size) NOEXCEPT {
#ifdef ARCH_X64
        return HashMem64(seed, p, size);
#else
        return HashMem32(seed, p, size);
#endif
    }

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
