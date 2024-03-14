#pragma once

#include "Maths/MathHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename U, Meta::TEnableIf<std::is_arithmetic_v<T>&& std::is_arithmetic_v<U>>* >
CONSTEXPR auto BarycentricLerp(T v0, T v1, T v2, U f0, U f1, U f2) NOEXCEPT {
    return static_cast<T>(v0*f0 + v1*f1 + v2*f2);
}
//----------------------------------------------------------------------------
template <typename T, typename U, Meta::TEnableIf<std::is_arithmetic_v<T>&& std::is_arithmetic_v<U>>* >
CONSTEXPR auto BilateralLerp(T v00, T v10, T v11, T v01, U f0, U f1) NOEXCEPT {
    return static_cast<T>(Lerp(
        Lerp(v00, v01, f1),
        Lerp(v10, v11, f1),
        f0));
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
inline CONSTEXPR float Lerp(float v0, float v1, float f) NOEXCEPT {
    Assert(f >= float(0) && f <= float(1));
    return ((v0 * (float(1) - f)) + (v1 * f));
}
//----------------------------------------------------------------------------
inline CONSTEXPR double Lerp(double v0, double v1, double f) NOEXCEPT {
    Assert(f >= double(0) && f <= double(1));
    return ((v0 * (double(1) - f)) + (v1 * f));
}
//----------------------------------------------------------------------------
template <typename T, class>
CONSTEXPR float LinearStep(T value, Meta::TDontDeduce<T> vmin, Meta::TDontDeduce<T> vmax) NOEXCEPT {
    Assert(vmin < vmax);
    return Saturate(static_cast<float>((value - vmin) / static_cast<float>(vmax - vmin)));
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
template <typename T, typename>
NODISCARD CONSTEXPR u32 MinElement(const std::initializer_list<T>& list) {
    return static_cast<u32>(std::distance(list.begin(), std::min_element(list.begin(), list.end())));
}
//----------------------------------------------------------------------------
template <typename T, typename>
NODISCARD CONSTEXPR u32 MaxElement(const std::initializer_list<T>& list) {
    return static_cast<u32>(std::distance(list.begin(), std::max_element(list.begin(), list.end())));
}
//----------------------------------------------------------------------------
template <typename T, typename>
NODISCARD CONSTEXPR TPair<u32, u32> MinMaxElement(const std::initializer_list<T>& list) {
    const auto it = std::minmax_element(list.begin(), list.end());
    return {
        static_cast<u32>(std::distance(list.begin(), it.first)),
        static_cast<u32>(std::distance(list.begin(), it.second))
    };
}
//----------------------------------------------------------------------------
inline double Pow(double d, double n) NOEXCEPT {
    return std::pow(d, n);
}
//----------------------------------------------------------------------------
#if !USE_PPE_ASSERT
inline CONSTEXPR float Rcp(float f) NOEXCEPT {
#else
inline float Rcp(float f) {
    Assert(Abs(f) > SmallEpsilon);
#endif
    return (1.f / f);
}
//----------------------------------------------------------------------------
#if !USE_PPE_ASSERT
inline CONSTEXPR double Rcp(double d) NOEXCEPT {
#else
inline double Rcp(double d) {
    Assert(Abs(d) > Epsilon);
#endif
    return (1. / d);
}
//----------------------------------------------------------------------------
template <typename T, typename U, Meta::TEnableIf<std::is_arithmetic_v<T>&& std::is_arithmetic_v<U>>* >
inline CONSTEXPR auto SLerp(T v0, Meta::TDontDeduce<T> v1, U f) NOEXCEPT {
    Assert(f >= U(0) && f <= U(1));
    return (f < U(0.5))
        ? (v1 - v0) * f + v0
        : (v0 - v1) * (1 - f) + v1;
}
//----------------------------------------------------------------------------
inline CONSTEXPR float SStep(float x) NOEXCEPT {
    Assert_NoAssume(Saturate(x) == x);
    const float ix = (1 - x);
    x = x * x;
    return x / (x + ix * ix); ;
}
//----------------------------------------------------------------------------
inline CONSTEXPR double SStep(double x) NOEXCEPT {
    Assert_NoAssume(Saturate(x) == x);
    const double ix = (1 - x);
    x = x * x;
    return x / (x + ix * ix); ;
}
//----------------------------------------------------------------------------
template <typename T, typename U, Meta::TEnableIf<std::is_arithmetic_v<T>&& std::is_arithmetic_v<U>>* >
inline CONSTEXPR auto SMin(T a, Meta::TDontDeduce<T> b, U k) NOEXCEPT {
    // Polynomial smooth minimum by iq
    U h = Saturate(U(0.5) + U(0.5)*(a - b) / k);
    return T(Lerp(a, b, h) - k*h*(U(1.0) - h));
}
//----------------------------------------------------------------------------
template <typename T, typename U, Meta::TEnableIf<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>>* >
inline CONSTEXPR auto Smoothstep(T vmin, Meta::TDontDeduce<T> vmax, U f) NOEXCEPT {
    auto ff = Saturate((f - vmin) / (vmax - vmin));
    return ff*ff*(3 - 2 * ff);
}
//----------------------------------------------------------------------------
template <typename T, typename U, Meta::TEnableIf<std::is_arithmetic_v<T>&& std::is_arithmetic_v<U>>* >
inline CONSTEXPR auto Smootherstep(T vmin, Meta::TDontDeduce<T> vmax, U f) NOEXCEPT {
    f = Saturate((f - vmin) / (vmax - vmin));
    return f*f*f*(f*(f*6 - 15) + 10);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline CONSTEXPR float Degrees(float radians) NOEXCEPT {
    return radians * Rad2Deg;
}
//----------------------------------------------------------------------------
inline CONSTEXPR float Radians(float degrees) NOEXCEPT {
    return degrees * Deg2Rad;
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
inline bool NearlyEquals(float A, float B, float maxRelDiff/* = Epsilon */) NOEXCEPT {
    Assert(maxRelDiff > 0);

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
    Assert(maxRelDiff > 0);

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
inline float NormPDF(float x, float sigma) NOEXCEPT {
    return 0.39894f*FPlatformMaths::Exp(-0.5f*x*x/(sigma*sigma))/sigma;
}
//----------------------------------------------------------------------------
inline double NormPDF(double x, double sigma) NOEXCEPT {
    return 0.39894*std::exp(-0.5*x*x/(sigma*sigma))/sigma;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
