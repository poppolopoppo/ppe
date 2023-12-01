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
// Unary functions
//----------------------------------------------------------------------------
PPE_SCALARVECTOR_UNARYOP_FUNC(Abs)
PPE_SCALARVECTOR_UNARYOP_FUNC(Frac)
PPE_SCALARVECTOR_UNARYOP_FUNC(Fractional)
PPE_SCALARVECTOR_UNARYOP_FUNC(Saturate)
PPE_SCALARVECTOR_UNARYOP_FUNC(Sign)
PPE_SCALARVECTOR_UNARYOP_FUNC(Rcp)
PPE_SCALARVECTOR_UNARYOP_FUNC(RSqrt)
PPE_SCALARVECTOR_UNARYOP_FUNC(RSqrt_Low)
PPE_SCALARVECTOR_UNARYOP_FUNC(Sqrt)
PPE_SCALARVECTOR_UNARYOP_FUNC(SStep)
PPE_SCALARVECTOR_UNARYOP_FUNC(CeilToFloat)
PPE_SCALARVECTOR_UNARYOP_FUNC(FloorToFloat)
PPE_SCALARVECTOR_UNARYOP_FUNC(RoundToFloat)
PPE_SCALARVECTOR_UNARYOP_FUNC(TruncToFloat)
PPE_SCALARVECTOR_UNARYOP_FUNC(CeilToInt)
PPE_SCALARVECTOR_UNARYOP_FUNC(FloorToInt)
PPE_SCALARVECTOR_UNARYOP_FUNC(RoundToInt)
PPE_SCALARVECTOR_UNARYOP_FUNC(TruncToInt)
PPE_SCALARVECTOR_UNARYOP_FUNC(CeilToUnsigned)
PPE_SCALARVECTOR_UNARYOP_FUNC(FloorToUnsigned)
PPE_SCALARVECTOR_UNARYOP_FUNC(RoundToUnsigned)
PPE_SCALARVECTOR_UNARYOP_FUNC(TruncToUnsigned)
PPE_SCALARVECTOR_UNARYOP_FUNC(Degrees)
PPE_SCALARVECTOR_UNARYOP_FUNC(Radians)
PPE_SCALARVECTOR_UNARYOP_FUNC(ClampAngle)
PPE_SCALARVECTOR_UNARYOP_FUNC(NormalizeAngle)
PPE_SCALARVECTOR_UNARYOP_FUNC(Float01_to_FloatM11)
PPE_SCALARVECTOR_UNARYOP_FUNC(FloatM11_to_Float01)
//----------------------------------------------------------------------------
// Binary functions
//----------------------------------------------------------------------------
PPE_SCALARVECTOR_BINARYOP_FUNC(FMod)
PPE_SCALARVECTOR_BINARYOP_FUNC(GridSnap)
PPE_SCALARVECTOR_BINARYOP_FUNC(Hypot)
PPE_SCALARVECTOR_BINARYOP_FUNC(IntDivCeil)
PPE_SCALARVECTOR_BINARYOP_FUNC(IntDivFloor)
PPE_SCALARVECTOR_BINARYOP_FUNC(IntDivRound)
PPE_SCALARVECTOR_BINARYOP_FUNC(Min)
PPE_SCALARVECTOR_BINARYOP_FUNC(Max)
PPE_SCALARVECTOR_BINARYOP_FUNC(Pow)
PPE_SCALARVECTOR_BINARYOP_FUNC(Step)
//----------------------------------------------------------------------------
// Ternary functions
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _A, typename _B, typename _C, typename _Op>
struct TScalarVectorTernaryOp : private _Op {
    TScalarVectorExpr<T, _Dim, _A> A;
    TScalarVectorExpr<T, _Dim, _B> B;
    TScalarVectorExpr<T, _Dim, _C> C;

    CONSTEXPR explicit TScalarVectorTernaryOp(
        TScalarVectorExpr<T, _Dim, _A>&& __restrict a,
        TScalarVectorExpr<T, _Dim, _B>&& __restrict b,
        TScalarVectorExpr<T, _Dim, _C>&& __restrict c,
        _Op&& ternaryOp)
        : _Op(std::move(ternaryOp))
        , A(std::move(a)), B(std::move(b)), C(std::move(c))
    {}

