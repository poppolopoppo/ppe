#pragma once

#include "Core/Maths/MathHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Abs(T value, typename std::enable_if<std::is_signed<T>::value>::type* = nullptr) {
    return (value < 0 ? -value : value);
}
//----------------------------------------------------------------------------
template <typename T>
T Abs(T value, typename std::enable_if<not std::is_signed<T>::value>::type* = nullptr) {
    return (value);
}
//----------------------------------------------------------------------------
template <typename T>
constexpr T Clamp(T value, T vmin, T vmax) {
    return std::min(vmax, std::max(vmin, value));
}
//----------------------------------------------------------------------------
template <typename T>
constexpr T Frac(T f) {
    return f - std::trunc(f);
}
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr T Lerp(T v0, T v1, U f) {
    return static_cast<T>(v0 + (v1 - v0) * f);
}
//----------------------------------------------------------------------------
template <typename T>
constexpr float LinearStep(T value, T vmin, T vmax) {
    Assert(vmin < vmax);
    return static_cast<float>(value - vmin) / (vmax - vmin);
}
//----------------------------------------------------------------------------
template <typename T>
constexpr T Rcp(T f) {
    return T(1) / f;
}
//----------------------------------------------------------------------------
template <typename T>
constexpr T RSqrt(T f) {
    return Rcp(std::sqrt(f));
}
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr T SLerp(T v0, T v1, U f) {
    return (f < T(0.5))
        ? (v1 - v0) * f + v0
        : (v0 - v1) * (1 - f) + v1;
}
//----------------------------------------------------------------------------
template <typename T>
constexpr T Sqr(T x) {
    return x * x;
}
//----------------------------------------------------------------------------
template <typename T>
constexpr T Sqrt(T x) {
    return std::sqrt(x);
}
//----------------------------------------------------------------------------
template <typename T>
constexpr T Step(T y, T x) {
    return (x >= y) ? T(1) : T(0);
}
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr U Smoothstep(T vmin, T vmax, U f) {
    f = Saturate((f - vmin) / (vmax - vmin));
    return f*f*(3 - 2 * f);
}
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr U Smootherstep(T vmin, T vmax, U f) {
    f = Saturate((f - vmin) / (vmax - vmin));
    return f*f*f*(f*(f*6 - 15) + 10);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
constexpr float Degrees(float radians) {
    return radians * F_Rad2Deg;
}
//----------------------------------------------------------------------------
constexpr float Radians(float degrees) {
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
inline bool NearlyEquals(float A, float B, float maxRelDiff/* = 1e-3f */) {
    // Calculate the difference.
    const float diff = fabs(A - B);
    A = fabs(A);
    B = fabs(B);

    // Find the largest
    float largest = (B > A) ? B : A;

    if (diff <= largest * maxRelDiff)
        return true;
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
