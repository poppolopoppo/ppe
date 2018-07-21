#include "stdafx.h"

#include "HashFunctions.h"

//----------------------------------------------------------------------------
// Farmhash for 32/64/128 bit fingerprint hash functions (stable)
//----------------------------------------------------------------------------
#include "Core.External/farmhash-external.h"

namespace Core {
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