    CONSTEXPR auto Get(u32 index) const {
        return _Op::operator ()(A.Get(index), B.Get(index), C.Get(index));
    }
};
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _A, typename _B, typename _C, typename _Op>
NODISCARD inline CONSTEXPR auto MakeScalarVectorOp(
    TScalarVectorExpr<T, _Dim, _A>&& __restrict a,
    TScalarVectorExpr<T, _Dim, _B>&& __restrict b,
    TScalarVectorExpr<T, _Dim, _C>&& __restrict c,
    _Op&& ternaryOp) {
    using op_type = TScalarVectorTernaryOp<T, _Dim, _A, _B, _C, _Op>;
    using destination_type = decltype(ternaryOp(std::declval<T>(), std::declval<T>(), std::declval<T>()));
    return TScalarVectorExpr<destination_type, _Dim, op_type>{ std::move(a), std::move(b), std::move(c), std::move(ternaryOp) };
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
#define PPE_SCALARVECTOR_TERNARYOP(_FunctionName, ...) \
    template <typename T, u32 _Dim, typename _A, typename _B, typename _C> \
    NODISCARD inline CONSTEXPR  auto _FunctionName(const details::TScalarVectorExpr<T, _Dim, _A>& a, const details::TScalarVectorExpr<T, _Dim, _B>& b, const details::TScalarVectorExpr<T, _Dim, _C>& c) { \
        return MakeScalarVectorOp(MakeScalarVectorRef(a), MakeScalarVectorRef(b), MakeScalarVectorRef(c), __VA_ARGS__); \
    } \
    template <typename T, u32 _Dim, typename _A, typename _B> \
    NODISCARD inline CONSTEXPR  auto _FunctionName(const details::TScalarVectorExpr<T, _Dim, _A>& a, const details::TScalarVectorExpr<T, _Dim, _B>& b, T c) { \
        return MakeScalarVectorOp(MakeScalarVectorRef(a), MakeScalarVectorRef(b), details::MakeScalarVectorLiteral<_Dim>(std::move(c)), __VA_ARGS__); \
    } \
    template <typename T, u32 _Dim, typename _A, typename _C> \
    NODISCARD inline CONSTEXPR  auto _FunctionName(const details::TScalarVectorExpr<T, _Dim, _A>& a, T b, const details::TScalarVectorExpr<T, _Dim, _C>& c) { \
        return MakeScalarVectorOp(MakeScalarVectorRef(a), details::MakeScalarVectorLiteral<_Dim>(std::move(b)), MakeScalarVectorRef(c), __VA_ARGS__); \
    } \
    template <typename T, u32 _Dim, typename _B, typename _C> \
    NODISCARD inline CONSTEXPR  auto _FunctionName(T a, const details::TScalarVectorExpr<T, _Dim, _B>& b, const details::TScalarVectorExpr<T, _Dim, _C>& c) { \
        return MakeScalarVectorOp(details::MakeScalarVectorLiteral<_Dim>(std::move(a)), MakeScalarVectorRef(b), MakeScalarVectorRef(c), __VA_ARGS__); \
    } \
    template <typename T, u32 _Dim, typename _A> \
    NODISCARD inline CONSTEXPR  auto _FunctionName(const details::TScalarVectorExpr<T, _Dim, _A>& a, T b, T c) { \
        return MakeScalarVectorOp(MakeScalarVectorRef(a), details::MakeScalarVectorLiteral<_Dim>(std::move(b)), details::MakeScalarVectorLiteral<_Dim>(std::move(c)), __VA_ARGS__); \
    } \
    template <typename T, u32 _Dim, typename _B> \
    NODISCARD inline CONSTEXPR  auto _FunctionName(T a, const details::TScalarVectorExpr<T, _Dim, _B>& b, T c) { \
        return MakeScalarVectorOp(details::MakeScalarVectorLiteral<_Dim>(std::move(a)), MakeScalarVectorRef(b), details::MakeScalarVectorLiteral<_Dim>(std::move(c)), __VA_ARGS__); \
    } \
    template <typename T, u32 _Dim, typename _C> \
    NODISCARD inline CONSTEXPR  auto _FunctionName(T a, T b, const details::TScalarVectorExpr<T, _Dim, _C>& c) { \
        return MakeScalarVectorOp(details::MakeScalarVectorLiteral<_Dim>(std::move(a)), details::MakeScalarVectorLiteral<_Dim>(std::move(b)), MakeScalarVectorRef(c), __VA_ARGS__); \
    }
#define PPE_SCALARVECTOR_TERNARYOP_FUNC(_FunctionName) \
    PPE_SCALARVECTOR_TERNARYOP(_FunctionName, [](const T& _a, const T& _b, const T& _c) { return PPE::_FunctionName(_a, _b, _c); })
//----------------------------------------------------------------------------
PPE_SCALARVECTOR_TERNARYOP_FUNC(BiasScale)
PPE_SCALARVECTOR_TERNARYOP_FUNC(Clamp)
PPE_SCALARVECTOR_TERNARYOP_FUNC(Lerp)
//PPE_SCALARVECTOR_TERNARYOP_FUNC(LinearStep)
PPE_SCALARVECTOR_TERNARYOP_FUNC(Min3)
PPE_SCALARVECTOR_TERNARYOP_FUNC(Max3)
PPE_SCALARVECTOR_TERNARYOP_FUNC(SLerp)
PPE_SCALARVECTOR_TERNARYOP_FUNC(SMin)
PPE_SCALARVECTOR_TERNARYOP_FUNC(Smoothstep)
PPE_SCALARVECTOR_TERNARYOP_FUNC(Smootherstep)
//----------------------------------------------------------------------------
#undef PPE_SCALARVECTOR_TERNARYOP
#undef PPE_SCALARVECTOR_TERNARYOP_FUNC
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Any / All
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
NODISCARD CONSTEXPR bool Any(const details::TScalarVectorExpr<bool, _Dim, _Expr>& v) {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR {
        return (v.template Get<idx>() || ...);
    });
}
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
NODISCARD CONSTEXPR bool All(const details::TScalarVectorExpr<bool, _Dim, _Expr>& v) {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR {
        return (v.template Get<idx>() && ...);
    });
}
//----------------------------------------------------------------------------
// Dot
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR T Dot(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) {
    return (lhs * rhs).HSum();
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Expr>
NODISCARD CONSTEXPR T Dot2(const details::TScalarVectorExpr<T, _Dim, _Expr>& v) {
    return Dot(v, v);
}
//----------------------------------------------------------------------------
// Length
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Expr>
NODISCARD CONSTEXPR T LengthSq(const details::TScalarVectorExpr<T, _Dim, _Expr>& v) {
    return Dot2(v);
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Expr>
NODISCARD CONSTEXPR auto Length(const details::TScalarVectorExpr<T, _Dim, _Expr>& v) {
    return Sqrt(LengthSq(v));
}
//----------------------------------------------------------------------------
// Distance
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename A, typename B>
NODISCARD CONSTEXPR T DistanceSq(const details::TScalarVectorExpr<T, _Dim, A>& a, const details::TScalarVectorExpr<T, _Dim, B>& b) {
    return LengthSq(b - a);
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename A, typename B>
NODISCARD CONSTEXPR auto Distance(const details::TScalarVectorExpr<T, _Dim, A>& a, const details::TScalarVectorExpr<T, _Dim, B>& b) {
    return Length(b - a);
}
//----------------------------------------------------------------------------
// Normalize
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Expr>
CONSTEXPR auto Normalize(const details::TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT {
#if USE_PPE_ASSERT
    const T norm = Length(v);
    Assert(norm);
    return (v / norm);
#else
    return (v * RSqrt(LengthSq(v)));
#endif
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Expr>
CONSTEXPR TScalarVector<T, _Dim> SafeNormalize(const details::TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT {
    const T normSQ = LengthSq(v);
    if (Likely(normSQ > EpsilonSQ))
        return (v * RSqrt(normSQ));

    TScalarVector<T, _Dim> safe(v);
    const u32 maxAxis = Abs(safe).MaxComponentIndex();
    safe[maxAxis] = static_cast<T>(v[maxAxis] > 0 ? 1 : -1);
    return safe;
}
//----------------------------------------------------------------------------
template <u32 _Dim>
bool IsNormalized(const TScalarVector<float, _Dim>& v, float epsilon = Epsilon) NOEXCEPT;
//----------------------------------------------------------------------------
template <u32 _Dim, typename _A, typename _B>
bool NearlyEquals(const details::TScalarVectorExpr<float, _Dim, _A>& a, const details::TScalarVectorExpr<float, _Dim, _B>& b, float maxRelDiff = Epsilon) NOEXCEPT;
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
CONSTEXPR T Cross(const TScalarVector<T, 2>& o, const TScalarVector<T, 2>& a, const TScalarVector<T, 2>& b) NOEXCEPT {
    // Det(a - o, b - o) <=> Twiced signed area of the triangle
    return (((a.x - o.x) * (b.y - o.y)) -
            ((a.y - o.y) * (b.x - o.x)) );
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TScalarVector<T, 3> Cross(const TScalarVector<T, 3>& a, const TScalarVector<T, 3>& b) NOEXCEPT {
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x };
}
//----------------------------------------------------------------------------
// Reflect / Refract
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TScalarVector<T, 3> Reflect(const TScalarVector<T, 3>& incident, const TScalarVector<T, 3>& normal) NOEXCEPT {
    return (incident - normal * (T(2) * Dot(incident, normal)));
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TScalarVector<T, 3> Reflect(const TScalarVector<T, 3>& incident, const TScalarVector<T, 3>& normal, T refractionIndex) NOEXCEPT {
    const T N_dot_I = Dot(normal, incident);
    const T k = 1 - refractionIndex * refractionIndex * (1 - N_dot_I * N_dot_I);
    Assert(k >= 0);
    return (incident * refractionIndex - normal * (refractionIndex * N_dot_I + Sqrt(k)));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// AllXXX()
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AllGreater(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return All(GreaterMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AllGreaterEqual(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return All(GreaterEqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AllLess(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return All(LessMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AllLessEqual(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return All(LessEqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AllEqual(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return All(EqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AllNotEqual(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return All(NotEqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
// AnyXXX()
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AnyGreater(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return Any(GreaterMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AnyGreaterEqual(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return Any(GreaterEqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AnyLess(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return Any(LessMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AnyLessEqual(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return Any(LessEqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AnyEqual(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return Any(EqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR bool AnyNotEqual(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return Any(NotEqualMask(lhs, rhs));
}
//----------------------------------------------------------------------------
// Blend
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _IfTrue, typename _IfFalse, typename _Mask>
CONSTEXPR auto Blend(const details::TScalarVectorExpr<T, _Dim, _IfTrue>& ifTrue, const details::TScalarVectorExpr<T, _Dim, _IfFalse>& ifFalse, const details::TScalarVectorExpr<bool, _Dim, _Mask>& mask) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) {
        return TScalarVector<T, _Dim>( Blend(ifTrue.template Get<idx>(), ifFalse.template Get<idx>(), mask.template Get<idx>())... );
    });
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _IfTrue, typename _IfFalse>
CONSTEXPR auto Blend(const details::TScalarVectorExpr<T, _Dim, _IfTrue>& ifTrue, const details::TScalarVectorExpr<T, _Dim, _IfFalse>& ifFalse, bool mask) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) {
        return TScalarVector<T, _Dim>( Blend(ifTrue.template Get<idx>(), ifFalse.template Get<idx>(), mask)... );
    });
}
//----------------------------------------------------------------------------
// BarycentricLerp
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename A, typename B, typename C>
CONSTEXPR auto BarycentricLerp(
    const details::TScalarVectorExpr<T, _Dim, A>& a,
    const details::TScalarVectorExpr<T, _Dim, B>& b,
    const details::TScalarVectorExpr<T, _Dim, C>& c,
    float f0, float f1, float f2 ) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) {
        return TScalarVector<T, _Dim>(BarycentricLerp(
            a.template Get<idx>(), b.template Get<idx>(), c.template Get<idx>(),
            f0, f1, f2)...);
    });
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename A, typename B, typename C, typename D>
CONSTEXPR auto BarycentricLerp(
    const details::TScalarVectorExpr<T, _Dim, A>& a,
    const details::TScalarVectorExpr<T, _Dim, B>& b,
    const details::TScalarVectorExpr<T, _Dim, C>& c,
    const details::TScalarVectorExpr<float, 3, D>& f) NOEXCEPT {
    return BarycentricLerp(a, b, c, f.template Get<0>(), f.template Get<1>(), f.template Get<2>());
}
//----------------------------------------------------------------------------
// BilateralLerp
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename A, typename B, typename C, typename D>
CONSTEXPR auto BilateralLerp(
    const details::TScalarVectorExpr<T, _Dim, A>& p00,
    const details::TScalarVectorExpr<T, _Dim, B>& p10,
    const details::TScalarVectorExpr<T, _Dim, C>& p11,
    const details::TScalarVectorExpr<T, _Dim, D>& p01,
    float f0, float f1) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) {
        return TScalarVector<T, _Dim>(Lerp(
            Lerp(p00.template Get<idx>(), p01.template Get<idx>(), f1),
            Lerp(p10.template Get<idx>(), p11.template Get<idx>(), f1),
            f0)...);
    });
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename A, typename B, typename C, typename D, typename E>
CONSTEXPR auto BilateralLerp(
    const details::TScalarVectorExpr<T, _Dim, A>& p00,
    const details::TScalarVectorExpr<T, _Dim, B>& p10,
    const details::TScalarVectorExpr<T, _Dim, C>& p11,
    const details::TScalarVectorExpr<T, _Dim, D>& p01,
    const details::TScalarVectorExpr<float, 2, E>& f) NOEXCEPT {
    return BilateralLerp(p00, p10, p11, p01, f.template Get<0>(), f.template Get<1>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TScalarVector<float, 2> SinCos(float angle) NOEXCEPT;
TScalarVector<double, 2> SinCos(double angle) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Expr>
bool IsINF(const details::TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Expr>
bool IsNAN(const details::TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Expr>
bool IsNANorINF(const details::TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<float, _Dim> UByte0255_to_Float01(const details::TScalarVectorExpr<u8, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<float, _Dim> UByte0255_to_FloatM11(const details::TScalarVectorExpr<u8, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<u8, _Dim> Float01_to_UByte0255(const details::TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<u8, _Dim> FloatM11_to_UByte0255(const details::TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<float, _Dim> UShort065535_to_Float01(const details::TScalarVectorExpr<u16, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<float, _Dim> UByte065535_to_FloatM11(const details::TScalarVectorExpr<u16, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<u16, _Dim> Float01_to_UShort065535(const details::TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<u16, _Dim> FloatM11_to_UShort065535(const details::TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Expr>
TScalarVector<u8, 4> Float3M11_to_UByte4N(const details::TScalarVectorExpr<float, 3, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Expr>
TScalarVector<float, 3> UByte4N_to_Float3M11(const details::TScalarVectorExpr<u8, 4, _Expr>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<float, _Dim> Float01_to_FloatM11(const details::TScalarVectorExpr<float, _Dim, _Expr>& v_01) NOEXCEPT;
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<float, _Dim> FloatM11_to_Float01(const details::TScalarVectorExpr<float, _Dim, _Expr>& v_M11) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
void SwapEndiannessInPlace(TScalarVector<T, _Dim>* value) NOEXCEPT {
    FPlatformEndian::SwapInPlace(value->data);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/ScalarVectorHelpers-inl.h"
