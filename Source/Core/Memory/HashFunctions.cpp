#include "stdafx.h"

#include "HashFunctions.h"

#define WITH_CORE_HASH_FNV1A 0

#ifdef PLATFORM_WINDOWS
#   pragma warning(push)
#   pragma warning(disable: 6297) // Arithmetic overflow:  32-bit value is shifted, then cast to 64-bit value.  Results might not be an expected value.
#endif

#include "External/farmhash.h"

#ifdef PLATFORM_WINDOWS
#   pragma warning(pop)
#endif

#pragma warning(push)
#pragma push_macro("FORCE_INLINE")
#undef FORCE_INLINE
#define XXH_PRIVATE_API
extern "C" {
#include "External/xxhash.h"
} //!extern "C"
#pragma pop_macro("FORCE_INLINE")
#pragma warning(pop)

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u32 hash_mem32(const void *ptr, size_t sizeInBytes) {
    return hash_mem32(ptr, sizeInBytes, 0xdeadbeeful);
}
//----------------------------------------------------------------------------
u32 hash_mem32(const void *ptr, size_t sizeInBytes, u32 seed) {
#if WITH_CORE_HASH_FNV1A
    return hash_fnv1a32((const u8*)ptr, (const u8*)ptr+sizeInBytes, seed);
#else
    //return FarmHash::Hash32WithSeed(static_cast<const char *>(ptr), sizeInBytes, seed);
    return XXH32(ptr, sizeInBytes, seed);
#endif
}
//----------------------------------------------------------------------------
u64 hash_mem64(const void *ptr, size_t sizeInBytes) {
    return hash_mem64(ptr, sizeInBytes, 0xdeadbeefabadcafeull);
}
//----------------------------------------------------------------------------
u64 hash_mem64(const void *ptr, size_t sizeInBytes, u64 seed) {
#if WITH_CORE_HASH_FNV1A
    return hash_fnv1a64((const u8*)ptr, (const u8*)ptr + sizeInBytes, seed);
#else
    //return FarmHash::Hash64WithSeed(static_cast<const char *>(ptr), sizeInBytes, seed);
    return XXH64(ptr, sizeInBytes, seed);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t hash_mem(const void *ptr, size_t sizeInBytes) {
#ifdef ARCH_X64
    return hash_mem64(ptr, sizeInBytes, size_t(0xdeadbeefabadcafeull));
#else
    return hash_mem32(ptr, sizeInBytes, size_t(0xabadcafeull));
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
