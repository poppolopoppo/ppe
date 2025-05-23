#pragma once

#include "Core_fwd.h"

#include "HAL/PlatformHash.h"

namespace PPE {
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE CONSTEXPR CONSTF u64 hash_uint64(u32 b) NOEXCEPT { return FPlatformHash::HashUInt64(b); }
FORCE_INLINE CONSTEXPR CONSTF u64 hash_uint64(u64 b) NOEXCEPT { return FPlatformHash::HashUInt64(b); }
FORCE_INLINE CONSTEXPR CONSTF u64 hash_uint64(u128 b) NOEXCEPT { return FPlatformHash::HashUInt64(b); }
//----------------------------------------------------------------------------
FORCE_INLINE CONSTEXPR CONSTF u32 hash_uint32(u32 b) NOEXCEPT { return FPlatformHash::HashUInt32(b); }
FORCE_INLINE CONSTEXPR CONSTF u32 hash_uint32(u64 b) NOEXCEPT { return FPlatformHash::HashUInt32(b); }
FORCE_INLINE CONSTEXPR CONSTF u32 hash_uint32(u128 b) NOEXCEPT { return FPlatformHash::HashUInt32(b); }
FORCE_INLINE CONSTEXPR CONSTF u32 hash_uint32(u256 b) NOEXCEPT { return FPlatformHash::HashUInt32(b); }
//----------------------------------------------------------------------------
FORCE_INLINE CONSTEXPR CONSTF size_t hash_uint(u32 b) NOEXCEPT { return FPlatformHash::HashUInt(b); }
FORCE_INLINE CONSTEXPR CONSTF size_t hash_uint(u64 b) NOEXCEPT { return FPlatformHash::HashUInt(b); }
FORCE_INLINE CONSTEXPR CONSTF size_t hash_uint(u128 b) NOEXCEPT { return FPlatformHash::HashUInt(b); }
FORCE_INLINE CONSTEXPR CONSTF size_t hash_uint(u256 b) NOEXCEPT { return FPlatformHash::HashUInt(b); }
//----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche
FORCE_INLINE CONSTEXPR CONSTF u32 hash_fmix32(u32 k) NOEXCEPT { return FPlatformHash::FMix32(k); }
FORCE_INLINE CONSTEXPR CONSTF u64 hash_fmix64(u64 k) NOEXCEPT { return FPlatformHash::FMix64(k); }
//----------------------------------------------------------------------------
// Hw accelerated CRC32 with _mm_crc32_uXX()
FORCE_INLINE CONSTF size_t hash_crc32(u32 b) NOEXCEPT { return FPlatformHash::CRC32(b); }
FORCE_INLINE CONSTF size_t hash_crc32(u64 b) NOEXCEPT { return FPlatformHash::CRC32(b); }
FORCE_INLINE CONSTF size_t hash_crc32(u128 b) NOEXCEPT { return FPlatformHash::HashUInt(b); }
FORCE_INLINE CONSTF size_t hash_crc32(u256 b) NOEXCEPT { return FPlatformHash::HashUInt(b); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CONSTF u32 hash_mem32(const void *ptr, size_t sizeInBytes);
template <typename T>
CONSTF u32 hash_mem32(const TMemoryView<T>& src) NOEXCEPT { return hash_mem32(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
CONSTF u32 hash_mem32(const void *ptr, size_t sizeInBytes, u32 seed);
template <typename T>
CONSTF u32 hash_mem32(const TMemoryView<T>& src, u32 seed) NOEXCEPT { return hash_mem32(src.Pointer(), src.SizeInBytes(), seed); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CONSTF u64 hash_mem64(const void *ptr, size_t sizeInBytes);
template <typename T>
CONSTF u64 hash_mem64(const TMemoryView<T>& src) NOEXCEPT { return hash_mem64(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
CONSTF u64 hash_mem64(const void *ptr, size_t sizeInBytes, u64 seed);
template <typename T>
CONSTF u64 hash_mem64(const TMemoryView<T>& src, u64 seed) NOEXCEPT { return hash_mem64(src.Pointer(), src.SizeInBytes(), seed); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE CONSTF size_t hash_mem(const void *ptr, size_t sizeInBytes) NOEXCEPT { return CODE3264(hash_mem32, hash_mem64)(ptr, sizeInBytes); }
template <typename T>
CONSTF size_t hash_mem(const TMemoryView<T>& src) NOEXCEPT { return hash_mem(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
FORCE_INLINE CONSTF size_t hash_mem(const void *ptr, size_t sizeInBytes, size_t seed) { return CODE3264(hash_mem32, hash_mem64)(ptr, sizeInBytes, seed); }
template <typename T>
CONSTF size_t hash_mem(const TMemoryView<T>& src, size_t seed) NOEXCEPT { return hash_mem(src.Pointer(), src.SizeInBytes(), seed); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline CONSTF size_t hash_ptr(const void* ptr) NOEXCEPT { return hash_uint(uintptr_t(ptr)); }
//----------------------------------------------------------------------------
template <typename T>
CONSTF size_t hash_as_crc32(const T& pod) NOEXCEPT; // Hw accelerated with _mm_crc32_uXX()
//----------------------------------------------------------------------------
template <typename T>
CONSTF size_t hash_as_pod(const T& pod) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T>
CONSTF size_t hash_as_pod_array(const T *ptr, size_t count) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTF size_t hash_as_pod_array(const T (&staticArray)[_Dim]) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T>
CONSTF size_t hash_as_pod_array(const TMemoryView<T>& src) NOEXCEPT { return hash_as_pod_array(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
template <typename _It>
CONSTF size_t hash_as_pod_range(_It&& first, _It&& last) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE CONSTF u32 hash_fnv1a32(const void* key, const size_t sizeInBytes, u32 hash = 0) NOEXCEPT {
    return FPlatformHash::FNV1a32(hash, key, sizeInBytes);
}
//----------------------------------------------------------------------------
FORCE_INLINE CONSTF u64 hash_fnv1a64(const void* key, const size_t sizeInBytes, u64 hash = 0) NOEXCEPT {
    return FPlatformHash::FNV1a64(hash, key, sizeInBytes);
}
//----------------------------------------------------------------------------
FORCE_INLINE CONSTF size_t hash_fnv1a(const void* key, const size_t len, size_t hash = 0) NOEXCEPT {
    return CODE3264(hash_fnv1a32, hash_fnv1a64)(key, len, hash);
}
//----------------------------------------------------------------------------
FORCE_INLINE CONSTF size_t hash_crc32(const void* key, size_t sizeInBytes, size_t crc32 = 0) NOEXCEPT {
    return FPlatformHash::CRC32(crc32, key, sizeInBytes);
}
//----------------------------------------------------------------------------
PPE_CORE_API CONSTF u128 hash_128(const void* key, const size_t len, u64 seed = 0) NOEXCEPT;
PPE_CORE_API CONSTF u128 hash_128(const void* key, const size_t len, FRawMemoryConst secret) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API CONSTF u128 Fingerprint128(const void* ptr, size_t sizeInBytes) NOEXCEPT;
PPE_CORE_API CONSTF u128 Fingerprint128(const void *ptr, size_t sizeInBytes, u128 seed) NOEXCEPT;
template <typename T> CONSTF u128 Fingerprint128(const TMemoryView<T>& src) { return Fingerprint128(src.Pointer(), src.SizeInBytes()); }
template <typename T> CONSTF u128 Fingerprint128(const TMemoryView<T>& src, u128 seed) { return Fingerprint128(src.Pointer(), src.SizeInBytes(), seed); }
//----------------------------------------------------------------------------
PPE_CORE_API CONSTF u64 Fingerprint64(const void *ptr, size_t sizeInBytes) NOEXCEPT;
template <typename T> CONSTF u64 Fingerprint64(const TMemoryView<T>& src) { return Fingerprint64(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
PPE_CORE_API CONSTF u32 Fingerprint32(const void *ptr, size_t sizeInBytes) NOEXCEPT;
template <typename T> CONSTF u32 Fingerprint32(const TMemoryView<T>& src) { return Fingerprint32(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Memory/HashFunctions-inl.h"
