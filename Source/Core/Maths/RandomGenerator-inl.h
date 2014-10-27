#pragma once

#include "Core/Maths/RandomGenerator.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u32 RandomGenerator::NextU32() {
    const u32 state0 = _state[0];
    const u32 nextState0 = NextState_(state0);

    _state[0] = _state[1];
    _state[1] = _state[2];
    _state[2] = _state[3];
    _state[3] = nextState0;

    return state0;
}
//----------------------------------------------------------------------------
u32 RandomGenerator::NextU32(u32 vmax) {
    return NextU32() % vmax;
}
//----------------------------------------------------------------------------
u32 RandomGenerator::NextU32(u32 vmin, u32 vmax) {
    Assert(vmin < vmax);
    return NextU32(vmax - vmin) + vmin;
}
//----------------------------------------------------------------------------
float RandomGenerator::NextFloat01() {
    // Pour rappel, la representation d'un float IEEE 754-2008 est :
    //   [31] : s, [30-23] : e, [22-0] : m
    // dont la valeur est : (-1)^s . (1 + m / 2^23) . 2^(e-127)
    // Afin de tirer un nombre aléatoire *uniforme* entre 0 et 1, on va d'abord calculer un nombre entre 1 et 2 (2 exclu)
    // en tirant aléatoirement les bits de la mantisse, mais en fixant l'exposant à 127 et le bit de signe à 0.
    // Note : 0x3f800000 est la représentation hexadecimale de 1.f
    //        0x007fffff est le masque de la mantisse.
    // NDP: malin, n'est ce pas ?

    const u32 r = NextU32();
    const u32 iResult = 0x3f800000 | (r & 0x007fffff);

    const char* pChar = (const char*)&iResult; // no type aliasing
    float result = *(const float*)pChar;
    Assert(result >= 1.f);
    Assert(result < 2.f);

    return result - 1.f;
}
//----------------------------------------------------------------------------
float RandomGenerator::NextFloatM11() {
    // On peut optimiser le calcul en tirant un nombre dans [2, 4[ et en supprimant 3, comme dans UnitRand().
    // Note : 0x40000000 est la représentation hexadecimale de 2.f
    //        0x007fffff est le masque de la mantisse.
    // NDP: re-malin, re-n'est ce pas ?

    const u32 r = NextU32();
    const u32 iResult = 0x40000000 | (r & 0x007fffff);

    const char* pChar = (const char*)&iResult; // no type aliasing
    float result = *(const float*)pChar;
    Assert(result >= 2.f);
    Assert(result < 4.f);

    return result - 3.f;
}
//----------------------------------------------------------------------------
u32 RandomGenerator::NextSeed_(u32 seed) {
    return seed * 16777619U + 2166136261U;
}
//----------------------------------------------------------------------------
u32 RandomGenerator::NextState_(u32 state) {
    // http://burtleburtle.net/bob/hash/integer.html
    u32 a = state ^ 0xABADCAFE; // hash function salt
    a -= (a << 6);
    a ^= (a >> 17);
    a -= (a << 9);
    a ^= (a << 4);
    a -= (a << 3);
    a ^= (a << 10);
    a ^= (a >> 15);
    return a;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u32, _Dim> NextRandU32(RandomGenerator& rand) {
    ScalarVector<u32, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result[i] = rand.NextU32();
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u32, _Dim> NextRandU32(RandomGenerator& rand, u32 vmax) {
    ScalarVector<u32, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result[i] = rand.NextU32(vmax);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u32, _Dim> NextRandU32(RandomGenerator& rand, const ScalarVector<u32, _Dim>& vmax) {
    ScalarVector<u32, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result[i] = rand.NextU32(vmax[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u32, _Dim> NextRandU32(RandomGenerator& rand, u32 vmin, u32 vmax) {
    ScalarVector<u32, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result[i] = rand.NextU32(vmin, vmax);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u32, _Dim> NextRandU32(RandomGenerator& rand, const ScalarVector<u32, _Dim>& vmin, const ScalarVector<u32, _Dim>& vmax) {
    ScalarVector<u32, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result[i] = rand.NextU32(vmin[i], vmax[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> NextRandFloat01(RandomGenerator& rand) {
    ScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result[i] = rand.NextFloat01();
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> NextRandFloatM11(RandomGenerator& rand) {
    ScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result[i] = rand.NextFloat01();
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
