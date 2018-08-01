#pragma once

#include "Core/Maths/MathHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
constexpr T Abs(T value, typename std::enable_if<std::is_signed<T>::value>::type* = nullptr) {
    return (value < 0 ? -value : value);
}
//----------------------------------------------------------------------------
template <typename T>
constexpr T Abs(T value, typename std::enable_if<not std::is_signed<T>::value>::type* = nullptr) {
    return (value);
}
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr T BarycentricLerp(T v0, T v1, T v2, U f0, U f1, U f2) {
    return static_cast<T>(v0*f0 + v1*f1 + v2*f2);
}
//----------------------------------------------------------------------------
inline constexpr bool BarycentricLerp(bool v0, bool v1, bool v2, float f0, float f1, float f2) {
    return (((v0?1:0)*f0 + (v1?1:0)*f1 + (v2?1:0)*f2) >= 0.5f);
}
//----------------------------------------------------------------------------
template <typename T>
constexpr T FMod(T f, T m) {
    return std::fmod(f, m);
}
//----------------------------------------------------------------------------
template <typename T>
T Frac(T f) {
    return (f - FloorToFloat(f));
}
//----------------------------------------------------------------------------
template <typename T>
T Fractional(T f) {
    return (f - TruncToFloat(f));
}
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr T Lerp(T v0, T v1, U f) {
    Assert(f >= U(0) && f <= U(1));
    return static_cast<T>((v0 * (U(1) - f)) + (v1 * f));
}
//----------------------------------------------------------------------------
inline constexpr bool Lerp(bool v0, bool v1, float f) {
    Assert(f >= 0 && f <= 1);
    return (Lerp(v0 ? 1.f : 0.f, v1 ? 1.f : 0.f, f) >= 0.5f);
}
//----------------------------------------------------------------------------
template <typename T>
constexpr float LinearStep(T value, T vmin, T vmax) {
    Assert(vmin < vmax);
    return Saturate((value - vmin) / static_cast<float>(vmax - vmin));
}
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr T Pow(T v, U n) {
    return (n == 0)  ? 1 : v * Pow(v, n - 1);
}
//----------------------------------------------------------------------------
template <typename U>
constexpr float Pow(float f, U n) {
    return std::pow(f, n);
}
//----------------------------------------------------------------------------
template <typename U>
constexpr double Pow(double d, U n) {
    return std::pow(d, n);
}
//----------------------------------------------------------------------------
#ifndef WITH_CORE_ASSERT
inline constexpr float Rcp(float f) {
#else
inline float Rcp(float f) {
    Assert(Abs(f) > F_SmallEpsilon);
#endif
    return (1.f / f);
}
//----------------------------------------------------------------------------
#ifndef WITH_CORE_ASSERT
inline constexpr double Rcp(double d) {
#else
inline double Rcp(double d) {
    Assert(Abs(d) > D_Epsilon);
#endif
    return (1. / d);
}
//----------------------------------------------------------------------------
template <typename T>
constexpr T RSqrt(T f) {
    return Rcp(std::sqrt(f));
}
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr T SLerp(T v0, T v1, U f) {
    Assert(f >= U(0) && f <= U(1));
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
constexpr T SMin(T a, T b, U k) {
    // Polynomial smooth minimum by iq
    U h = Saturate(U(0.5) + U(0.5)*(a - b) / k);
    return T(Lerp(a, b, h) - k*h*(U(1.0) - h));
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
constexpr void SinCos(T radians, T *fsin, T *fcos) {
    *fsin = FPlatformMaths::Sin(radians);
    *fcos = FPlatformMaths::Cos(radians);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline bool NearlyEquals(float A, float B, float maxRelDiff/* = F_Epsilon */) {
    // Calculate the difference.
    const float diff = Abs(A - B);
    A = Abs(A);
    B = Abs(B);

    // Find the largest
    float largest = (B > A) ? B : A;

    return (diff <= largest * maxRelDiff);
}
//----------------------------------------------------------------------------
inline bool NearlyEquals(double A, double B, double maxRelDiff/* = D_Epsilon */) {
    // Calculate the difference.
    const double diff = Abs(A - B);
    A = Abs(A);
    B = Abs(B);

    // Find the largest
    double largest = (B > A) ? B : A;

    return (diff <= largest * maxRelDiff);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline float ClampAngle(float degrees) { // [0,360)
    degrees = FMod(degrees, 360.f);
    degrees = (degrees < 0.f ? degrees + 360.f : degrees);
    return degrees;
}
//----------------------------------------------------------------------------
inline float NormalizeAngle(float degrees) { // (-180,180]
    degrees = ClampAngle(degrees);
    degrees = (degrees > 180.f ? degrees - 360.f : degrees);
    return degrees;
}
//----------------------------------------------------------------------------
inline size_t CubeMapFaceID(float x, float y, float z) {
    if (Abs(z) >= Abs(x) && Abs(z) >= Abs(y))
        return (z < 0.0f) ? 5 : 4;
    else if (Abs(y) >= Abs(x))
        return (y < 0.0f) ? 3 : 2;
    else
        return (x < 0.0f) ? 1 : 0;
}
//----------------------------------------------------------------------------
inline float GridSnap(float location, float grid) {
    //Assert(Abs(grid) > F_SmallEpsilon); // constexpr :'(
    return (FloorToFloat((location + 0.5f * grid) / grid) * grid);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
