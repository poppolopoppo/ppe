#pragma once

#include "Core/Core.h"

// Pseudo-Random Number Generator
// http://xorshift.di.unimi.it/

namespace Core {
namespace Random {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct XorShift64Star {

    u64 X;

    void Reset(size_t seed);
    u64  NextU64();
};
//----------------------------------------------------------------------------
struct XorShift128Plus {

    u64 States[2];

    void Reset(size_t seed);
    u64  NextU64();
};
//----------------------------------------------------------------------------
struct XorShift1024Star {

    u64 N;
    u64 States[16];

    void Reset(size_t seed);
    u64  NextU64();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Generator>
class Rng {
public:
    typedef _Generator generator_type;

    Rng();
    Rng(u64 seed);

    void Reset(u64 seed);

    u64 NextU64();
    u64 NextU64(u64 vmax);
    u64 NextU64(u64 vmin, u64 vmax);

    u32 NextU32();
    u32 NextU32(u32 vmax);
    u32 NextU32(u32 vmin, u32 vmax);

#ifdef ARCH_X64
    size_t Next() { return NextU64(); }
    size_t Next(size_t vmax) { return NextU64(vmax); }
    size_t Next(size_t vmin, size_t vmax) { return NextU64(vmin, vmax); }
#else
    size_t Next() { return NextU32(); }
    size_t Next(size_t vmax) { return NextU32(vmax); }
    size_t Next(size_t vmin, size_t vmax) { return NextU32(vmin, vmax); }
#endif

    float NextFloat01();
    float NextFloatM11();

    template <typename T, class = typename std::enable_if< std::is_integral<T>::value >::type >
    void Randomize(T& i) { i = static_cast<T>(Next()); }
    void Randomize(bool& b) { b = (1 == (Next() & 1)); }
    void Randomize(float& f) { f = NextFloatM11(); }
    void Randomize(double& d) { d = static_cast<double>(NextFloatM11()); }

private:
    generator_type _generator;
};
//----------------------------------------------------------------------------
typedef Rng< XorShift1024Star > StableRng;
typedef Rng< XorShift128Plus >  FastRng;
//----------------------------------------------------------------------------
u64 MakeSeed(u64 salt = 0);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Random
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// using a struct instead of typedef to be able to fwd declare RandomGenerator
INSTANTIATE_CLASS_TYPEDEF(RandomGenerator, ::Core::Random::StableRng);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/RandomGenerator-inl.h"
