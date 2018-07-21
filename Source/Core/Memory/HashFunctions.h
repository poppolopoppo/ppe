#pragma once

#include "Core/Core.h"

#include "Core/HAL/PlatformHash.h"

namespace Core {
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE CONSTEXPR u64 hash_uint64(u64 b) { return FPlatformHash::HashUInt64(b); }
FORCE_INLINE CONSTEXPR u64 hash_uint64(u128 b) { return FPlatformHash::HashUInt64(b); }
FORCE_INLINE CONSTEXPR u32 hash_uint32(u32 b) { return FPlatformHash::HashUInt32(b); }
FORCE_INLINE CONSTEXPR u32 hash_uint32(u64 b) { return FPlatformHash::HashUInt32(b); }
FORCE_INLINE CONSTEXPR u32 hash_uint32(u128 b) { return FPlatformHash::HashUInt32(b); }
//----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche
FORCE_INLINE CONSTEXPR u64 hash_fmix64(u64 k) { return FPlatformHash::FMix64(k); }
//----------------------------------------------------------------------------
FORCE_INLINE CONSTEXPR size_t hash_uint(u32 b) { return FPlatformHash::HashUInt(b); }
FORCE_INLINE CONSTEXPR size_t hash_uint(u64 b) { return FPlatformHash::HashUInt(b); }
FORCE_INLINE CONSTEXPR size_t hash_uint(u128 b) { return FPlatformHash::HashUInt(b); }
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
FORCE_INLINE size_t hash_mem(const void *ptr, size_t sizeInBytes) { return CODE3264(hash_mem32, hash_mem64)(ptr, sizeInBytes); }
template <typename T> size_t hash_mem(const TMemoryView<T>& src) { return hash_mem(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
FORCE_INLINE size_t hash_mem(const void *ptr, size_t sizeInBytes, size_t seed) { return CODE3264(hash_mem32, hash_mem64)(ptr, sizeInBytes, seed); }
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
FORCE_INLINE u32 hash_fnv1a32(const void* key, const size_t sizeInBytes, u32 hash = 0) {
    return FPlatformHash::FNV1a32(hash, key, sizeInBytes);
}
//----------------------------------------------------------------------------
FORCE_INLINE u64 hash_fnv1a64(const void* key, const size_t sizeInBytes, u64 hash = 0) {
    return FPlatformHash::FNV1a64(hash, key, sizeInBytes);
}
//----------------------------------------------------------------------------
FORCE_INLINE size_t hash_fnv1a(const void* key, const size_t len, size_t hash = 0) {
    return CODE3264(hash_fnv1a32, hash_fnv1a64)(key, len, hash);
}
//----------------------------------------------------------------------------
FORCE_INLINE size_t hash_crc32(const void* key, size_t sizeInBytes, size_t crc32 = 0) {
    return FPlatformHash::CRC32(crc32, key, sizeInBytes);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_API u128 Fingerprint128(const void *ptr, size_t sizeInBytes);
template <typename T> u128 Fingerprint128(const TMemoryView<const T>& src) { return Fingerprint128(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
CORE_API u64 Fingerprint64(const void *ptr, size_t sizeInBytes);
template <typename T> u64 Fingerprint64(const TMemoryView<const T>& src) { return Fingerprint64(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
CORE_API u32 Fingerprint32(const void *ptr, size_t sizeInBytes);
template <typename T> u32 Fingerprint32(const TMemoryView<const T>& src) { return Fingerprint32(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Memory/HashFunctions-inl.h"
