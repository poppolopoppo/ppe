#pragma once

#include "Core/Core.h"

#include <type_traits>

namespace Core {
template <typename T>
class MemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u32 hash_mem32(const void *ptr, size_t sizeInBytes);
template <typename T> u32 hash_mem32(const MemoryView<T>& src) { return hash_mem32(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
u32 hash_mem32(const void *ptr, size_t sizeInBytes, u32 seed);
template <typename T> u32 hash_mem32(const MemoryView<T>& src, u32 seed) { return hash_mem32(src.Pointer(), src.SizeInBytes(), seed); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u64 hash_mem64(const void *ptr, size_t sizeInBytes);
template <typename T> u64 hash_mem64(const MemoryView<T>& src) { return hash_mem64(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
u64 hash_mem64(const void *ptr, size_t sizeInBytes, u64 seed);
template <typename T> u64 hash_mem64(const MemoryView<T>& src, u64 seed) { return hash_mem64(src.Pointer(), src.SizeInBytes(), seed); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t hash_mem(const void *ptr, size_t sizeInBytes);
template <typename T> size_t hash_mem(const MemoryView<T>& src) { return hash_mem(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
size_t hash_mem(const void *ptr, size_t sizeInBytes, size_t seed);
template <typename T> size_t hash_mem(const MemoryView<T>& src, size_t seed) { return hash_mem(src.Pointer(), src.SizeInBytes(), seed); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
size_t hash_as_pod_array(const MemoryView<T>& src) { return hash_as_pod_array(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
template <typename _It>
size_t hash_as_pod_range(_It&& first, _It&& last);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u128 Fingerprint128(const void *ptr, size_t sizeInBytes);
template <typename T> u128 Fingerprint128(const MemoryView<T>& src) { return Fingerprint128(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
u64 Fingerprint64(const void *ptr, size_t sizeInBytes);
template <typename T> u64 Fingerprint64(const MemoryView<T>& src) { return Fingerprint64(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
u32 Fingerprint32(const void *ptr, size_t sizeInBytes);
template <typename T> u32 Fingerprint32(const MemoryView<T>& src) { return Fingerprint32(src.Pointer(), src.SizeInBytes()); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
}

#include "Core/Memory/HashFunctions-inl.h"
