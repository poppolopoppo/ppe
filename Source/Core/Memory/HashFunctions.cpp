#include "stdafx.h"

#include "HashFunctions.h"

#include "External/farmhash.h"

#define WITH_CORE_FARMHASH 1

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const u64 _FNV_offset_basis_64 = 14695981039346656037ULL;
const u64 _FNV_prime_64 = 1099511628211ULL;
//----------------------------------------------------------------------------
const u32 _FNV_offset_basis_32 = 2166136261U;
const u32 _FNV_prime_32 = 16777619U;
//----------------------------------------------------------------------------
#if defined(ARCH_X64)
static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
const size_t _FNV_offset_basis = _FNV_offset_basis_64;
const size_t _FNV_prime = _FNV_prime_64;
 #else
static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
const size_t _FNV_offset_basis = _FNV_offset_basis_32;
const size_t _FNV_prime = _FNV_prime_32;
#endif
//----------------------------------------------------------------------------
u32 hash_mem_fnv1a32(const void *ptr, size_t sizeInBytes, u32 seed = _FNV_offset_basis_32) {
    const unsigned char *_First = (const unsigned char *)ptr;
    const size_t _Count = sizeInBytes;

    u32 _Val = seed;
    for (size_t _Next = 0; _Next < _Count; ++_Next) {
        // fold in another byte
        _Val ^= (u32)_First[_Next];
        _Val *= _FNV_prime_32;
    }

    return (_Val);
}
//----------------------------------------------------------------------------
u64 hash_mem_fnv1a64(const void *ptr, size_t sizeInBytes, u64 seed = _FNV_offset_basis_64) {
    const unsigned char *_First = (const unsigned char *)ptr;
    const size_t _Count = sizeInBytes;

    u64 _Val = seed;
    for (size_t _Next = 0; _Next < _Count; ++_Next) {
        // fold in another byte
        _Val ^= (u64)_First[_Next];
        _Val *= _FNV_prime_64;
    }

    return (_Val);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u32 hash_mem32(const void *ptr, size_t sizeInBytes) {
    return hash_mem32(ptr, sizeInBytes, _FNV_offset_basis_32);
}
//----------------------------------------------------------------------------
u32 hash_mem32(const void *ptr, size_t sizeInBytes, u32 seed) {
#if WITH_CORE_FARMHASH
    return FarmHash::Hash32WithSeed(static_cast<const char *>(ptr), sizeInBytes, seed);
#else
    return hash_mem_fnv1a32(ptr, sizeInBytes, seed);;
#endif
}
//----------------------------------------------------------------------------
u64 hash_mem64(const void *ptr, size_t sizeInBytes) {
    return hash_mem64(ptr, sizeInBytes, _FNV_offset_basis_64);
}
//----------------------------------------------------------------------------
u64 hash_mem64(const void *ptr, size_t sizeInBytes, u64 seed) {
#if WITH_CORE_FARMHASH
    return FarmHash::Hash64WithSeed(static_cast<const char *>(ptr), sizeInBytes, seed);
#else
    return hash_mem_fnv1a64(ptr, sizeInBytes, seed);;
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t hash_mem(const void *ptr, size_t sizeInBytes) {
    return hash_mem(ptr, sizeInBytes, _FNV_offset_basis);
}
//----------------------------------------------------------------------------
size_t hash_mem(const void *ptr, size_t sizeInBytes, size_t seed) {
#ifdef ARCH_X64
    STATIC_ASSERT(sizeof(uint64_t) == sizeof(size_t));
    return hash_mem64(ptr, sizeInBytes, seed);
#else
    STATIC_ASSERT(sizeof(uint32_t) == sizeof(size_t));
    return hash_mem32(ptr, sizeInBytes, seed);
#endif
}

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u128 Fingerprint128(const void *ptr, size_t sizeInBytes) {
    FarmHash::uint128_t h;
    h = FarmHash::Fingerprint128(reinterpret_cast<const char *>(ptr), sizeInBytes);
    return *reinterpret_cast<const u128 *>(&h);
}
//----------------------------------------------------------------------------
u64 Fingerprint64(const void *ptr, size_t sizeInBytes) {
    return FarmHash::Fingerprint64(reinterpret_cast<const char *>(ptr), sizeInBytes);;
}
//----------------------------------------------------------------------------
u32 Fingerprint32(const void *ptr, size_t sizeInBytes) {
    return FarmHash::Fingerprint32(reinterpret_cast<const char *>(ptr), sizeInBytes);;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
}