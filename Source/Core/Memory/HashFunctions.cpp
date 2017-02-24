#include "stdafx.h"

#include "HashFunctions.h"

#include "External/farmhash.h"

#define WITH_CORE_FARMHASH 1

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u32 hash_mem32(const void *ptr, size_t sizeInBytes) {
    return hash_mem32(ptr, sizeInBytes, 0xdeadbeeful);
}
//----------------------------------------------------------------------------
u32 hash_mem32(const void *ptr, size_t sizeInBytes, u32 seed) {
#if WITH_CORE_FARMHASH
    return FarmHash::Hash32WithSeed(static_cast<const char *>(ptr), sizeInBytes, seed);
#else
    return hash_fnv1a32((const u8*)ptr, (const u8*)ptr+sizeInBytes, seed);
#endif
}
//----------------------------------------------------------------------------
u64 hash_mem64(const void *ptr, size_t sizeInBytes) {
    return hash_mem64(ptr, sizeInBytes, 0xdeadbeefabadcafeull);
}
//----------------------------------------------------------------------------
u64 hash_mem64(const void *ptr, size_t sizeInBytes, u64 seed) {
#if WITH_CORE_FARMHASH
    return FarmHash::Hash64WithSeed(static_cast<const char *>(ptr), sizeInBytes, seed);
#else
    return hash_fnv1a64((const u8*)ptr, (const u8*)ptr+sizeInBytes, seed);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t hash_mem(const void *ptr, size_t sizeInBytes) {
#ifdef ARCH_X64
    return hash_mem(ptr, sizeInBytes, size_t(0xdeadbeefabadcafeull));
#else
    return hash_mem(ptr, sizeInBytes, size_t(0xabadcafeull));
#endif
}
//----------------------------------------------------------------------------
size_t hash_mem(const void *ptr, size_t sizeInBytes, size_t seed) {
#ifdef ARCH_X64
    return hash_mem64(ptr, sizeInBytes, seed);
#else
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
    return FarmHash::Fingerprint64(reinterpret_cast<const char *>(ptr), sizeInBytes);
}
//----------------------------------------------------------------------------
u32 Fingerprint32(const void *ptr, size_t sizeInBytes) {
    return FarmHash::Fingerprint32(reinterpret_cast<const char *>(ptr), sizeInBytes);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
}
