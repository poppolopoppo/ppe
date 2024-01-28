// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Memory/HashFunctions.h"

//----------------------------------------------------------------------------
// xxHash for 128 bit hash functions (XXH3)
//----------------------------------------------------------------------------
#include "xxHash-external.h"

//----------------------------------------------------------------------------
// Farmhash for 32/64/128 bit fingerprint hash functions (stable)
//----------------------------------------------------------------------------
#include "farmhash-external.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Those functions are faster than Farmhash variants, but not guaranteed as stable
//----------------------------------------------------------------------------
u128 hash_128(const void* key, const size_t len, u64 seed/* = 0 */) NOEXCEPT {
    auto[lo,hi] = XXH3_128bits_withSeed(key, len, seed);
    return { lo, hi };
}
//----------------------------------------------------------------------------
u128 hash_128(const void* key, const size_t len, FRawMemoryConst secret) NOEXCEPT {
    auto[lo,hi] = XXH3_128bits_withSecret(key, len, secret.data(), secret.SizeInBytes());
    return { lo, hi };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Choose those functions when you want a stable hash across platforms.
// The implementation here should change only for very good reasons.
//----------------------------------------------------------------------------
u128 Fingerprint128(const void *ptr, size_t sizeInBytes) NOEXCEPT {
    FarmHash::uint128_t h;
    h = FarmHash::Fingerprint128(reinterpret_cast<const char *>(ptr), sizeInBytes);
    return *reinterpret_cast<const u128 *>(&h);
}
//----------------------------------------------------------------------------
u128 Fingerprint128(const void *ptr, size_t sizeInBytes, u128 seed) NOEXCEPT {
    FarmHash::uint128_t h;
    h = FarmHash::Hash128WithSeed(reinterpret_cast<const char *>(ptr), sizeInBytes, { seed.lo, seed.hi });
    return *reinterpret_cast<const u128 *>(&h);
}
//----------------------------------------------------------------------------
u64 Fingerprint64(const void *ptr, size_t sizeInBytes) NOEXCEPT {
    return FarmHash::Fingerprint64(reinterpret_cast<const char *>(ptr), sizeInBytes);
}
//----------------------------------------------------------------------------
u32 Fingerprint32(const void *ptr, size_t sizeInBytes) NOEXCEPT {
    return FarmHash::Fingerprint32(reinterpret_cast<const char *>(ptr), sizeInBytes);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
