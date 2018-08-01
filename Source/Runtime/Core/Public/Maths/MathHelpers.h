#pragma once

#include "Core.h"

#include "HAL/PlatformMaths.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
constexpr float D_Epsilon = 1e-5;
//----------------------------------------------------------------------------
constexpr float F_Epsilon = 1e-3f;
constexpr float F_EpsilonSQ = 1e-9f;
constexpr float F_SmallEpsilon = 1e-6f;
constexpr float F_LargeEpsilon = 0.01f;
constexpr float F_Delta = 0.00001f;
constexpr float F_PI = 3.14159265359f;
constexpr float F_2PI = 6.28318530718f;
constexpr float F_3PI = 9.42477796077f;
constexpr float F_4PI = 12.5663706144f;
constexpr float F_PIOver3 = 1.0471975512f;
constexpr float F_PIOver4 = 0.78539816339f;
constexpr float F_2PIOver3 = 2.09439510239f;
constexpr float F_HalfPi = 1.57079632679f;
constexpr float F_Deg2Rad = 0.01745329251f;
constexpr float F_Rad2Deg = 57.2957795131f;
constexpr float F_Sqrt2 = 1.4142135623730951f;
constexpr float F_Sqrt2OO = 0.7071067811865475f;
constexpr float F_SqrtHalf = F_Sqrt2OO;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline float Abs(float f) { return FPlatformMaths::Abs(f); }
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr T BarycentricLerp(T v0, T v1, T v2, U f0, U f1, U f2);
//----------------------------------------------------------------------------
template <typename T>
T CeilToFloat(T f) {
    return std::ceil(f);
}
inline float CeilToFloat(float f) { return FPlatformMaths::CeilToFloat(f); }
inline double CeilToFloat(double d) { return FPlatformMaths::CeilToDouble(d); }
//----------------------------------------------------------------------------
template <typename T>
T FloorToFloat(T f) {
    return std::floor(f);
}
inline float FloorToFloat(float f) { return FPlatformMaths::FloorToFloat(f); }
inline double FloorToFloat(double d) { return FPlatformMaths::FloorToDouble(d); }
//----------------------------------------------------------------------------
template <typename T>
T Frac(T f);
inline float Frac(float f) { return FPlatformMaths::Frac(f); }
//----------------------------------------------------------------------------
template <typename T>
T Fractional(T f);
inline float Fractional(float f) { return FPlatformMaths::Fractional(f); }
//----------------------------------------------------------------------------
template <typename T>
constexpr T FMod(T f, T m);
inline float FMod(float f, float m) { return FPlatformMaths::Fmod(f, m); }
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr T Lerp(T v0, T v1, U f);
//----------------------------------------------------------------------------
template <typename T>
constexpr float LinearStep(T value, T vmin, T vmax);
//----------------------------------------------------------------------------
template <typename T>
constexpr T Max3(T a, T b, T c) { return Max(a, Max(b, c)); }
//----------------------------------------------------------------------------
template <typename T>
constexpr T Min3(T a, T b, T c) { return Min(a, Min(b, c)); }
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr T Pow(T v, U n);
//----------------------------------------------------------------------------
template <typename U>
constexpr float Pow(float f, U n);
inline float Pow(float f, float n) { return FPlatformMaths::Pow(f, n); }
//----------------------------------------------------------------------------
template <typename U>
constexpr double Pow(double d, U n);
//----------------------------------------------------------------------------
#ifdef WITH_PPE_ASSERT
float Rcp(float f);
double Rcp(double d);
#else
constexpr float Rcp(float f);
constexpr double Rcp(double d);
#endif
//----------------------------------------------------------------------------
template <typename T>
T Round(T f) {
    return std::round(f);
}
inline float RoundToFloat(float f) { return FPlatformMaths::RoundToFloat(f); }
inline double RoundToFloat(double d) { return FPlatformMaths::RoundToDouble(d); }
//----------------------------------------------------------------------------
template <typename T>
constexpr T RSqrt(T f);
inline float RSqrt(float f) { return FPlatformMaths::RSqrt(f); }
//----------------------------------------------------------------------------
template <typename T>
constexpr T Saturate(T value) { return Clamp(value, T(0), T(1)); }
//----------------------------------------------------------------------------
template <typename T>
constexpr T Sign(T value) { return (value < T(0) ? T(-1) : (value == T(0) ? T(0) : T(1))); }
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr T SLerp(T v0, T v1, U f);
//----------------------------------------------------------------------------
template <typename T>
constexpr T Sqr(T x);
//----------------------------------------------------------------------------
template <typename T>
constexpr T Sqrt(T x);
inline float Sqrt(float f) { return FPlatformMaths::Sqrt(f); }
//----------------------------------------------------------------------------
template <typename T>
constexpr T Step(T y, T x);
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr T SMin(T a, T b, U k);
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr U Smoothstep(T vmin, T vmax, U f);
//----------------------------------------------------------------------------
template <typename T, typename U>
constexpr U Smootherstep(T vmin, T vmax, U f);
//----------------------------------------------------------------------------
template <typename T>
T Trunc(T f) {
    return std::trunc(f);
}
inline float TruncToFloat(float f) { return FPlatformMaths::TruncToFloat(f); }
inline double TruncToFloat(double d) { return FPlatformMaths::TruncToDouble(d); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline int CeilToInt(float f) { return FPlatformMaths::CeilToInt(f); }
//----------------------------------------------------------------------------
inline int FloorToInt(float f) { return FPlatformMaths::FloorToInt(f); }
//----------------------------------------------------------------------------
inline int RoundToInt(float f) { return FPlatformMaths::RoundToInt(f); }
//----------------------------------------------------------------------------
inline int TruncToInt(float f) { return FPlatformMaths::TruncToInt(f); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
constexpr float Degrees(float radians);
//----------------------------------------------------------------------------
constexpr float Radians(float degrees);
//----------------------------------------------------------------------------
constexpr float Float01_to_FloatM11(float v_01) { return (v_01 * 2.f - 1.f); }
constexpr float FloatM11_to_Float01(float v_M11) { return (v_M11 * .5f + .5f); }
//----------------------------------------------------------------------------
template <typename T>
constexpr void SinCos(T radians, T *fsin, T *fcos);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool NearlyEquals(float A, float B, float maxRelDiff = F_Epsilon);
bool NearlyEquals(double A, double B, double maxRelDiff = D_Epsilon);
//----------------------------------------------------------------------------
inline bool IsINF(float f)  { return std::isinf(f); }
inline bool IsINF(double d) { return std::isinf(d); }
//----------------------------------------------------------------------------
inline bool IsNAN(float f)  { return std::isnan(f); }
inline bool IsNAN(double d) { return std::isnan(d); }
//----------------------------------------------------------------------------
inline bool IsNANorINF(float f)  { return (IsNAN(f) || IsINF(f)); }
inline bool IsNANorINF(double d) { return (IsNAN(d) || IsINF(d)); }
//----------------------------------------------------------------------------
float ClampAngle(float degrees);
//----------------------------------------------------------------------------
float NormalizeAngle(float degrees);
//----------------------------------------------------------------------------
// https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
size_t CubeMapFaceID(float x, float y, float z);
//----------------------------------------------------------------------------
float GridSnap(float location, float grid);
//----------------------------------------------------------------------------
// https://github.com/lemire/fastrange/blob/master/fastrange.h
inline constexpr u32 Bounded(u32 x, u32 N) {
    return (u32)(((u64)x * (u64)N) >> 32);
}
inline u64 Bounded(u64 x, u64 N) {
#ifdef __SIZEOF_INT128__ // then we know we have a 128-bit int
    return (u64)(((__uint128_t)x * (__uint128_t)N) >> 64);
#elif defined(_MSC_VER) && defined(_WIN64)
    // supported in Visual Studio 2005 and better
    u64 highProduct;
    _umul128(x, N, &highProduct); // ignore output
    return highProduct;
    unsigned __int64 _umul128(
        unsigned __int64 Multiplier,
        unsigned __int64 Multiplicand,
        unsigned __int64 *HighProduct
    );
#else
    return x % N; // fallback
#endif // __SIZEOF_INT128__
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/MathHelpers-inl.h"
