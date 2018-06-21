#pragma once

#include "Core/Core.h"

#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Memory/MemoryView.h"

#include <atomic>

// Pseudo-Random Number Generator
// http://xorshift.di.unimi.it/

namespace Core {
namespace Random {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct CORE_API FXorShift64Star {

    u64 X;

    void Reset(u64 seed);
    u64  NextU64();
};
//----------------------------------------------------------------------------
struct CORE_API FXorShift128Plus {

    u64 States[2];

    void Reset(u64 seed);
    u64  NextU64();
};
//----------------------------------------------------------------------------
struct CORE_API FXorShift1024Star {

    u64 N;
    u64 States[16];

    void Reset(u64 seed);
    u64  NextU64();
};
//----------------------------------------------------------------------------
struct CORE_API FAtomicXorShift64Star {

    std::atomic<u64> X;

    void Reset(u64 seed);
    u64  NextU64();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Generator>
class TRng {
public:
    typedef _Generator generator_type;

    TRng();
    TRng(u64 seed);

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

    // Compatibility with STL : callable like a function
    size_t operator ()(size_t vmax) { return Next(vmax); }

    float NextFloat01();
    float NextFloatM11();

    template <typename T, class = typename std::enable_if< std::is_integral<T>::value >::type >
    void Randomize(T& i) { i = static_cast<T>(Next()); }
    void Randomize(bool& b) { b = (1 == (Next() & 1)); }
    void Randomize(float& f) { f = NextFloatM11(); }
    void Randomize(double& d) { d = static_cast<double>(NextFloatM11()); }

    template <typename T, size_t _Dim>
    void Randomize(TScalarVector<T, _Dim>& v) {
        for (size_t i = 0; i < _Dim; ++i)
            Randomize(v[i]);
    }

    template <typename T>
    void Randomize(const TMemoryView<T>& view) {
        for (T& elt : view)
            Randomize(elt);
    }

    template <typename T>
    T& RandomElement(const TMemoryView<T>& view) {
        Assert(not view.empty());
        const size_t n = size_t(Next() % view.size());
        return view[n];
    }

private:
    generator_type _generator;
};
//----------------------------------------------------------------------------
typedef TRng< FXorShift1024Star >       FStableRng;
typedef TRng< FXorShift128Plus >        FFastRng;
typedef TRng< FAtomicXorShift64Star >   FAtomicRng;
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
// using a struct instead of typedef to be able to fwd declare FRandomGenerator
INSTANTIATE_CLASS_TYPEDEF(CORE_API, FRandomGenerator, ::Core::Random::FStableRng);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/RandomGenerator-inl.h"
