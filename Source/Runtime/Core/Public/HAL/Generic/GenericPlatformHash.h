#pragma once

#include "HAL/TargetPlatform.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformHash {
public: // must be defined for every platform

    static size_t CRC32(u32 v) NOEXCEPT = delete;
    static size_t CRC32(u64 v) NOEXCEPT = delete;
    static size_t CRC32(u96 v) NOEXCEPT = delete;
    static size_t CRC32(u128 v) NOEXCEPT = delete;
    static size_t CRC32(u160 v) NOEXCEPT = delete;
    static size_t CRC32(u192 v) NOEXCEPT = delete;
    static size_t CRC32(u224 v) NOEXCEPT = delete;
    static size_t CRC32(u256 v) NOEXCEPT = delete;

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

    // One of a smallest non-multiplicative One-At-a-Time function
    // that passes whole SMHasher.
    // Author: Sokolov Yura aka funny-falcon <funny.falcon@gmail.com>
    // https://github.com/rurban/smhasher/blob/master/Hashes.cpp
    static CONSTEXPR u32 GoodOAAT(const void* key, size_t len, u32 seed) NOEXCEPT {
#define grol(x,n) (((x)<<(n))|((x)>>(32-(n))))
#define gror(x,n) (((x)>>(n))|((x)<<(32-(n))))
        u8* str = (u8*)key;
        const u8* const end = (const u8*)str + len;
        u32 h1 = seed ^ 0x3b00;
        u32 h2 = grol(seed, 15);
        for (; str != end; str++) {
            h1 += str[0];
            h1 += h1 << 3; // h1 *= 9
            h2 += h1;
            // the rest could be as in MicroOAAT: h1 = grol(h1, 7)
            // but clang doesn't generate ROTL instruction then.
            h2 = grol(h2, 7);
            h2 += h2 << 2; // h2 *= 5
        }
        h1 ^= h2;
        /* now h1 passes all collision checks,
         * so it is suitable for hash-tables with prime numbers. */
        h1 += grol(h2, 14);
        h2 ^= h1; h2 += gror(h1, 6);
        h1 ^= h2; h1 += grol(h2, 5);
        h2 ^= h1; h2 += gror(h1, 8);
        return h2;
#undef grol
#undef gror
    }

    // MicroOAAT suitable for hash-tables using prime numbers.
    // It passes all collision checks.
    // Author: Sokolov Yura aka funny-falcon <funny.falcon@gmail.com>
    // https://github.com/rurban/smhasher/blob/master/Hashes.cpp
    static CONSTEXPR u32 MicroOAAT(const void* key, size_t len, u32 seed) NOEXCEPT {
#define grol(x,n) (((x)<<(n))|((x)>>(32-(n))))
#define gror(x,n) (((x)>>(n))|((x)<<(32-(n))))
        u8* str = (u8*)key;
        const u8* const end = (const u8*)str + len;
        u32 h1 = seed ^ 0x3b00;
        u32 h2 = grol(seed, 15);
        for (; str != end; str++) {
            h1 += str[0];
            h1 += h1 << 3; // h1 *= 9
            h2 -= h1;
            h1 = grol(h1, 7);
        }
        return (h1 ^ h2);
#undef grol
#undef gror
    }

    static u32 HashMem32(u32 seed, const void* p, size_t size) NOEXCEPT = delete;
    static u64 HashMem64(u64 seed, const void* p, size_t size) NOEXCEPT = delete;
    static u128 HashMem128(u128 seed, const void* p, size_t size) NOEXCEPT = delete;

    static size_t HashMem(size_t seed, const void* p, size_t size) NOEXCEPT = delete;

public: // generic helpers

    //------------------------------------------------------------------------
    // Finger printing *SHOULD NEVER CHANGE THOSE*

    static u32 Fingeprint32(const void* p, size_t size) NOEXCEPT;
    static u64 Fingeprint64(const void* p, size_t size) NOEXCEPT;
    static u128 Fingeprint128(const void* p, size_t size) NOEXCEPT;

    //----------------------------------------------------------------------------
    // Finalization mix - force all bits of a hash block to avalanche

