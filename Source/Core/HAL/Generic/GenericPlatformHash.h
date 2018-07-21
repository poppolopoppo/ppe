#pragma once

#include "Core/HAL/TargetPlatform.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct CORE_API FGenericPlatformHash {
public: // must be defined for every platform

    static u32 CRC32(u32 seed, const void* p, size_t size) NOEXCEPT;

    static u32 FNV1a32(u32 seed, const void* key, const size_t sizeInBytes) NOEXCEPT {
        u32 hash = seed ^ 0x811c9dc5ul;
        forrange(pbyte, (const u8*)key, (const u8*)key + sizeInBytes) {
            hash = hash ^ *pbyte;
            hash *= 0x1000193ul;
        }
        return hash;
    }

    static u64 FNV1a64(u64 seed, const void* key, const size_t sizeInBytes) NOEXCEPT {
        u64 hash = seed ^ 0xcbf29ce484222325ull;
        forrange(pbyte, (const u8*)key, (const u8*)key + sizeInBytes) {
            hash = hash ^ *pbyte;
            hash *= 0x100000001b3ull;
        }
        return hash;
    }

    static u32 HashMem32(u32 seed, const void* p, size_t size) NOEXCEPT = delete;
    static u64 HashMem64(u64 seed, const void* p, size_t size) NOEXCEPT = delete;

    static size_t HashMem(size_t seed, const void* p, size_t size) NOEXCEPT = delete;

public: // generic helpers

    //------------------------------------------------------------------------
    // Finger printing *SHOULD NEVER CHANGE THOSE*

    static u32 Fingeprint32(const void* p, size_t size) NOEXCEPT;
    static u64 Fingeprint64(const void* p, size_t size) NOEXCEPT;
    static u128 Fingeprint128(const void* p, size_t size) NOEXCEPT;

    //------------------------------------------------------------------------
    // Murmur-inspired hashing: https://github.com/google/farmhash/blob/master/dev/farmhash.h

    static FORCE_INLINE CONSTEXPR u64 HashUInt64(u64 b) NOEXCEPT {
        b *= 0x9ddfea08eb382d69ULL;
        b ^= (b >> 44);
        b *= 0x9ddfea08eb382d69ULL;
        b ^= (b >> 41);
        b *= 0x9ddfea08eb382d69ULL;
        return b;
    }

    //----------------------------------------------------------------------------
    // Murmur-inspired hashing: https://github.com/google/farmhash/blob/master/dev/farmhash.h

    static FORCE_INLINE CONSTEXPR u64 HashUInt64(u128 b) NOEXCEPT {
        b.lo = (b.lo ^ b.hi) * 0x9ddfea08eb382d69ULL;
        b.lo ^= (b.lo >> 47);
        b.hi = (b.hi ^ b.lo) * 0x9ddfea08eb382d69ULL;
        b.hi ^= (b.hi >> 44);
        b.hi *= 0x9ddfea08eb382d69ULL;
        b.hi ^= (b.hi >> 41);
        b.hi *= 0x9ddfea08eb382d69ULL;
        return b.hi;
    }

    //----------------------------------------------------------------------------
    // Burtle, Full avalanche: http://burtleburtle.net/bob/hash/integer.html

    static FORCE_INLINE CONSTEXPR u32 HashUInt32(u32 a) NOEXCEPT {
        a = (a + 0x7ed55d16) + (a << 12);
        a = (a ^ 0xc761c23c) ^ (a >> 19);
        a = (a + 0x165667b1) + (a << 5);
        a = (a + 0xd3a2646c) ^ (a << 9);
        a = (a + 0xfd7046c5) + (a << 3);
        a = (a ^ 0xb55a4f09) ^ (a >> 16);
        return a;
    }

    //----------------------------------------------------------------------------
    // Thomas Wang's 64 bit Mix Function: http://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm

    static FORCE_INLINE CONSTEXPR u32 HashUInt32(u64 key) NOEXCEPT {
        key += ~(key << 32);
        key ^= (key >> 22);
        key += ~(key << 13);
        key ^= (key >> 8);
        key += (key << 3);
        key ^= (key >> 15);
        key += ~(key << 27);
        key ^= (key >> 31);
        return u32(key);
    }

    //----------------------------------------------------------------------------
    // Fall back on Hash64 for simplicity

    static FORCE_INLINE CONSTEXPR u32 HashUInt32(u128 value) NOEXCEPT {
        return u32(HashUInt64(value)); // takes lower bits, should be alright
    }

    //----------------------------------------------------------------------------
    // Finalization mix - force all bits of a hash block to avalanche

    static FORCE_INLINE CONSTEXPR u64 FMix64(u64 k) NOEXCEPT {
        k ^= k >> 33;
        k *= 0xff51afd7ed558ccdull;
        k ^= k >> 33;
        k *= 0xc4ceb9fe1a85ec53ull;
        k ^= k >> 33;
        return k;
    }

    //----------------------------------------------------------------------------
#ifdef ARCH_X64
    static FORCE_INLINE CONSTEXPR u64 HashUInt(u32 value) NOEXCEPT { return HashUInt64(u64(value)); }
    static FORCE_INLINE CONSTEXPR u64 HashUInt(u64 value) NOEXCEPT { return HashUInt64(value); }
    static FORCE_INLINE CONSTEXPR u64 HashUInt(u128 value) NOEXCEPT { return HashUInt64(value); }
#else
    static FORCE_INLINE CONSTEXPR u32 HashUInt(u32 value) NOEXCEPT { return HashUInt32(value); }
    static FORCE_INLINE CONSTEXPR u32 HashUInt(u64 value) NOEXCEPT { return HashUInt32(value); }
    static FORCE_INLINE CONSTEXPR u32 HashUInt(u128 value) NOEXCEPT { return HashUInt32(value); }
#endif

    //------------------------------------------------------------------------
    // https://www.boost.org/doc/libs/1_64_0/boost/functional/hash/hash.hpp

    static FORCE_INLINE CONSTEXPR u32 HashCombine32(u32 h, u32 k) NOEXCEPT {
        k *= 0xcc9e2d51ul;
        k = (k << 15ul) | (k >> (32ul - 15ul));
        k *= 0x1b873593ul;
        h ^= k;
        h = (h << 13ul) | (h >> (32ul - 13ul));
        h = h * 5ul + 0xe6546b64ul;
        return h;
    }

    static FORCE_INLINE CONSTEXPR u64 HashCombine64(u64 h, u64 k) NOEXCEPT {
        k *= 0xc6a4a7935bd1e995ull;
        k ^= k >> 47ull;
        k *= 0xc6a4a7935bd1e995ull;
        h ^= k;
        h *= 0xc6a4a7935bd1e995ull;
        h += 0xe6546b64ull; // Completely arbitrary number, to prevent 0's from hashing to 0.
        return h;
    }

    static FORCE_INLINE CONSTEXPR size_t HashCombine(size_t seed, size_t value) NOEXCEPT {
        return CODE3264(HashCombine32, HashCombine64)(seed, value);
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
