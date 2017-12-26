#pragma once

#include "Core/Maths/RandomGenerator.h"

namespace Core {
namespace Random {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Generator>
TRng<_Generator>::TRng() : TRng(MakeSeed(0xDEADBEEFABADCAFEull)) {}
//----------------------------------------------------------------------------
template <typename _Generator>
TRng<_Generator>::TRng(u64 seed) {
    _generator.Reset(seed);
}
//----------------------------------------------------------------------------
template <typename _Generator>
void TRng<_Generator>::Reset(u64 seed) {
    _generator.Reset(seed);
}
//----------------------------------------------------------------------------
template <typename _Generator>
u64 TRng<_Generator>::NextU64() {
    return _generator.NextU64();
}
//----------------------------------------------------------------------------
template <typename _Generator>
u64 TRng<_Generator>::NextU64(u64 vmax) {
    return _generator.NextU64() % vmax;
}
//----------------------------------------------------------------------------
template <typename _Generator>
u64 TRng<_Generator>::NextU64(u64 vmin, u64 vmax) {
    Assert(vmin < vmax);
    return NextU64(vmax - vmin) + vmin;
}
//----------------------------------------------------------------------------
template <typename _Generator>
u32 TRng<_Generator>::NextU32() {
    return u32(_generator.NextU64());
}
//----------------------------------------------------------------------------
template <typename _Generator>
u32 TRng<_Generator>::NextU32(u32 vmax) {
    return _generator.NextU64() % vmax;
}
//----------------------------------------------------------------------------
template <typename _Generator>
u32 TRng<_Generator>::NextU32(u32 vmin, u32 vmax) {
    Assert(vmin < vmax);
    return NextU32(vmax - vmin) + vmin;
}
//----------------------------------------------------------------------------
template <typename _Generator>
float TRng<_Generator>::NextFloat01() {
    // Pour rappel, la representation d'un float IEEE 754-2008 est :
    //   [31] : s, [30-23] : e, [22-0] : m
    // dont la valeur est : (-1)^s . (1 + m / 2^23) . 2^(e-127)
    // Afin de tirer un nombre aleatoire *uniforme* entre 0 et 1, on va d'abord calculer un nombre entre 1 et 2 (2 exclu)
    // en tirant aleatoirement les bits de la mantisse, mais en fixant l'exposant a 127 et le bit de signe a 0.
    // Note : 0x3f800000 est la representation hexadecimale de 1.f
    //        0x007fffff est le masque de la mantisse.

    const u32 r = u32(_generator.NextU64());
    const u32 iResult = 0x3f800000 | (r & 0x007fffff);

    const char* pChar = (const char*)&iResult; // no type aliasing
    float result = *(const float*)pChar;
    Assert(result >= 1.f);
    Assert(result < 2.f);

    return result - 1.f;
}
//----------------------------------------------------------------------------
template <typename _Generator>
float TRng<_Generator>::NextFloatM11() {
    // On peut optimiser le calcul en tirant un nombre dans [2, 4[ et en supprimant 3, comme dans UnitRand().
    // Note : 0x40000000 est la representation hexadecimale de 2.f
    //        0x007fffff est le masque de la mantisse.

    const u32 r = u32(_generator.NextU64());
    const u32 iResult = 0x40000000 | (r & 0x007fffff);

    const char* pChar = (const char*)&iResult; // no type aliasing
    float result = *(const float*)pChar;
    Assert(result >= 2.f);
    Assert(result < 4.f);

    return result - 3.f;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Random
} //!namespace Core
