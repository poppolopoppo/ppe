#pragma once

#include "Core/Core.h"

#include <type_traits>

namespace Core {
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Murmur-inspired hashing: https://github.com/google/farmhash/blob/master/dev/farmhash.h
inline CONSTEXPR u64 hash_uint64(u64 b) {
#if 1 // for constexpr
    b *= 0x9ddfea08eb382d69ULL;
    b ^= (b >> 44);
    b *= 0x9ddfea08eb382d69ULL;
    b ^= (b >> 41);
    b *= 0x9ddfea08eb382d69ULL;
#else
    constexpr u64 kMul = 0x9ddfea08eb382d69ULL;
    u64 b = value * kMul;
    b ^= (b >> 44);
    b *= kMul;
    b ^= (b >> 41);
    b *= kMul;
#endif
    return b;
}
//----------------------------------------------------------------------------
// Murmur-inspired hashing: https://github.com/google/farmhash/blob/master/dev/farmhash.h
inline CONSTEXPR u64 hash_uint64(u128 b) {
#if 1 // for constexpr
    b.lo  = (b.lo ^ b.hi) * 0x9ddfea08eb382d69ULL;
    b.lo ^= (b.lo >> 47);
    b.hi  = (b.hi ^ b.lo) * 0x9ddfea08eb382d69ULL;
    b.hi ^= (b.hi >> 44);
    b.hi *= 0x9ddfea08eb382d69ULL;
    b.hi ^= (b.hi >> 41);
    b.hi *= 0x9ddfea08eb382d69ULL;
    return b.hi;
#else
    constexpr u64 kMul = 0x9ddfea08eb382d69ULL;
    u64 a = (value.lo ^ value.hi) * kMul;
    a ^= (a >> 47);
    u64 b = (value.hi ^ a) * kMul;
    b ^= (b >> 44);
    b *= kMul;
    b ^= (b >> 41);
    b *= kMul;
    return b;
#endif
}
//----------------------------------------------------------------------------
// Burtle, Full avalanche: http://burtleburtle.net/bob/hash/integer.html
inline CONSTEXPR u32 hash_uint32(u32 a) {
    a = (a+0x7ed55d16) + (a<<12);
    a = (a^0xc761c23c) ^ (a>>19);
    a = (a+0x165667b1) + (a<<5 );
    a = (a+0xd3a2646c) ^ (a<<9 );
    a = (a+0xfd7046c5) + (a<<3 );
    a = (a^0xb55a4f09) ^ (a>>16);
    return a;
}
//----------------------------------------------------------------------------
// Thomas Wang's 64 bit Mix Function: http://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
inline CONSTEXPR u32 hash_uint32(u64 key) {
    key += ~(key << 32);
    key ^=  (key >> 22);
    key += ~(key << 13);
    key ^=  (key >> 8 );
    key +=  (key << 3 );
    key ^=  (key >> 15);
    key += ~(key << 27);
    key ^=  (key >> 31);
    return u32(key);
}
//----------------------------------------------------------------------------
FORCE_INLINE CONSTEXPR u32 hash_uint32(u128 value) {
    return u32(hash_uint64(value)); // takes lower bits, should be alright
}
//----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche
FORCE_INLINE CONSTEXPR u64 hash_fmix64(u64 k) {
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdull;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ull;
    k ^= k >> 33;
    return k;
}
//----------------------------------------------------------------------------
#ifdef ARCH_X64
FORCE_INLINE CONSTEXPR u64 hash_uint(u32 value) { return hash_uint64(u64(value)); }
FORCE_INLINE CONSTEXPR u64 hash_uint(u64 value) { return hash_uint64(value); }
FORCE_INLINE CONSTEXPR u64 hash_uint(u128 value){ return hash_uint64(value); }
#else
FORCE_INLINE CONSTEXPR u32 hash_uint(u32 value) { return hash_uint32(value); }
FORCE_INLINE CONSTEXPR u32 hash_uint(u64 value) { return hash_uint32(value); }
FORCE_INLINE CONSTEXPR u32 hash_uint(u128 value){ return hash_uint32(value); }
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u32 hash_mem32(const void *ptr, size_t sizeInBytes);
template <typename T> u32 hash_mem32(const TMemoryView<T>& src) { return hash_mem32(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
u32 hash_mem32(const void *ptr, size_t sizeInBytes, u32 seed);
template <typename T> u32 hash_mem32(const TMemoryView<T>& src, u32 seed) { return hash_mem32(src.Pointer(), src.SizeInBytes(), seed); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u64 hash_mem64(const void *ptr, size_t sizeInBytes);
template <typename T> u64 hash_mem64(const TMemoryView<T>& src) { return hash_mem64(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
u64 hash_mem64(const void *ptr, size_t sizeInBytes, u64 seed);
template <typename T> u64 hash_mem64(const TMemoryView<T>& src, u64 seed) { return hash_mem64(src.Pointer(), src.SizeInBytes(), seed); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t hash_mem(const void *ptr, size_t sizeInBytes);
template <typename T> size_t hash_mem(const TMemoryView<T>& src) { return hash_mem(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
size_t hash_mem(const void *ptr, size_t sizeInBytes, size_t seed);
template <typename T> size_t hash_mem(const TMemoryView<T>& src, size_t seed) { return hash_mem(src.Pointer(), src.SizeInBytes(), seed); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline size_t hash_ptr(const void* ptr) { return hash_uint(uintptr_t(ptr)); }
//----------------------------------------------------------------------------
template <typename T>
size_t hash_as_pod(const T& pod);
//----------------------------------------------------------------------------
template <typename T>
size_t hash_as_pod_array(const T *ptr, size_t count);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
size_t hash_as_pod_array(const T (&staticArray)[_Dim]);
//----------------------------------------------------------------------------
template <typename T>
size_t hash_as_pod_array(const TMemoryView<T>& src) { return hash_as_pod_array(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
template <typename _It>
size_t hash_as_pod_range(_It&& first, _It&& last);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline u32 hash_fnv1a32(const void* key, const size_t sizeInBytes, u32 hash = 0x811c9dc5ul) {
    auto data = (const u8*)key;
    for(size_t i = 0; i < sizeInBytes; ++i) {
        u8 value = data[i];
        hash = hash ^ value;
        hash *= 0x1000193ul;
    }
    return hash;
}
//----------------------------------------------------------------------------
inline u64 hash_fnv1a64(const void* key, const size_t sizeInBytes, u64 hash = 0xcbf29ce484222325ull) {
    auto data = (const u8*)key;
    for(size_t i = 0; i < sizeInBytes; ++i) {
        u8 value = data[i];
        hash = hash ^ value;
        hash *= 0x100000001b3ull;
    }
    return hash;
}
//----------------------------------------------------------------------------
FORCE_INLINE size_t hash_fnv1a(const void* key, const size_t len, size_t hash = CODE3264(0x811c9dc5ul, 0xcbf29ce484222325ull)) {
    return CODE3264(hash_fnv1a32, hash_fnv1a64)(key, len, hash);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE size_t hash_crc32(const void* key, size_t sizeInBytes, size_t crc32 = 0) {
    const size_t fast = sizeInBytes / sizeof(intptr_t);
    size_t offset = fast * sizeof(intptr_t);
    size_t current_block = 0;

    crc32 ^= 0xffffffff;

    for (; current_block < fast; ++current_block)
        crc32 = CODE3264(_mm_crc32_u32, _mm_crc32_u64)(crc32, ((const CODE3264(u32, u64)*)key)[current_block]);

    for (; offset < sizeInBytes; ++offset)
        crc32 = _mm_crc32_u8(u32(crc32), ((const u8*)key)[offset]);

    return (crc32 ^ 0xffffffff);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u128 Fingerprint128(const void *ptr, size_t sizeInBytes);
template <typename T> u128 Fingerprint128(const TMemoryView<const T>& src) { return Fingerprint128(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
u64 Fingerprint64(const void *ptr, size_t sizeInBytes);
template <typename T> u64 Fingerprint64(const TMemoryView<const T>& src) { return Fingerprint64(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
u32 Fingerprint32(const void *ptr, size_t sizeInBytes);
template <typename T> u32 Fingerprint32(const TMemoryView<const T>& src) { return Fingerprint32(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Memory/HashFunctions-inl.h"
