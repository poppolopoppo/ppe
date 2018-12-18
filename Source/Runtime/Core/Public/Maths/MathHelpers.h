#pragma once

#include "Core.h"

#include "HAL/PlatformMaths.h"

#if defined(_MSC_VER) && defined (_WIN64)
#include <intrin.h>// should be part of all recent Visual Studio
#pragma intrinsic(_umul128)
#endif // defined(_MSC_VER) && defined (_WIN64)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CONSTEXPR double D_Epsilon = 1e-5;
//----------------------------------------------------------------------------
CONSTEXPR float F_Epsilon = 1e-3f;
CONSTEXPR float F_EpsilonSQ = 1e-9f;
CONSTEXPR float F_SmallEpsilon = 1e-6f;
CONSTEXPR float F_LargeEpsilon = 0.01f;
CONSTEXPR float F_Delta = 0.00001f;
CONSTEXPR float F_PI = 3.14159265359f;
CONSTEXPR float F_2PI = 6.28318530718f;
CONSTEXPR float F_3PI = 9.42477796077f;
CONSTEXPR float F_4PI = 12.5663706144f;
CONSTEXPR float F_PIOver3 = 1.0471975512f;
CONSTEXPR float F_PIOver4 = 0.78539816339f;
CONSTEXPR float F_2PIOver3 = 2.09439510239f;
CONSTEXPR float F_HalfPi = 1.57079632679f;
CONSTEXPR float F_Deg2Rad = 0.01745329251f;
CONSTEXPR float F_Rad2Deg = 57.2957795131f;
CONSTEXPR float F_Sqrt2 = 1.4142135623730951f;
CONSTEXPR float F_Sqrt2OO = 0.7071067811865475f;
CONSTEXPR float F_SqrtHalf = F_Sqrt2OO;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline CONSTEXPR i8 Abs(i8 v) NOEXCEPT { return FPlatformMaths::Abs(v); }
inline CONSTEXPR i16 Abs(i16 v) NOEXCEPT { return FPlatformMaths::Abs(v); }
inline CONSTEXPR i32 Abs(i32 v) NOEXCEPT { return FPlatformMaths::Abs(v); }
inline CONSTEXPR i64 Abs(i64 v) NOEXCEPT { return FPlatformMaths::Abs(v); }
inline CONSTEXPR float Abs(float v) NOEXCEPT { return FPlatformMaths::Abs(v); }
inline CONSTEXPR double Abs(double v) NOEXCEPT { return FPlatformMaths::Abs(v); }
//----------------------------------------------------------------------------
template <typename T, typename U>
CONSTEXPR T BarycentricLerp(T v0, T v1, T v2, U f0, U f1, U f2) NOEXCEPT;
//----------------------------------------------------------------------------
inline CONSTEXPR auto BiasScale(float x, float bias, float scale) NOEXCEPT { return ((x + bias) * scale); }
inline CONSTEXPR auto BiasScale(double x, double bias, double scale) NOEXCEPT { return ((x + bias) * scale); }
//----------------------------------------------------------------------------
inline float Exp(float value) NOEXCEPT { return FPlatformMaths::Exp(value); }
inline float Exp2(float value) NOEXCEPT { return FPlatformMaths::Exp2(value); }
inline float Loge(float value) NOEXCEPT { return FPlatformMaths::Loge(value); }
inline float LogX(float value, float n) NOEXCEPT { return FPlatformMaths::LogX(value, n); }
inline float Log2(float value) NOEXCEPT { return FPlatformMaths::Log2(value); }
//----------------------------------------------------------------------------
inline float Frac(float f) NOEXCEPT { return FPlatformMaths::Frac(f); }
inline float Fractional(float f) NOEXCEPT { return FPlatformMaths::Fractional(f); }
//----------------------------------------------------------------------------
inline float FMod(float f, float m) NOEXCEPT { return FPlatformMaths::Fmod(f, m); }
//----------------------------------------------------------------------------
float Hypot(float a, float b) NOEXCEPT;
double Hypot(double a, double b) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, typename U, class = Meta::TEnableIf<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>> >
CONSTEXPR T Lerp(T v0, Meta::TDontDeduce<T> v1, U f) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, class = Meta::TEnableIf<std::is_arithmetic_v<T>> >
CONSTEXPR float LinearStep(T value, Meta::TDontDeduce<T> vmin, Meta::TDontDeduce<T> vmax) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, class = Meta::TEnableIf<std::is_arithmetic_v<T>> >
CONSTEXPR void MinMax(Meta::TDontDeduce<T> a, Meta::TDontDeduce<T> b, T* pmin, T* pmax) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, class = Meta::TEnableIf<std::is_arithmetic_v<T>> >
CONSTEXPR void MinMax3(Meta::TDontDeduce<T> a, Meta::TDontDeduce<T> b, Meta::TDontDeduce<T> c, T* pmin, T* pmax) NOEXCEPT;
//----------------------------------------------------------------------------
inline float Pow(float f, float n) NOEXCEPT { return FPlatformMaths::Pow(f, n); }
double Pow(double d, double n) NOEXCEPT;
//----------------------------------------------------------------------------
#ifdef WITH_PPE_ASSERT
float Rcp(float f);
double Rcp(double d);
#else
CONSTEXPR float Rcp(float f) NOEXCEPT;
CONSTEXPR double Rcp(double d) NOEXCEPT;
#endif
//----------------------------------------------------------------------------
inline float RSqrt(float f) NOEXCEPT { return FPlatformMaths::RSqrt(f); }
inline float RSqrt_Low(float f) NOEXCEPT { return FPlatformMaths::RSqrt_Low(f); }
//----------------------------------------------------------------------------
inline CONSTEXPR float Saturate(float value) NOEXCEPT { return Clamp(value, 0.0f, 1.0f); }
inline CONSTEXPR double Saturate(double value) NOEXCEPT { return Clamp(value, 0.0f, 1.0f); }
//----------------------------------------------------------------------------
inline CONSTEXPR i8 Sign(i8 value) NOEXCEPT { return FPlatformMaths::Sign(value); }
inline CONSTEXPR i16 Sign(i16 value) NOEXCEPT { return FPlatformMaths::Sign(value); }
inline CONSTEXPR i32 Sign(i32 value) NOEXCEPT { return FPlatformMaths::Sign(value); }
inline CONSTEXPR i64 Sign(i64 value) NOEXCEPT { return FPlatformMaths::Sign(value); }
inline CONSTEXPR float Sign(float value) NOEXCEPT { return FPlatformMaths::Sign(value); }
inline CONSTEXPR double Sign(double value) NOEXCEPT { return FPlatformMaths::Sign(value); }
//----------------------------------------------------------------------------
template <typename T, typename U, class = Meta::TEnableIf<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>> >
CONSTEXPR T SLerp(T v0, Meta::TDontDeduce<T> v1, U f) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T Sqr(T x) NOEXCEPT;
//----------------------------------------------------------------------------
inline float Sqrt(float f) NOEXCEPT { return FPlatformMaths::Sqrt(f); }
double Sqrt(double d) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, class = Meta::TEnableIf<std::is_arithmetic_v<T>> >
CONSTEXPR T Step(T y, Meta::TDontDeduce<T> x) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, typename U, class = Meta::TEnableIf<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>> >
CONSTEXPR T SMin(T a, Meta::TDontDeduce<T> b, U k) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, typename U, class = Meta::TEnableIf<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>>  >
CONSTEXPR auto Smoothstep(T vmin, Meta::TDontDeduce<T> vmax, U f) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, typename U, class = Meta::TEnableIf<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>> >
CONSTEXPR auto Smootherstep(T vmin, Meta::TDontDeduce<T> vmax, U f) NOEXCEPT;
//----------------------------------------------------------------------------
inline float CeilToFloat(float f) NOEXCEPT { return FPlatformMaths::CeilToFloat(f); }
inline double CeilToFloat(double d) NOEXCEPT { return FPlatformMaths::CeilToDouble(d); }
inline int CeilToInt(float f) NOEXCEPT { return FPlatformMaths::CeilToInt(f); }
//----------------------------------------------------------------------------
inline float FloorToFloat(float f) NOEXCEPT { return FPlatformMaths::FloorToFloat(f); }
inline double FloorToFloat(double d) NOEXCEPT { return FPlatformMaths::FloorToDouble(d); }
inline int FloorToInt(float f) NOEXCEPT { return FPlatformMaths::FloorToInt(f); }
//----------------------------------------------------------------------------
inline float RoundToFloat(float f) NOEXCEPT { return FPlatformMaths::RoundToFloat(f); }
inline double RoundToFloat(double d) NOEXCEPT { return FPlatformMaths::RoundToDouble(d); }
inline int RoundToInt(float f) NOEXCEPT { return FPlatformMaths::RoundToInt(f); }
//----------------------------------------------------------------------------
inline float TruncToFloat(float f) NOEXCEPT { return FPlatformMaths::TruncToFloat(f); }
inline double TruncToFloat(double d) NOEXCEPT { return FPlatformMaths::TruncToDouble(d); }
inline int TruncToInt(float f) NOEXCEPT { return FPlatformMaths::TruncToInt(f); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CONSTEXPR float Degrees(float radians) NOEXCEPT;
//----------------------------------------------------------------------------
CONSTEXPR float Radians(float degrees) NOEXCEPT;
//----------------------------------------------------------------------------
CONSTEXPR float Float01_to_FloatM11(float v_01) NOEXCEPT { return (v_01 * 2.f - 1.f); }
CONSTEXPR float FloatM11_to_Float01(float v_M11) NOEXCEPT { return (v_M11 * .5f + .5f); }
//----------------------------------------------------------------------------
void SinCos(float radians, float *fsin, float *fcos) NOEXCEPT;
void SinCos(double radians, double *fsin, double *fcos) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool NearlyEquals(float A, float B, float maxRelDiff = F_Epsilon) NOEXCEPT;
bool NearlyEquals(double A, double B, double maxRelDiff = D_Epsilon) NOEXCEPT;
//----------------------------------------------------------------------------
inline bool IsINF(float f) NOEXCEPT { return std::isinf(f); }
inline bool IsINF(double d) NOEXCEPT { return std::isinf(d); }
//----------------------------------------------------------------------------
inline bool IsNAN(float f) NOEXCEPT { return std::isnan(f); }
inline bool IsNAN(double d) NOEXCEPT { return std::isnan(d); }
//----------------------------------------------------------------------------
inline bool IsNANorINF(float f) NOEXCEPT { return (IsNAN(f) || IsINF(f)); }
inline bool IsNANorINF(double d) NOEXCEPT { return (IsNAN(d) || IsINF(d)); }
//----------------------------------------------------------------------------
float ClampAngle(float degrees) NOEXCEPT;
//----------------------------------------------------------------------------
float NormalizeAngle(float degrees) NOEXCEPT;
//----------------------------------------------------------------------------
// https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
u32 CubeMapFaceID(float x, float y, float z) NOEXCEPT;
//----------------------------------------------------------------------------
float GridSnap(float location, float grid) NOEXCEPT;
//----------------------------------------------------------------------------
// http://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
// https://github.com/lemire/fastrange/blob/master/fastrange.h
inline CONSTEXPR u32 Bounded(u32 x, u32 N) NOEXCEPT {
    return (u32)(((u64)x * (u64)N) >> 32);
}
inline u64 Bounded(u64 x, u64 N) NOEXCEPT {
#ifdef __SIZEOF_INT128__ // then we know we have a 128-bit int
    return (u64)(((__uint128_t)x * (__uint128_t)N) >> 64);
#elif defined(_MSC_VER) && defined(_WIN64)
    // supported in Visual Studio 2005 and better
    u64 highProduct;
    ::_umul128(x, N, &highProduct); // ignore output
    return highProduct;
    unsigned __int64 ::_umul128(
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
