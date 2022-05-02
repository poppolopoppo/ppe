#pragma once

#include "Core.h"

#include "Maths/ScalarVector.h"
#include "Maths/ScalarVector_fwd.h"
#include "Maths/MathHelpers.h"
#include "Maths/PackingHelpers.h"

#include "HAL/PlatformEndian.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Any / All
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
NODISCARD CONSTEXPR bool Any(const TScalarVectorExpr<bool, _Dim, _Expr>& v) {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR{
        return (v.template get<idx>() || ...);
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
NODISCARD CONSTEXPR bool All(const TScalarVectorExpr<bool, _Dim, _Expr>& v) {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR{
        return (v.template get<idx>() && ...);
    });
}
//----------------------------------------------------------------------------
// Dot
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR T Dot(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) {
    return (lhs * rhs).HSum();
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
NODISCARD CONSTEXPR T Dot2(const TScalarVectorExpr<T, _Dim, _Expr>& v) {
    return Dot(v, v);
}
//----------------------------------------------------------------------------
// Length
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
NODISCARD CONSTEXPR T LengthSq(const TScalarVectorExpr<T, _Dim, _Expr>& v) {
    return Dot2(v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
NODISCARD CONSTEXPR auto Length(const TScalarVectorExpr<T, _Dim, _Expr>& v) {
    return Sqrt(LengthSq(v));
}
//----------------------------------------------------------------------------
// Distance
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename A, typename B>
NODISCARD CONSTEXPR T DistanceSq(const TScalarVectorExpr<T, _Dim, A>& a, const TScalarVectorExpr<T, _Dim, B>& b) {
    return LengthSq(b - a);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename A, typename B>
NODISCARD CONSTEXPR auto Distance(const TScalarVectorExpr<T, _Dim, A>& a, const TScalarVectorExpr<T, _Dim, B>& b) {
    return Length(b - a);
}
//----------------------------------------------------------------------------
// Normalize
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
CONSTEXPR auto Normalize(const TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT {
#if USE_PPE_ASSERT
    const T norm = Length(v);
    Assert(norm);
    return (v / norm);
#else
    return (v * RSqrt(LengthSq(v)));
#endif
}
//----------------------------------------------------------------------------
// Det
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T Det(const TScalarVector<T, 2>& a, const TScalarVector<T, 2>& b) NOEXCEPT {
    return (a.x * b.y - a.y * b.x);
}
//----------------------------------------------------------------------------
// Cross
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T Cross(const TScalarVector<T, 2>& o, const Meta::TDontDeduce<TScalarVector<T, 2>>& a, const Meta::TDontDeduce<TScalarVector<T, 2>>& b) NOEXCEPT {
    // Det(a - o, b - o) <=> Twiced signed area of the triangle
    return (((a.x - o.x) * (b.y - o.y)) -
            ((a.y - o.y) * (b.x - o.x)) );
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TScalarVector<T, 3> Cross(const TScalarVector<T, 3>& a, const Meta::TDontDeduce<TScalarVector<T, 3>>& b) NOEXCEPT {
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x };
}
//----------------------------------------------------------------------------
// Reflect / Refract
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TScalarVector<T, 3> Reflect(const TScalarVector<T, 3>& incident, const Meta::TDontDeduce<TScalarVector<T, 3>>& normal) NOEXCEPT {
    return (incident - normal * (T(2) * Dot(incident, normal)));
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TScalarVector<T, 3> Reflect(const TScalarVector<T, 3>& incident, const Meta::TDontDeduce<TScalarVector<T, 3>>& normal, T refractionIndex) NOEXCEPT {
    const T N_dot_I = Dot(normal, incident);
    const T k = 1 - refractionIndex * refractionIndex * (1 - N_dot_I * N_dot_I);
    Assert(k >= 0);
    return (incident * refractionIndex - normal * (refractionIndex * N_dot_I + Sqrt(k)));
}

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Function to predicate struct
//----------------------------------------------------------------------------
#define PPE_SCALARVECTOR_FUNCTION_PREDICATE(_Func) \
    namespace details::predicate { \
        struct F##_Func { \
            template <typename... _Args> \
            CONSTEXPR auto operator ()(_Args... args) const { \
                return _Func(std::forward<_Args>(args)...); \
            } \
        }; \
    } //!details
//----------------------------------------------------------------------------
// Unary functions
//----------------------------------------------------------------------------
#define PPE_SCALARVECTOR_UNARY_FUNC(_Func) \
    PPE_SCALARVECTOR_FUNCTION_PREDICATE(_Func) \
    template <typename T, size_t _Dim, typename _Expr> \
    NODISCARD CONSTEXPR auto _Func(const TScalarVectorExpr<T, _Dim, _Expr>& v) { \
        return details::MakeScalarVectorUnaryExpr<details::predicate::F##_Func>(v); \
    }
//----------------------------------------------------------------------------
PPE_SCALARVECTOR_UNARY_FUNC(Abs)
PPE_SCALARVECTOR_UNARY_FUNC(Frac)
PPE_SCALARVECTOR_UNARY_FUNC(Fractional)
PPE_SCALARVECTOR_UNARY_FUNC(Saturate)
PPE_SCALARVECTOR_UNARY_FUNC(Sign)
PPE_SCALARVECTOR_UNARY_FUNC(Rcp)
PPE_SCALARVECTOR_UNARY_FUNC(RSqrt)
PPE_SCALARVECTOR_UNARY_FUNC(RSqrt_Low)
PPE_SCALARVECTOR_UNARY_FUNC(Sqrt)
PPE_SCALARVECTOR_UNARY_FUNC(CeilToFloat)
PPE_SCALARVECTOR_UNARY_FUNC(FloorToFloat)
PPE_SCALARVECTOR_UNARY_FUNC(RoundToFloat)
PPE_SCALARVECTOR_UNARY_FUNC(TruncToFloat)
PPE_SCALARVECTOR_UNARY_FUNC(CeilToInt)
PPE_SCALARVECTOR_UNARY_FUNC(FloorToInt)
PPE_SCALARVECTOR_UNARY_FUNC(RoundToInt)
PPE_SCALARVECTOR_UNARY_FUNC(TruncToInt)
PPE_SCALARVECTOR_UNARY_FUNC(CeilToUnsigned)
PPE_SCALARVECTOR_UNARY_FUNC(FloorToUnsigned)
PPE_SCALARVECTOR_UNARY_FUNC(RoundToUnsigned)
PPE_SCALARVECTOR_UNARY_FUNC(TruncToUnsigned)
PPE_SCALARVECTOR_UNARY_FUNC(Degrees)
PPE_SCALARVECTOR_UNARY_FUNC(Radians)
PPE_SCALARVECTOR_UNARY_FUNC(ClampAngle)
PPE_SCALARVECTOR_UNARY_FUNC(NormalizeAngle)
PPE_SCALARVECTOR_UNARY_FUNC(Float01_to_FloatM11)
PPE_SCALARVECTOR_UNARY_FUNC(FloatM11_to_Float01)
#undef PPE_SCALARVECTOR_UNARY_FUNC
//----------------------------------------------------------------------------
// binary functions
//----------------------------------------------------------------------------
#define PPE_SCALARVECTOR_BINARY_FUNC(_Func) \
    PPE_SCALARVECTOR_FUNCTION_PREDICATE(_Func) \
    template <typename T, size_t _Dim, typename _Lhs, typename _Rhs> \
    NODISCARD CONSTEXPR auto _Func(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs ) { \
        return details::MakeScalarVectorBinaryExpr<details::predicate::F##_Func>(lhs, rhs); \
    } \
    template <typename T, size_t _Dim, typename _Expr> \
    NODISCARD CONSTEXPR auto _Func(const TScalarVectorExpr<T, _Dim, _Expr>& lhs, const T& rhs) { \
        return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR { \
            return details::MakeScalarVector(_Func(lhs.template get<idx>(), rhs)...); \
        }); \
    } \
    template <typename T, size_t _Dim, typename _Expr> \
    NODISCARD CONSTEXPR auto _Func(const T& lhs, const TScalarVectorExpr<T, _Dim, _Expr>& rhs ) { \
        return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR { \
            return details::MakeScalarVector(_Func(lhs, rhs.template get<idx>())...); \
        }); \
    }
//----------------------------------------------------------------------------
PPE_SCALARVECTOR_BINARY_FUNC(FMod)
PPE_SCALARVECTOR_BINARY_FUNC(Hypot)
PPE_SCALARVECTOR_BINARY_FUNC(IntDivCeil)
PPE_SCALARVECTOR_BINARY_FUNC(IntDivFloor)
PPE_SCALARVECTOR_BINARY_FUNC(IntDivRound)
PPE_SCALARVECTOR_BINARY_FUNC(Min)
PPE_SCALARVECTOR_BINARY_FUNC(Max)
PPE_SCALARVECTOR_BINARY_FUNC(Pow)
PPE_SCALARVECTOR_BINARY_FUNC(Step)
#undef PPE_SCALARVECTOR_BINARY_FUNC
//----------------------------------------------------------------------------
// ternary functions
//----------------------------------------------------------------------------
#define PPE_SCALARVECTOR_TERNARY_FUNC(_Func) \
    PPE_SCALARVECTOR_FUNCTION_PREDICATE(_Func) \
    template <typename T, size_t _Dim, typename _A, typename _B, typename _C> \
    NODISCARD CONSTEXPR auto _Func(const TScalarVectorExpr<T, _Dim, _A>& a, const TScalarVectorExpr<T, _Dim, _B>& b, const TScalarVectorExpr<T, _Dim, _C>& c ) { \
        return details::MakeScalarVectorTernaryExpr<details::predicate::F##_Func>(a, b, c); \
    } \
    template <typename T, size_t _Dim, typename _A, typename _B> \
    NODISCARD CONSTEXPR auto _Func(const TScalarVectorExpr<T, _Dim, _A>& a, const TScalarVectorExpr<T, _Dim, _B>& b, const T& c) { \
        return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR { \
            return details::MakeScalarVector(_Func(a.template get<idx>(), b.template get<idx>(), c)...); \
        }); \
    } \
    template <typename T, size_t _Dim, typename _A, typename _C> \
    NODISCARD CONSTEXPR auto _Func(const TScalarVectorExpr<T, _Dim, _A>& a, const T& b, const TScalarVectorExpr<T, _Dim, _C>& c) { \
        return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR { \
            return details::MakeScalarVector(_Func(a.template get<idx>(), b, c.template get<idx>())...); \
        }); \
    } \
    template <typename T, size_t _Dim, typename _B, typename _C> \
    NODISCARD CONSTEXPR auto _Func(const T& a, const TScalarVectorExpr<T, _Dim, _B>& b, const TScalarVectorExpr<T, _Dim, _C>& c) { \
        return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR { \
            return details::MakeScalarVector(_Func(a, b.template get<idx>(), c.template get<idx>())...); \
        }); \
    } \
    template <typename T, size_t _Dim, typename _C> \
    NODISCARD CONSTEXPR auto _Func(const T& a, const T& b, const TScalarVectorExpr<T, _Dim, _C>& c) { \
        return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR { \
            return details::MakeScalarVector( _Func(a, b, c.template get<idx>())...); \
        }); \
    } \
    template <typename T, size_t _Dim, typename _B> \
    NODISCARD CONSTEXPR auto _Func(const T& a, const TScalarVectorExpr<T, _Dim, _B>& b, const T& c) { \
        return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR { \
            return details::MakeScalarVector(_Func(a, b.template get<idx>(), c)...); \
        }); \
    } \
    template <typename T, size_t _Dim, typename _A> \
    NODISCARD CONSTEXPR auto _Func(const TScalarVectorExpr<T, _Dim, _A>& a, const T& b, const T& c) { \
        return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR { \
            return details::MakeScalarVector(_Func(a.template get<idx>(), b, c)...); \
        }); \
    }
