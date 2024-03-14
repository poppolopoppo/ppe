#pragma once

#include "Core_fwd.h"

#include "Container/Pair.h"

#include "HAL/PlatformMaths.h"

#if defined(_MSC_VER) && defined (_WIN64)
#include <intrin.h>// should be part of all recent Visual Studio
#pragma intrinsic(_umul128)
#endif // defined(_MSC_VER) && defined (_WIN64)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FConstantNumber {
    template <typename T>
    struct TInvalid {
        static_assert(Meta::AlwaysFalse<T>,
            "A program that instantiates a primary template of a mathematical constant "
            "variable template is ill-formed. (N4835 [math.constants]/3)");
    };

    const float Float;
    const double Double;

    constexpr FConstantNumber(double d)
        : FConstantNumber(static_cast<float>(d), d)
    {}

    constexpr FConstantNumber(float f, double d) : Float(f), Double(d) {}

    constexpr FORCE_INLINE operator float() const { return Float; }
    constexpr FORCE_INLINE operator double() const { return Double; }

    constexpr FORCE_INLINE float Get(Meta::TType<float>) const { return Float; }
    constexpr FORCE_INLINE double Get(Meta::TType<double>) const { return Double; }

#define PPE_CONSTANTNUMBER_OPERATOR_T_DEF(_Float, _Op) \
    friend constexpr FORCE_INLINE decltype(std::declval<_Float>() _Op std::declval<_Float>()) operator _Op(const FConstantNumber& a, _Float b) { return a.Get(Meta::Type<_Float>) _Op b; } \
    friend constexpr FORCE_INLINE decltype(std::declval<_Float>() _Op std::declval<_Float>()) operator _Op(_Float a, const FConstantNumber& b) { return a _Op b.Get(Meta::Type<_Float>); }
#define PPE_CONSTANTNUMBER_OPERATOR_DEF(_Op) \
    PPE_CONSTANTNUMBER_OPERATOR_T_DEF(float, _Op) \
    PPE_CONSTANTNUMBER_OPERATOR_T_DEF(double, _Op)

    PPE_CONSTANTNUMBER_OPERATOR_DEF(+)
    PPE_CONSTANTNUMBER_OPERATOR_DEF(-)
    PPE_CONSTANTNUMBER_OPERATOR_DEF(*)
    PPE_CONSTANTNUMBER_OPERATOR_DEF(/)

    PPE_CONSTANTNUMBER_OPERATOR_DEF(<)
    PPE_CONSTANTNUMBER_OPERATOR_DEF(<=)
    PPE_CONSTANTNUMBER_OPERATOR_DEF(>)
    PPE_CONSTANTNUMBER_OPERATOR_DEF(>=)

#undef PPE_CONSTANTNUMBER_OPERATOR_DEF
#undef PPE_CONSTANTNUMBER_OPERATOR_T_DEF
};
//----------------------------------------------------------------------------
#define PPE_CONSTANTNUMBER_DEF(_Name, ...) \
    inline CONSTEXPR FConstantNumber _Name{ __VA_ARGS__ }; \
    template <typename T> inline CONSTEXPR FConstantNumber::TInvalid<T> CONCAT(_Name, _v); \
    template <> inline CONSTEXPR float CONCAT(_Name, _v)<float>{ _Name }; \
    template <> inline CONSTEXPR double CONCAT(_Name, _v)<double>{ _Name }

PPE_CONSTANTNUMBER_DEF(Epsilon, 1e-3f, 1e-5);
PPE_CONSTANTNUMBER_DEF(EpsilonSQ, Epsilon_v<float>*Epsilon_v<float>, Epsilon_v<double>*Epsilon_v<double>);
PPE_CONSTANTNUMBER_DEF(SmallEpsilon, 1e-7f, 1e-9);
PPE_CONSTANTNUMBER_DEF(LargeEpsilon, 0.01f, 0.001f);
PPE_CONSTANTNUMBER_DEF(Delta, 0.00001);