#if 0
    // XXH32_avalanche()
    static FORCE_INLINE CONSTEXPR u32 FMix32(u32 h32) NOEXCEPT {
        h32 ^= h32 >> 15;
        h32 *= 2246822519U;
        h32 ^= h32 >> 13;
        h32 *= 3266489917U;
        h32 ^= h32 >> 16;
        return h32;
    }

    // XXH64_avalanche()
    static FORCE_INLINE CONSTEXPR u64 FMix64(u64 h64) NOEXCEPT {
        h64 ^= h64 >> 33;
        h64 *= 14029467366897019727ULL;
        h64 ^= h64 >> 29;
        h64 *= 1609587929392839161ULL;
        h64 ^= h64 >> 32;
        return h64;
    }

#else
    // https://nullprogram.com/blog/2018/07/31/
    // exact bias: 0.17353355999581582
    // uint32_t lowbias32(uint32_t x)
    static FORCE_INLINE CONSTEXPR u32 FMix32(u32 x) NOEXCEPT {
        x ^= x >> 16;
        x *= UINT32_C(0x7feb352d);
        x ^= x >> 15;
        x *= UINT32_C(0x846ca68b);
        x ^= x >> 16;
        return x;
    }

    // http://xoshiro.di.unimi.it/splitmix64.c
    static FORCE_INLINE CONSTEXPR u64 FMix64(u64 x) NOEXCEPT {
        x ^= x >> 30;
        x *= UINT64_C(0xbf58476d1ce4e5b9);
        x ^= x >> 27;
        x *= UINT64_C(0x94d049bb133111eb);
        x ^= x >> 31;
        return x;
    }

#endif

    //----------------------------------------------------------------------------

    static FORCE_INLINE CONSTEXPR u32 HashUInt32(u32 key) NOEXCEPT { return FMix32(key); }
    static FORCE_INLINE CONSTEXPR u64 HashUInt64(u32 key) NOEXCEPT { return FMix64(u64(key)); }
    static FORCE_INLINE CONSTEXPR u64 HashUInt64(u64 key) NOEXCEPT { return FMix64(key); }

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

    static FORCE_INLINE CONSTEXPR u32 HashUInt32(u128 value) NOEXCEPT {
        return u32(HashUInt64(value)); // takes lower bits, should be alright
    }

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

    //----------------------------------------------------------------------------

    static FORCE_INLINE CONSTEXPR u32 HashUInt32(u256 value) NOEXCEPT {
        return HashCombine32(HashUInt32(value.lo), HashUInt32(value.hi));
    }

    static FORCE_INLINE CONSTEXPR u64 HashUInt64(u256 value) NOEXCEPT {
        return HashCombine64(HashUInt64(value.lo), HashUInt64(value.hi));
    }

    //----------------------------------------------------------------------------
#ifdef ARCH_X64
    static FORCE_INLINE CONSTEXPR u64 HashUInt(u32 value) NOEXCEPT { return HashUInt64(value); }
    static FORCE_INLINE CONSTEXPR u64 HashUInt(u64 value) NOEXCEPT { return HashUInt64(value); }
    static FORCE_INLINE CONSTEXPR u64 HashUInt(u128 value) NOEXCEPT { return HashUInt64(value); }
    static FORCE_INLINE CONSTEXPR u64 HashUInt(u256 value) NOEXCEPT { return HashUInt64(value); }
#else
    static FORCE_INLINE CONSTEXPR u32 HashUInt(u32 value) NOEXCEPT { return HashUInt32(value); }
    static FORCE_INLINE CONSTEXPR u32 HashUInt(u64 value) NOEXCEPT { return HashUInt32(value); }
    static FORCE_INLINE CONSTEXPR u32 HashUInt(u128 value) NOEXCEPT { return HashUInt32(value); }
    static FORCE_INLINE CONSTEXPR u32 HashUInt(u256 value) NOEXCEPT { return HashUInt32(value); }
#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
