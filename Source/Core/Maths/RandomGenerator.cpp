#include "stdafx.h"

#include "RandomGenerator.h"

#include "Container/Hash.h"

#include <chrono> // seed salt

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RandomGenerator::RandomGenerator()
:   RandomGenerator(0) {}
//----------------------------------------------------------------------------
RandomGenerator::RandomGenerator(RandomSeedTag)
:   RandomGenerator(RandomSeed(reinterpret_cast<u32>(this))) {}
//----------------------------------------------------------------------------
RandomGenerator::RandomGenerator(u32 seed) {
    Reset(seed);
}
//----------------------------------------------------------------------------
RandomGenerator::~RandomGenerator() {}
//----------------------------------------------------------------------------
void RandomGenerator::Reset(u32 seed) {
    _seed = seed;

    _state[0] = NextSeed_(seed);
    _state[1] = NextSeed_(_state[0]);
    _state[2] = NextSeed_(_state[1]);
    _state[3] = NextSeed_(_state[2]);

    // warm-up, else return _seed on first 4 call :
    NextU32(); NextU32(); NextU32(); NextU32();
}
//----------------------------------------------------------------------------
u32 RandomGenerator::RandomSeed(u32 salt/* = 0 */) {
   const i64 t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
   return static_cast<u32>(hash_value(t, salt));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
