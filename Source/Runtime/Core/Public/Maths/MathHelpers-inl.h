#pragma once

#include "Maths/MathHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename U>
CONSTEXPR T BarycentricLerp(T v0, T v1, T v2, U f0, U f1, U f2) NOEXCEPT {
    return static_cast<T>(v0*f0 + v1*f1 + v2*f2);
}
//----------------------------------------------------------------------------
inline CONSTEXPR bool BarycentricLerp(bool v0, bool v1, bool v2, float f0, float f1, float f2) NOEXCEPT {
    return (((v0?1:0)*f0 + (v1?1:0)*f1 + (v2?1:0)*f2) >= 0.5f);
}
//----------------------------------------------------------------------------
inline float Hypot(float a, float b) NOEXCEPT {
    return Sqrt(a * a + b * b);
}
//----------------------------------------------------------------------------
inline double Hypot(double a, double b) NOEXCEPT {
    return Sqrt(a * a + b * b);
}
//----------------------------------------------------------------------------
template <typename T, typename U, class>
CONSTEXPR T Lerp(T v0, Meta::TDontDeduce<T> v1, U f) NOEXCEPT {
    Assert(f >= U(0) && f <= U(1));
    return static_cast<T>((v0 * (U(1) - f)) + (v1 * f));
}
//----------------------------------------------------------------------------
inline CONSTEXPR bool Lerp(bool v0, bool v1, float f) NOEXCEPT {
    Assert(f >= 0 && f <= 1);
    return (Lerp(v0 ? 1.f : 0.f, v1 ? 1.f : 0.f, f) >= 0.5f);
}
//----------------------------------------------------------------------------
template <typename T, class>
CONSTEXPR float LinearStep(T value, Meta::TDontDeduce<T> vmin, Meta::TDontDeduce<T> vmax) NOEXCEPT {
    Assert(vmin < vmax);
    return Saturate((value - vmin) / static_cast<float>(vmax - vmin));
}
//----------------------------------------------------------------------------
template <typename T, class>
CONSTEXPR void MinMax(Meta::TDontDeduce<T> a, Meta::TDontDeduce<T> b, T* pmin, T* pmax) NOEXCEPT {
    if (a < b)
        *pmin = a, *pmax = b;
    else
        *pmin = b, *pmax = a;
}
//----------------------------------------------------------------------------
template <typename T, class>
CONSTEXPR void MinMax3(Meta::TDontDeduce<T> a, Meta::TDontDeduce<T> b, Meta::TDontDeduce<T> c, T* pmin, T* pmax) NOEXCEPT {
    MinMax(a, b, pmin, pmax);
    *pmin = Min(*pmin, c);
    *pmax = Max(*pmax, c);
}
//----------------------------------------------------------------------------
inline double Pow(double d, double n) NOEXCEPT {
    return std::pow(d, n);
}
//----------------------------------------------------------------------------
#ifndef WITH_PPE_ASSERT
inline CONSTEXPR float Rcp(float f) NOEXCEPT {
#else
inline float Rcp(float f) {
    Assert(Abs(f) > F_SmallEpsilon);
#endif
    return (1.f / f);
}
//----------------------------------------------------------------------------
#ifndef WITH_PPE_ASSERT
inline CONSTEXPR double Rcp(double d) NOEXCEPT {
#else
inline double Rcp(double d) {
    Assert(Abs(d) > D_Epsilon);
#endif
    return (1. / d);
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T RSqrt(T f) NOEXCEPT {
    return Rcp(Sqrt(f));
}
//----------------------------------------------------------------------------
template <typename T, typename U, class>
CONSTEXPR T SLerp(T v0, Meta::TDontDeduce<T> v1, U f) NOEXCEPT {
    Assert(f >= U(0) && f <= U(1));
    return (f < T(0.5))
        ? (v1 - v0) * f + v0
        : (v0 - v1) * (1 - f) + v1;
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T Sqr(T x) NOEXCEPT {
    return x * x;
}
//----------------------------------------------------------------------------
inline double Sqrt(double d) NOEXCEPT {
    return std::sqrt(d);
}
//----------------------------------------------------------------------------
template <typename T, class>
CONSTEXPR T Step(T y, Meta::TDontDeduce<T> x) NOEXCEPT {
    return (x >= y) ? T(1) : T(0);
}
//----------------------------------------------------------------------------
template <typename T, typename U, class>
CONSTEXPR T SMin(T a, Meta::TDontDeduce<T> b, U k) NOEXCEPT {
    // Polynomial smooth minimum by iq
    U h = Saturate(U(0.5) + U(0.5)*(a - b) / k);
    return T(Lerp(a, b, h) - k*h*(U(1.0) - h));
}
//----------------------------------------------------------------------------
template <typename T, typename U, class>
CONSTEXPR auto Smoothstep(T vmin, Meta::TDontDeduce<T> vmax, U f) NOEXCEPT {
    auto ff = Saturate((f - vmin) / (vmax - vmin));
    return ff*ff*(3 - 2 * ff);
}
//----------------------------------------------------------------------------
template <typename T, typename U, class>
CONSTEXPR auto Smootherstep(T vmin, Meta::TDontDeduce<T> vmax, U f) NOEXCEPT {
    f = Saturate((f - vmin) / (vmax - vmin));
    return f*f*f*(f*(f*6 - 15) + 10);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline CONSTEXPR float Degrees(float radians) NOEXCEPT {
    return radians * F_Rad2Deg;
}
//----------------------------------------------------------------------------
inline CONSTEXPR float Radians(float degrees) NOEXCEPT {
    return degrees * F_Deg2Rad;
}
//----------------------------------------------------------------------------
inline void SinCos(float radians, float *fsin, float *fcos) NOEXCEPT {
    *fsin = FPlatformMaths::Sin(radians);
    *fcos = FPlatformMaths::Cos(radians);
}
//----------------------------------------------------------------------------
inline void SinCos(double radians, double *fsin, double *fcos) NOEXCEPT {
    *fsin = std::sin(radians);
    *fcos = std::cos(radians);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline bool NearlyEquals(float A, float B, float maxRelDiff/* = F_Epsilon */) NOEXCEPT {
    // Calculate the difference.
    const float diff = Abs(A - B);
    A = Abs(A);
    B = Abs(B);

    // Find the largest
    float largest = (B > A) ? B : A;

    return (diff <= largest * maxRelDiff);
}
//----------------------------------------------------------------------------
inline bool NearlyEquals(double A, double B, double maxRelDiff/* = D_Epsilon */) NOEXCEPT {
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
inline float ClampAngle(float degrees) NOEXCEPT { // [0,360)
    degrees = FMod(degrees, 360.f);
    degrees = (degrees < 0.f ? degrees + 360.f : degrees);
    return degrees;
}
//----------------------------------------------------------------------------
inline float NormalizeAngle(float degrees) NOEXCEPT { // (-180,180]
    degrees = ClampAngle(degrees);
    degrees = (degrees > 180.f ? degrees - 360.f : degrees);
    return degrees;
}
//----------------------------------------------------------------------------
inline u32 CubeMapFaceID(float x, float y, float z) NOEXCEPT {
    if (Abs(z) >= Abs(x) && Abs(z) >= Abs(y))
        return (z < 0.0f) ? 5 : 4;
    else if (Abs(y) >= Abs(x))
        return (y < 0.0f) ? 3 : 2;
    else
        return (x < 0.0f) ? 1 : 0;
}
//----------------------------------------------------------------------------
inline float GridSnap(float location, float grid) NOEXCEPT {
    //Assert(Abs(grid) > F_SmallEpsilon); // CONSTEXPR :'(
    return (FloorToFloat((location + 0.5f * grid) / grid) * grid);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
