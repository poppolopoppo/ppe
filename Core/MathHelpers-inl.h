#pragma once

#include "MathHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Clamp(T value, T vmin, T vmax) {
    return std::min(vmax, std::max(vmin, value));
}
//----------------------------------------------------------------------------
template <typename T>
T Frac(T f) {
    return f - std::trunc(f);
}
//----------------------------------------------------------------------------
template <typename T, typename U>
T Lerp(T v0, T v1, U f) {
    return static_cast<T>(v0 + (v1 - v0) * f);
}
//----------------------------------------------------------------------------
template <typename T>
float LinearStep(T value, T vmin, T vmax) {
    Assert(vmin < vmax);
    return static_cast<float>(value - vmin) / (vmax - vmin);
}
//----------------------------------------------------------------------------
template <typename T>
T Rcp(T f) {
    return T(1) / f;
}
//----------------------------------------------------------------------------
template <typename T>
T RSqrt(T f) {
    return Rcp(std::sqrt(f));
}
//----------------------------------------------------------------------------
template <typename T, typename U>
T SLerp(T v0, T v1, U f) {
    return (f < T(0.5))
        ? (v1 - v0) * f + v0
        : (v0 - v1) * (1 - f) + v1;
}
//----------------------------------------------------------------------------
template <typename T>
T Sqr(T x) {
    return x * x;
}
//----------------------------------------------------------------------------
template <typename T>
T Step(T y, T x) {
    return (x >= y) ? T(1) : T(0);
}
//----------------------------------------------------------------------------
template <typename T, typename U>
U Smoothstep(T vmin, T vmax, U f) {
    f = Saturate((f - vmin) / (vmax - vmin));
    return f*f*(3 - 2 * f);
}
//----------------------------------------------------------------------------
template <typename T, typename U>
U Smootherstep(T vmin, T vmax, U f) {
    f = Saturate((f - vmin) / (vmax - vmin));
    return f*f*f*(f*(f*6 - 15) + 10);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float Degrees(float radians) {
    return radians * F_Rad2Deg;
}
//----------------------------------------------------------------------------
float Radians(float degrees) {
    return degrees * F_Deg2Rad;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void SinCos(T radians, T *fsin, T *fcos) {
    *fsin = std::sin(radians);
    *fcos = std::cos(radians);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