PPE_CONSTANTNUMBER_DEF(E, 2.718281828459045090795598298427648842334747314453);
PPE_CONSTANTNUMBER_DEF(PI, 3.141592653589793115997963468544185161590576171875);
PPE_CONSTANTNUMBER_DEF(PIOver2, 1.047197551196597631317786181170959025621414184570);
PPE_CONSTANTNUMBER_DEF(PIOver3, 1.047197551196597631317786181170959025621414184570);
PPE_CONSTANTNUMBER_DEF(PIOver4, 0.785398163397448278999490867136046290397644042969);
PPE_CONSTANTNUMBER_DEF(HalfPi, PIOver2);
PPE_CONSTANTNUMBER_DEF(Deg2Rad, 0.017453292519943295474371680597869271878153085709);
PPE_CONSTANTNUMBER_DEF(Rad2Deg, 57.29577951308232286464772187173366546630859375000);
PPE_CONSTANTNUMBER_DEF(Sqrt2, 1.414213562373095145474621858738828450441360473633);
PPE_CONSTANTNUMBER_DEF(Sqrt2OO, 0.707106781186547461715008466853760182857513427734);
PPE_CONSTANTNUMBER_DEF(SqrtHalf, Sqrt2OO);

#undef PPE_CONSTANTNUMBER_DEF
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline CONSTEXPR bool Any(bool value) { return value; }
inline CONSTEXPR bool All(bool value) { return value; }
//----------------------------------------------------------------------------
template <typename T, class = Meta::TEnableIf<std::is_arithmetic_v<T>> >
inline CONSTEXPR bool Any(T value) { return (!!value); }
//----------------------------------------------------------------------------
template <typename T, class = Meta::TEnableIf<std::is_integral_v<T>> >
inline CONSTEXPR bool All(T value) { return (value == UMax); }
//----------------------------------------------------------------------------
inline CONSTEXPR i8 Abs(i8 v) NOEXCEPT { return FPlatformMaths::Abs(v); }
inline CONSTEXPR i16 Abs(i16 v) NOEXCEPT { return FPlatformMaths::Abs(v); }
inline CONSTEXPR i32 Abs(i32 v) NOEXCEPT { return FPlatformMaths::Abs(v); }
inline CONSTEXPR i64 Abs(i64 v) NOEXCEPT { return FPlatformMaths::Abs(v); }
inline CONSTEXPR float Abs(float v) NOEXCEPT { return FPlatformMaths::Abs(v); }
inline CONSTEXPR double Abs(double v) NOEXCEPT { return FPlatformMaths::Abs(v); }
//----------------------------------------------------------------------------
template <typename T, typename U, Meta::TEnableIf<std::is_arithmetic_v<T>&& std::is_arithmetic_v<U>>* = nullptr >
CONSTEXPR auto BarycentricLerp(T v0, T v1, T v2, U f0, U f1, U f2) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, typename U, Meta::TEnableIf<std::is_arithmetic_v<T>&& std::is_arithmetic_v<U>>* = nullptr >
CONSTEXPR auto BilateralLerp(T v00, T v10, T v11, T v01, U f0, U f1) NOEXCEPT;
//----------------------------------------------------------------------------
inline CONSTEXPR auto BiasScale(float x, float bias, float scale) NOEXCEPT { return ((x + bias) * scale); }
inline CONSTEXPR auto BiasScale(double x, double bias, double scale) NOEXCEPT { return ((x + bias) * scale); }
//----------------------------------------------------------------------------
template <typename U, typename V, typename  _Result = std::common_type_t<U, V> >
CONSTEXPR auto Blend(const U& ifTrue, const V& ifFalse, bool mask) -> _Result {
    return (mask ? ifTrue : ifFalse);
}
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
CONSTEXPR float Lerp(float v0, float v1, float f) NOEXCEPT;
CONSTEXPR double Lerp(double v0, double v1, double f) NOEXCEPT;
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
template <typename T, typename = Meta::has_trivial_less_t<T>>
NODISCARD CONSTEXPR u32 MinElement(const std::initializer_list<T>& list);
//----------------------------------------------------------------------------
template <typename T, typename = Meta::has_trivial_less_t<T>>
NODISCARD CONSTEXPR u32 MaxElement(const std::initializer_list<T>& list);
//----------------------------------------------------------------------------
template <typename T, typename = Meta::has_trivial_less_t<T>>
NODISCARD CONSTEXPR TPair<u32, u32> MinMaxElement(const std::initializer_list<T>& list);
//----------------------------------------------------------------------------
inline float Pow(float f, float n) NOEXCEPT { return FPlatformMaths::Pow(f, n); }
double Pow(double d, double n) NOEXCEPT;
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
float Rcp(float f);
double Rcp(double d);
#else
CONSTEXPR float Rcp(float f) NOEXCEPT;
CONSTEXPR double Rcp(double d) NOEXCEPT;
#endif
//----------------------------------------------------------------------------
inline float RSqrt(float f) NOEXCEPT { return FPlatformMaths::RSqrt(f); }
inline float RSqrt_Low(float f) NOEXCEPT { return FPlatformMaths::RSqrt_Low(f); }
inline double RSqrt(double f) NOEXCEPT { return FPlatformMaths::RSqrt(f); }
//----------------------------------------------------------------------------
inline CONSTEXPR float Saturate(float value) NOEXCEPT { return Clamp(value, 0.0f, 1.0f); }
inline CONSTEXPR double Saturate(double value) NOEXCEPT { return Clamp(value, 0.0, 1.0); }
//----------------------------------------------------------------------------
inline CONSTEXPR i8 Sign(i8 value) NOEXCEPT { return FPlatformMaths::Sign(value); }
inline CONSTEXPR i16 Sign(i16 value) NOEXCEPT { return FPlatformMaths::Sign(value); }
inline CONSTEXPR i32 Sign(i32 value) NOEXCEPT { return FPlatformMaths::Sign(value); }
inline CONSTEXPR i64 Sign(i64 value) NOEXCEPT { return FPlatformMaths::Sign(value); }
inline CONSTEXPR float Sign(float value) NOEXCEPT { return FPlatformMaths::Sign(value); }
inline CONSTEXPR double Sign(double value) NOEXCEPT { return FPlatformMaths::Sign(value); }
//----------------------------------------------------------------------------
template <typename T, typename U, Meta::TEnableIf<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>>* = nullptr >
inline CONSTEXPR auto SLerp(T v0, Meta::TDontDeduce<T> v1, U f) NOEXCEPT;
//----------------------------------------------------------------------------
inline float Sqrt(float f) NOEXCEPT { return FPlatformMaths::Sqrt(f); }
inline double Sqrt(double f) NOEXCEPT { return FPlatformMaths::Sqrt(f); }
//----------------------------------------------------------------------------
template <typename T>
inline CONSTEXPR T Sqr(T x) NOEXCEPT { return x * x; }
//----------------------------------------------------------------------------
template <typename T, Meta::TEnableIf<std::is_arithmetic_v<T>>* = nullptr >
inline CONSTEXPR T Step(T y, Meta::TDontDeduce<T> x) NOEXCEPT { return (x >= y) ? T(1) : T(0); }
//----------------------------------------------------------------------------
inline CONSTEXPR float SStep(float x) NOEXCEPT;
inline CONSTEXPR double SStep(double x) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, typename U, Meta::TEnableIf<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>>* = nullptr>
inline CONSTEXPR auto SMin(T a, Meta::TDontDeduce<T> b, U k) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, typename U, Meta::TEnableIf<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>>* = nullptr>
inline CONSTEXPR auto Smoothstep(T vmin, Meta::TDontDeduce<T> vmax, U f) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, typename U, Meta::TEnableIf<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>>* = nullptr>
inline CONSTEXPR auto Smootherstep(T vmin, Meta::TDontDeduce<T> vmax, U f) NOEXCEPT;
//----------------------------------------------------------------------------
inline float CeilToFloat(float f) NOEXCEPT { return FPlatformMaths::CeilToFloat(f); }
inline double CeilToFloat(double d) NOEXCEPT { return FPlatformMaths::CeilToDouble(d); }
inline int CeilToInt(float f) NOEXCEPT { return FPlatformMaths::CeilToInt(f); }
inline unsigned CeilToUnsigned(float f) NOEXCEPT { return FPlatformMaths::CeilToUnsigned(f); }
//----------------------------------------------------------------------------
inline float FloorToFloat(float f) NOEXCEPT { return FPlatformMaths::FloorToFloat(f); }
inline double FloorToFloat(double d) NOEXCEPT { return FPlatformMaths::FloorToDouble(d); }
inline int FloorToInt(float f) NOEXCEPT { return FPlatformMaths::FloorToInt(f); }
inline unsigned FloorToUnsigned(float f) NOEXCEPT { return FPlatformMaths::FloorToUnsigned(f); }
//----------------------------------------------------------------------------
inline float RoundToFloat(float f) NOEXCEPT { return FPlatformMaths::RoundToFloat(f); }
inline double RoundToFloat(double d) NOEXCEPT { return FPlatformMaths::RoundToDouble(d); }
inline int RoundToInt(float f) NOEXCEPT { return FPlatformMaths::RoundToInt(f); }
inline unsigned RoundToUnsigned(float f) NOEXCEPT { return FPlatformMaths::RoundToUnsigned(f); }
//----------------------------------------------------------------------------
inline float TruncToFloat(float f) NOEXCEPT { return FPlatformMaths::TruncToFloat(f); }
inline double TruncToFloat(double d) NOEXCEPT { return FPlatformMaths::TruncToDouble(d); }
inline int TruncToInt(float f) NOEXCEPT { return FPlatformMaths::TruncToInt(f); }
inline unsigned TruncToUnsigned(float f) NOEXCEPT { return FPlatformMaths::TruncToUnsigned(f); }
//----------------------------------------------------------------------------
template <typename T, class = Meta::TEnableIf<std::is_integral_v<T>> >
inline CONSTEXPR T IntDivCeil(T x, T divider) { return (x + divider - 1) / divider; }
template <typename T, class = Meta::TEnableIf<std::is_integral_v<T>> >
inline CONSTEXPR T IntDivFloor(T x, T divider) { return x / divider; }
template <typename T, class = Meta::TEnableIf<std::is_integral_v<T>> >
inline CONSTEXPR T IntDivRound(T x, T divider) { return (x + divider / 2 - 1) / divider; }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CONSTEXPR float Degrees(float radians) NOEXCEPT;
//----------------------------------------------------------------------------
CONSTEXPR float Radians(float degrees) NOEXCEPT;
//----------------------------------------------------------------------------
CONSTEXPR float Float01_to_FloatM11(float v_01) NOEXCEPT { return Clamp(v_01 * 2.f - 1.f, -1.f, 1.f); }
CONSTEXPR float FloatM11_to_Float01(float v_M11) NOEXCEPT { return Saturate(v_M11 * .5f + .5f); }
//----------------------------------------------------------------------------
void SinCos(float radians, float *fsin, float *fcos) NOEXCEPT;
void SinCos(double radians, double *fsin, double *fcos) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool NearlyEquals(float A, float B, float maxRelDiff = Epsilon) NOEXCEPT;
bool NearlyEquals(double A, double B, double maxRelDiff = Epsilon) NOEXCEPT;
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
float NormPDF(float x, float sigma) NOEXCEPT;
double NormPDF(double x, double sigma) NOEXCEPT;
//----------------------------------------------------------------------------
// http://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
// https://github.com/lemire/fastrange/blob/master/fastrange.h
inline CONSTEXPR u32 Bounded(u32 x, u32 N) NOEXCEPT {
    return (u32)(((u64)x * (u64)N) >> 32);
}
#if 0
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
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/MathHelpers-inl.h"
