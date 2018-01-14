#include "stdafx.h"

#include "HashFunctions.h"

#define CORE_HASH_FNV1A     0
#define CORE_HASH_CRC32     1
#define CORE_HASH_XXHASH    2
#define CORE_HASH_FARMHASH  3

//#define CORE_HASH_FUNCTION  CORE_HASH_CRC32 // use CRC32 (SSE4.2) instead of xxHASH (simply faster)
#define CORE_HASH_FUNCTION  CORE_HASH_XXHASH  // use XXHASH since its support for SSE4.2 was enabled

//----------------------------------------------------------------------------
// Farmhash for 32/64/128 bit fingerprint hash functions (stable)
//----------------------------------------------------------------------------
#include "Core.External/farmhash-external.h"

//----------------------------------------------------------------------------
// xxHash for fast non crypto 32/64 bits hash functions (may varry accros platforms/versions)
//----------------------------------------------------------------------------
#include "Core.External/xxHash-external.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u32 hash_mem32(const void *ptr, size_t sizeInBytes) {
    return hash_mem32(ptr, sizeInBytes, CORE_HASH_VALUE_SEED_32);
}
//----------------------------------------------------------------------------
u32 hash_mem32(const void *ptr, size_t sizeInBytes, u32 seed) {
#if     CORE_HASH_FUNCTION == CORE_HASH_FNV1A
    return hash_fnv1a32(ptr, sizeInBytes, seed);
#elif   CORE_HASH_FUNCTION == CORE_HASH_CRC32
    return u32(hash_crc32(ptr, sizeInBytes, seed));
#elif   CORE_HASH_FUNCTION == CORE_HASH_XXHASH
    return XXH32(ptr, sizeInBytes, seed);
#elif   CORE_HASH_FUNCTION == CORE_HASH_FARMHASH
    return FarmHash::Hash32WithSeed(static_cast<const char *>(ptr), sizeInBytes, seed);
#else
#   error "unimplemented hash function"
#endif
}
//----------------------------------------------------------------------------
u64 hash_mem64(const void *ptr, size_t sizeInBytes) {
    return hash_mem64(ptr, sizeInBytes, CORE_HASH_VALUE_SEED_64);
}
//----------------------------------------------------------------------------
u64 hash_mem64(const void *ptr, size_t sizeInBytes, u64 seed) {
#if     CORE_HASH_FUNCTION == CORE_HASH_FNV1A
    return hash_fnv1a64(ptr, sizeInBytes, seed);
#elif   CORE_HASH_FUNCTION == CORE_HASH_CRC32
    return hash_fmix64(hash_crc32(ptr, sizeInBytes, u32(seed)));
#elif   CORE_HASH_FUNCTION == CORE_HASH_XXHASH
    return XXH64(ptr, sizeInBytes, seed);
#elif   CORE_HASH_FUNCTION == CORE_HASH_FARMHASH
    return FarmHash::Hash64WithSeed(static_cast<const char *>(ptr), sizeInBytes, seed);
#else
#   error "unimplemented hash function"
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t hash_mem(const void *ptr, size_t sizeInBytes) {
#ifdef ARCH_X64
    return hash_mem64(ptr, sizeInBytes, CORE_HASH_VALUE_SEED_64);
#else
    return hash_mem32(ptr, sizeInBytes, CORE_HASH_VALUE_SEED_32);
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
// Choose those functions when you want a stable hash across platforms.
// The implementation here should change only for very good reasons.
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
} //!namespace Core