//----------------------------------------------------------------------------
PPE_SCALARVECTOR_TERNARY_FUNC(BiasScale)
PPE_SCALARVECTOR_TERNARY_FUNC(Clamp)
PPE_SCALARVECTOR_TERNARY_FUNC(Lerp)
//PPE_SCALARVECTOR_TERNARY_FUNC(LinearStep)
PPE_SCALARVECTOR_TERNARY_FUNC(Min3)
PPE_SCALARVECTOR_TERNARY_FUNC(Max3)
PPE_SCALARVECTOR_TERNARY_FUNC(SLerp)
PPE_SCALARVECTOR_TERNARY_FUNC(SMin)
PPE_SCALARVECTOR_TERNARY_FUNC(Smoothstep)
PPE_SCALARVECTOR_TERNARY_FUNC(Smootherstep)
#undef PPE_SCALARVECTOR_TERNARY_FUNC
//----------------------------------------------------------------------------
#undef PPE_SCALARVECTOR_FUNCTION_PREDICATE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// XXXMask()
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR auto GreaterMask(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return TScalarVectorExpr<T, _Dim>::Map(Meta::TGreater<T>{}, lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR auto GreaterEqualMask(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return TScalarVectorExpr<T, _Dim>::Map(Meta::TGreaterEqual<T>{}, lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR auto LessMask(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return TScalarVectorExpr<T, _Dim>::Map(Meta::TLess<T>{}, lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR auto LessEqualMask(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return TScalarVectorExpr<T, _Dim>::Map(Meta::TLessEqual<T>{}, lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR auto EqualMask(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return TScalarVectorExpr<T, _Dim>::Map(Meta::TEqualTo<T>{}, lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR auto NotEqualMask(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return TScalarVectorExpr<T, _Dim>::Map(Meta::TNotEqual<T>{}, lhs, rhs);
}
//----------------------------------------------------------------------------
// AllXXX()
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AllGreater(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return All(GreaterMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AllGreaterEqual(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return All(GreaterEqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AllLess(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return All(LessMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AllLessEqual(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return All(LessEqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AllEqual(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return All(EqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AllNotEqual(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return All(NotEqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
// AnyXXX()
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AnyGreater(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return Any(GreaterMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AnyGreaterEqual(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return Any(GreaterEqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AnyLess(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return Any(LessMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AnyLessEqual(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return Any(LessEqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AnyEqual(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return Any(EqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AnyNotEqual(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return Any(NotEqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
// Blend
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _IfTrue, typename _IfFalse, typename _Mask>
CONSTEXPR auto Blend(const TScalarVectorExpr<T, _Dim, _IfTrue>& ifTrue, const TScalarVectorExpr<T, _Dim, _IfFalse>& ifFalse, const TScalarVectorExpr<bool, _Dim, _Mask>& mask) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) {
        return details::MakeScalarVector( Blend(ifTrue.template get<idx>(), ifFalse.template get<idx>(), mask.template get<idx>())... );
    });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _IfTrue, typename _IfFalse>
CONSTEXPR auto Blend(const TScalarVectorExpr<T, _Dim, _IfTrue>& ifTrue, const TScalarVectorExpr<T, _Dim, _IfFalse>& ifFalse, bool mask) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) {
        return details::MakeScalarVector( Blend(ifTrue.template get<idx>(), ifFalse.template get<idx>(), mask)... );
      });
}
//----------------------------------------------------------------------------
// BarycentricLerp
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename A, typename B, typename C>
CONSTEXPR auto BarycentricLerp(
    const TScalarVectorExpr<T, _Dim, A>& a,
    const TScalarVectorExpr<T, _Dim, B>& b,
    const TScalarVectorExpr<T, _Dim, C>& c,
    float f0, float f1, float f2 ) NOEXCEPT {
    return TScalarVectorExpr<T, _Dim>::Map([&](const T& ai, const T& bi, const T& ci) {
        return BarycentricLerp(ai, bi, ci, f0, f1, f2);
    },  a, b, c);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename A, typename B, typename C, typename D>
CONSTEXPR auto BarycentricLerp(
    const TScalarVectorExpr<T, _Dim, A>& a,
    const TScalarVectorExpr<T, _Dim, B>& b,
    const TScalarVectorExpr<T, _Dim, C>& c,
    const TScalarVectorExpr<float, _Dim, D>& f) NOEXCEPT {
    return BarycentricLerp(a, b, c, f.x, f.y, f.z);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> GridSnap(Meta::TDontDeduce<const TScalarVector<T, _Dim>>& location, T grid) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> GridSnap(const Meta::TDontDeduce<TScalarVector<T, _Dim>>& location, const TScalarVector<T, _Dim>& grid) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim>
bool IsNormalized(const TScalarVector<float, _Dim>& v, float epsilon = F_Epsilon) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim>
bool NearlyEquals(const TScalarVector<float, _Dim>& a, const Meta::TDontDeduce<TScalarVector<float, _Dim>>& b, float maxRelDiff = F_Epsilon) NOEXCEPT;
//----------------------------------------------------------------------------
TScalarVector<float, 2> SinCos(float angle) NOEXCEPT;
TScalarVector<double, 2> SinCos(double angle) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
bool IsINF(const TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
bool IsNAN(const TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
bool IsNANorINF(const TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<float, _Dim> UByte0255_to_Float01(const TScalarVectorExpr<u8, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<float, _Dim> UByte0255_to_FloatM11(const TScalarVectorExpr<u8, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<u8, _Dim> Float01_to_UByte0255(const TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<u8, _Dim> FloatM11_to_UByte0255(const TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<float, _Dim> UShort065535_to_Float01(const TScalarVectorExpr<u16, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<float, _Dim> UByte065535_to_FloatM11(const TScalarVectorExpr<u16, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<u16, _Dim> Float01_to_UShort065535(const TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<u16, _Dim> FloatM11_to_UShort065535(const TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Expr>
TScalarVector<u8, 4> Float3M11_to_UByte4N(const TScalarVectorExpr<float, 3, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Expr>
TScalarVector<float, 3> UByte4N_to_Float3M11(const TScalarVectorExpr<u8, 4, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<float, _Dim> Float01_to_FloatM11(const TScalarVectorExpr<float, _Dim, _Expr>& v_01) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<float, _Dim> FloatM11_to_Float01(const TScalarVectorExpr<float, _Dim, _Expr>& v_M11) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void SwapEndiannessInPlace(TScalarVector<T, _Dim>* value) NOEXCEPT {
    FPlatformEndian::SwapInPlace(value->data);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/ScalarVectorHelpers-inl.h"
