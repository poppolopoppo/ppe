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
inline constexpr u64 hash_uint64(u64 value) {
    const u64 kMul = 0x9ddfea08eb382d69ULL;
    u64 b = value * kMul;
    b ^= (b >> 44);
    b *= kMul;
    b ^= (b >> 41);
    b *= kMul;
    return b;
}
//----------------------------------------------------------------------------
// Murmur-inspired hashing: https://github.com/google/farmhash/blob/master/dev/farmhash.h
FORCE_INLINE constexpr u64 hash_uint64(u128 value) {
    const u64 kMul = 0x9ddfea08eb382d69ULL;
    u64 a = (value.lo ^ value.hi) * kMul;
    a ^= (a >> 47);
    u64 b = (value.hi ^ a) * kMul;
    b ^= (b >> 44);
    b *= kMul;
    b ^= (b >> 41);
    b *= kMul;
    return b;
}
//----------------------------------------------------------------------------
// Burtle, Full avalanche: http://burtleburtle.net/bob/hash/integer.html
inline constexpr u32 hash_uint32(u32 a) {
    a = (a+0x7ed55d16) + (a<<12);
    a = (a^0xc761c23c) ^ (a>>19);
    a = (a+0x165667b1) + (a<<5);
    a = (a+0xd3a2646c) ^ (a<<9);
    a = (a+0xfd7046c5) + (a<<3);
    a = (a^0xb55a4f09) ^ (a>>16);
    return a;
}
//----------------------------------------------------------------------------
// Thomas Wang's 64 bit Mix Function: http://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
inline constexpr u32 hash_uint32(u64 key) {
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
FORCE_INLINE constexpr u32 hash_uint32(u128 value) {
    return u32(hash_uint64(value)); // takes lower bits, should be alright
}
//----------------------------------------------------------------------------
#ifdef ARCH_X64
FORCE_INLINE constexpr u64 hash_uint(u32 value) { return hash_uint64(u64(value)); }
FORCE_INLINE constexpr u64 hash_uint(u64 value) { return hash_uint64(value); }
FORCE_INLINE constexpr u64 hash_uint(u128 value){ return hash_uint64(value); }
#else
FORCE_INLINE constexpr u32 hash_uint(u32 value) { return hash_uint32(value); }
FORCE_INLINE constexpr u32 hash_uint(u64 value) { return hash_uint32(value); }
FORCE_INLINE constexpr u32 hash_uint(u128 value){ return hash_uint32(value); }
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
FORCE_INLINE size_t hash_ptr(const void* ptr) { return hash_uint(uintptr_t(ptr)); }
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
template <typename _It>
u32 hash_fnv1a32(_It first, _It last, u32 seed = 2166136261U) {
    u32 _Val = seed;
    for (; first != last; ++first) {
        _Val ^= (u32)*first;
        _Val *= 16777619U;
    }
    return (_Val);
}
//----------------------------------------------------------------------------
template <typename _It>
u64 hash_fnv1a64(_It first, _It last, u64 seed = 14695981039346656037ULL) {
    u64 _Val = seed;
    for (; first != last; ++first) {
        _Val ^= (u64)*first;
        _Val *= 1099511628211ULL;
    }
    return (_Val);
}
//----------------------------------------------------------------------------
#if defined(ARCH_X64)
template <typename _It> size_t hash_fnv1a(_It first, _It last) { return hash_fnv1a64(first, last); }
template <typename _It> size_t hash_fnv1a(_It first, _It last, size_t seed) { return hash_fnv1a64(first, last, seed); }
#else
template <typename _It> size_t hash_fnv1a(_It first, _It last) { return hash_fnv1a32(first, last); }
template <typename _It> size_t hash_fnv1a(_It first, _It last, size_t seed) { return hash_fnv1a32(first, last, seed); }
#endif
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
