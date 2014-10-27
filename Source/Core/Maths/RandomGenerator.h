#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarVector_fwd.h"

// Pseudo-Random Number Generator

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class RandomGenerator{
public:
    static u32 RandomSeed(u32 salt/* = 0 */);
    struct RandomSeedTag {};

    RandomGenerator();
    RandomGenerator(RandomSeedTag);
    RandomGenerator(u32 seed);
    ~RandomGenerator();

    u32 Seed() const { return _seed; }

    void Reset(u32 seed);

    FORCE_INLINE u32 NextU32();
    FORCE_INLINE u32 NextU32(u32 vmax);
    FORCE_INLINE u32 NextU32(u32 vmin, u32 vmax);

    FORCE_INLINE float NextFloat01();
    FORCE_INLINE float NextFloatM11();

private:
    FORCE_INLINE static u32 NextSeed_(u32 seed);
    FORCE_INLINE static u32 NextState_(u32 state);

    u32 _state[4];
    u32 _seed;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class ScalarVector;
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u32, _Dim> NextRandU32(RandomGenerator& rand);
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u32, _Dim> NextRandU32(RandomGenerator& rand, u32 vmax);
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u32, _Dim> NextRandU32(RandomGenerator& rand, const ScalarVector<u32, _Dim>& vmax);
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u32, _Dim> NextRandU32(RandomGenerator& rand, u32 vmin, u32 vmax);
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u32, _Dim> NextRandU32(RandomGenerator& rand, const ScalarVector<u32, _Dim>& vmin, const ScalarVector<u32, _Dim>& vmax);
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> NextRandFloat01(RandomGenerator& rand);
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> NextRandFloatM11(RandomGenerator& rand);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/RandomGenerator-inl.h"
