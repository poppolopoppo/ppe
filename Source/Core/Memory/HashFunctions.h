#pragma once

#include "Core/Core.h"

#include <type_traits>

namespace Core {
template <typename T>
class TMemoryView;
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
size_t hash_ptr(const void* ptr);
//----------------------------------------------------------------------------
template <typename T>
size_t hash_as_pod(T&& pod);
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
