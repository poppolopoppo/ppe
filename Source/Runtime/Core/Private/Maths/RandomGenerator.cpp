// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Maths/RandomGenerator.h"

#include "CoreModule.h"
#include "Container/Hash.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "Misc/Guid.h"
#include "Modular/ModuleInfo.h"
#include "Time/Timestamp.h"

#include <chrono> // seed salt

namespace PPE {
namespace Random {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FXorShift64Star::Reset(u64 seed) {
    X = seed;
}
//----------------------------------------------------------------------------
u64 FXorShift64Star::NextU64() {
    X ^= X >> 12; // a
    X ^= X << 25; // b
    X ^= X >> 27; // c
    return X * 2685821657736338717LL;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FXorShift128Plus::Reset(u64 seed) {
    FXorShift64Star rng;
    rng.Reset(seed);
    States[0] = rng.NextU64();
    States[1] = rng.NextU64();
}
//----------------------------------------------------------------------------
u64 FXorShift128Plus::NextU64() {
    u64 s1 = States[ 0 ];
    const u64 s0 = States[ 1 ];
    States[ 0 ] = s0;
    s1 ^= s1 << 23; // a
    return ( States[ 1 ] = ( s1 ^ s0 ^ ( s1 >> 17 ) ^ ( s0 >> 26 ) ) ) + s0; // b, c
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FXorShift1024Star::Reset(u64 seed) {
    FXorShift128Plus rng;
    rng.Reset(seed);
    N = (rng.NextU64() & 15);
    for (u64& s : States)
        s = rng.NextU64();
}
//----------------------------------------------------------------------------
u64 FXorShift1024Star::NextU64() {
    u64 s0 = States[ N ];
    u64 s1 = States[ N = (( N + 1 ) & 15) ];
    s1 ^= s1 << 31; // a
    s1 ^= s1 >> 11; // b
    s0 ^= s0 >> 30; // c
    return ( States[ N ] = s0 ^ s1 ) * 1181783497276652981LL;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u64 MakeSeed(u64 salt/* = 0 */) {
    static const FGuid GProcessGUID = FGuid::Generate();
    const struct {
        u64 Salt[4];
        FGuid Guid;
    }   seed{
        {
            salt,
            hash_tuple(FPlatformMisc::CPUInfo().Ordinal(), std::hash<std::thread::id>{}(std::this_thread::get_id())),
            hash_value(FPlatformTime::NetworkTime()),
            hash_value(FCoreModule::StaticInfo.BuildVersion.Timestamp.Value()),
        },
        GProcessGUID,
    };
    return Fingerprint64(&seed, sizeof(seed));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FAtomicXorShift64Star::Reset(u64 seed) {
    X = seed;
}
//----------------------------------------------------------------------------
u64 FAtomicXorShift64Star::NextU64() {
    for (u64 seed = X;;) {
        FXorShift64Star rng{ seed };
        const u64 result = rng.NextU64();

        if (X.compare_exchange_weak(seed, rng.X))
            return result;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Random
} //!namespace PPE
