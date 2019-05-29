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
// Dot()
//----------------------------------------------------------------------------
template <typename _Expr, size_t _Dim>
CONSTEXPR auto Dot(const TScalarVectorExpr<_Expr, _Dim>& v) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return (v.template get<idx>() + ...);
    });
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR auto Dot(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return Dot(lhs * rhs);
}
//----------------------------------------------------------------------------
// Det()
//----------------------------------------------------------------------------
template <typename _Expr>
CONSTEXPR auto Det(const TScalarVectorExpr<_Expr, 2>& lhs, const TScalarVectorExpr<_Expr, 2>& rhs) NOEXCEPT {
    return (lhs.template get<0>() * rhs.template get<1>() -
            lhs.template get<1>() * rhs.template get<0>() );
}
//----------------------------------------------------------------------------
// Cross()
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR auto Cross(const TScalarVector<T, 2>& o, const Meta::TDontDeduce<TScalarVector<T, 2>>& a, const Meta::TDontDeduce<TScalarVector<T, 2>>& b) NOEXCEPT {
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
// Reflect/Refract()
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
// Distance()
//----------------------------------------------------------------------------
template <typename _Lhs, size_t _Dim, typename _Rhs>
CONSTEXPR auto Distance(const TScalarVectorExpr<_Lhs, _Dim>& a, const TScalarVectorExpr<_Rhs, _Dim>& b) NOEXCEPT {
    return Length(a - b);
}
//----------------------------------------------------------------------------
template <typename _Lhs, size_t _Dim, typename _Rhs>
CONSTEXPR auto DistanceSq(const TScalarVectorExpr<_Lhs, _Dim>& a, const TScalarVectorExpr<_Rhs, _Dim>& b) NOEXCEPT {
    return LengthSq(a - b);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Length/LengthSq/Normalize()
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR auto Length(const TScalarVector<T, _Dim>& v) NOEXCEPT {
    return Sqrt(Dot(v, v));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR auto LengthSq(const TScalarVector<T, _Dim>& v) NOEXCEPT {
    return Dot(v, v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR auto Normalize(const TScalarVector<T, _Dim>& v) NOEXCEPT {
#if USE_PPE_ASSERT
    const T norm = Length(v);
    Assert(norm);
    return (v * (T(1) / norm));
#else
    return (v * RSqrt(LengthSq(v)));
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// MaxComponent()
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T MaxComponent(const TScalarVectorExpr<T, 2>& v) NOEXCEPT {
    return Max(v.template get<0>(), v.template get<1>());
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T MaxComponent(const TScalarVectorExpr<T, 3>& v) NOEXCEPT {
    return Max3(v.template get<0>(), v.template get<1>(), v.template get<2>());
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T MaxComponent(const TScalarVectorExpr<T, 4>& v) NOEXCEPT {
    return Max( Max(v.template get<0>(), v.template get<1>()),
                Max(v.template get<2>(), v.template get<3>()) );
}
//----------------------------------------------------------------------------
// MaxComponentIndex()
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR u32 MaxComponentIndex(const TScalarVectorExpr<T, 2>& v) NOEXCEPT {
    return (v.template get<0>() < v.template get<1>() ? 1 : 0);
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR u32 MaxComponentIndex(const TScalarVectorExpr<T, 3>& v) NOEXCEPT {
    return (CubeMapFaceID(v.template get<0>(), v.template get<1>(), v.template get<2>()) >> 1);
}
//----------------------------------------------------------------------------
// MinComponent()
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T MinComponent(const TScalarVectorExpr<T, 2>& v) NOEXCEPT {
    return Min(v.template get<0>(), v.template get<1>());
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T MinComponent(const TScalarVectorExpr<T, 3>& v) NOEXCEPT {
    return Min3(v.template get<0>(), v.template get<1>(), v.template get<2>());
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T MinComponent(const TScalarVectorExpr<T, 4>& v) NOEXCEPT {
    return Min( Min(v.template get<0>(), v.template get<1>()),
                Min(v.template get<2>(), v.template get<3>()) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// MinMax/MinMax3()
//----------------------------------------------------------------------------
template <typename _A, size_t _Dim, typename _B, typename T>
CONSTEXPR void MinMax(
    const TScalarVectorExpr<_A, _Dim>& a,
    const TScalarVectorExpr<_B, _Dim>& b,
    TScalarVector<T, _Dim>* pmin,
    TScalarVector<T, _Dim>* pmax ) {
    Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        (MinMax( a.template get<idx>(), b.template get<idx>(),
                 &pmin->data[idx], &pmax->data[idx]), ... );
    });
}
//----------------------------------------------------------------------------
template <typename _A, size_t _Dim, typename _B, typename _C, typename T>
CONSTEXPR void MinMax3(
    const TScalarVectorExpr<_A, _Dim>& a,
    const TScalarVectorExpr<_B, _Dim>& b,
    const TScalarVectorExpr<_B, _Dim>& c,
    TScalarVector<T, _Dim>* pmin,
    TScalarVector<T, _Dim>* pmax ) {
    Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        (MinMax3(a.template get<idx>(), b.template get<idx>(), c.template get<idx>(),
                 &pmin->data[idx], &pmax->data[idx]), ... );
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// BitAnd/BitOr/BitXor()
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR auto BitAnd(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT { return (a & b); });
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR auto BitOr(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT { return (a | b); });
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR auto BitXor(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT { return (a ^ b); });
}
//----------------------------------------------------------------------------
// Blend()
//----------------------------------------------------------------------------
template <typename _True, typename _False, typename _Mask>
struct TScalarVectorBlend : TScalarVectorExpr<TScalarVectorBlend<_True, _False, _Mask>, _True::Dim> {
    using TScalarVectorExpr<TScalarVectorBlend<_True, _False, _Mask>, _True::Dim>::Dim;
    STATIC_ASSERT(_True::Dim == _False::Dim);
    STATIC_ASSERT(_True::Dim == _Mask::Dim);

    const _True& ifTrue;
    const _False& ifFalse;
    const _Mask& mask;

    FORCE_INLINE CONSTEXPR TScalarVectorBlend(const _Mask& _mask, const _True& _ifTrue, const _False& _ifFalse) NOEXCEPT
        : ifTrue(_ifTrue)
        , ifFalse(_ifFalse)
        , mask(_mask)
    {}

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR auto get() const NOEXCEPT {
        return (mask.template get<_Idx>() ? ifTrue.template get<_Idx>() : ifFalse.template get<_Idx>());
    }
};
//----------------------------------------------------------------------------
template <typename _True, size_t _Dim, typename _False>
CONSTEXPR auto Blend(const TScalarVectorExpr<_True, _Dim>& ifTrue, const TScalarVectorExpr<_False, _Dim>& ifFalse, bool mask) NOEXCEPT {
    return TScalarVectorBlend(ifTrue, ifFalse, TScalarVectorLiteral<bool, _Dim>(mask));
}
//----------------------------------------------------------------------------
template <typename _True, size_t _Dim, typename _False, typename _Mask>
CONSTEXPR auto Blend(const TScalarVectorExpr<_True, _Dim>& ifTrue, const TScalarVectorExpr<_False, _Dim>& ifFalse, const TScalarVectorExpr<_Mask, _Dim>& mask) NOEXCEPT {
    return TScalarVectorBlend(ifTrue, ifFalse, mask);
}
//----------------------------------------------------------------------------
// AllXXX()
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR bool AllGreater(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return ((lhs.template get<idx>() > rhs.template get<idx>()) & ...);
    });
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR bool AllGreaterEqual(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT{
        return ((lhs.template get<idx>() >= rhs.template get<idx>()) & ...);
    });
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR bool AllLess(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT{
        return ((lhs.template get<idx>() < rhs.template get<idx>()) & ...);
    });
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR bool AllLessEqual(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT{
        return ((lhs.template get<idx>() <= rhs.template get<idx>()) & ...);
    });
}
//----------------------------------------------------------------------------
// XXXMask()
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR auto GreaterMask(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT -> bool { return (a > b); });
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR auto GreaterEqualMask(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT -> bool { return (a >= b); });
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR auto LessMask(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT -> bool { return (a < b); });
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR auto LessEqualMask(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT -> bool { return (a <= b); });
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR auto EqualMask(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT -> bool { return (a == b); });
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR auto NotEqualMask(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT -> bool { return (a != b); });
}
//----------------------------------------------------------------------------
// Shuffle()
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t... _Shuffle>
struct TScalarVectorShuffle : TScalarVectorExpr<TScalarVectorShuffle<_Lhs, _Rhs, _Shuffle...>, sizeof...(_Shuffle)> {
    using TScalarVectorExpr<TScalarVectorShuffle<_Lhs, _Rhs, _Shuffle...>, sizeof...(_Shuffle)>::Dim;

    const _Lhs& lhs;
    const _Rhs& rhs;

    FORCE_INLINE CONSTEXPR TScalarVectorShuffle(const _Lhs& _lhs, const _Rhs& _rhs) NOEXCEPT
        : lhs(_lhs)
        , rhs(_rhs)
    {}

    static CONSTEXPR size_t indices[Dim] = { _Shuffle... };
    template <size_t _Idx>
    static CONSTEXPR size_t shuffled = indices[_Idx];

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR auto shuffle(std::enable_if_t < _Idx < _Lhs::Dim>* = nullptr) const NOEXCEPT {
        return (lhs.template get<_Idx>());
    }
    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR auto shuffle(std::enable_if_t<_Idx >= _Lhs::Dim>* = nullptr) const NOEXCEPT {
        static_assert(_Idx < _Lhs::Dim + _Rhs::Dim, "out of bounds shuffle index");
        return (rhs.template get<_Idx - _Lhs::Dim>());
    }

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR auto get() const NOEXCEPT {
        return (shuffle< shuffled<_Idx> >());
    }
};
//----------------------------------------------------------------------------
template <size_t... _Shuffle, typename _Lhs, size_t _Dim0, typename _Rhs, size_t _Dim1>
CONSTEXPR auto Shuffle(const TScalarVectorExpr<_Lhs, _Dim0>& lhs, const TScalarVectorExpr<_Rhs, _Dim1>& rhs) NOEXCEPT {
    return TScalarVectorShuffle<TScalarVectorExpr<_Lhs, _Dim0>, TScalarVectorExpr<_Rhs, _Dim1>, _Shuffle...>(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Unary functions :
//----------------------------------------------------------------------------
#define PPE_SCALARVECTOR_UNARYFUNC_DEF(_NAME, _FUNC) \
    template <typename _Expr, size_t _Dim> \
    CONSTEXPR auto _NAME(const TScalarVectorExpr<_Expr, _Dim>& v) { \
        using component_type = std::decay_t<decltype(_FUNC(v.template get<0>()))>; \
        return TScalarVectorUnaryOp(v, [](auto x) CONSTEXPR NOEXCEPT { return _FUNC(x); }); \
    }

#define PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(_NAME) PPE_SCALARVECTOR_UNARYFUNC_DEF(_NAME, _NAME)

PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(Abs)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(Frac)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(Fractional)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(Saturate)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(Sign)

PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(Rcp)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(RSqrt)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(RSqrt_Low)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(Sqrt)

PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(CeilToFloat)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(FloorToFloat)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(RoundToFloat)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(TruncToFloat)

PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(CeilToInt)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(FloorToInt)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(RoundToInt)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(TruncToInt)

PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(Degrees)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(Radians)
PPE_SCALARVECTOR_UNARYFUNC_DEF(Cos, FPlatformMaths::Cos)
PPE_SCALARVECTOR_UNARYFUNC_DEF(Sin, FPlatformMaths::Sin)

PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(Float01_to_FloatM11)
PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF(FloatM11_to_Float01)

#undef PPE_SCALARVECTOR_UNARYFUNC_ALIAS_DEF
#undef PPE_SCALARVECTOR_UNARYFUNC_DEF
//----------------------------------------------------------------------------
// Binary functions :
//----------------------------------------------------------------------------
#define PPE_SCALARVECTOR_BINARYFUNC_DEF(_NAME, _FUNC) \
    template <typename T, typename _Expr, size_t _Dim, typename _Result = TScalarComponent<TScalarVectorExpr<_Expr, _Dim>, T> > \
    CONSTEXPR auto _NAME(T a, const TScalarVectorExpr<_Expr, _Dim>& b) { \
        using component_type = std::decay_t<decltype(_FUNC(a, b.template get<0>()))>; \
        return TScalarVectorUnaryOp(b, [a](auto x) CONSTEXPR NOEXCEPT -> _Result { return _FUNC(a, x); }); \
    } \
    template <typename _Expr, size_t _Dim, typename T, typename _Result = TScalarComponent<TScalarVectorExpr<_Expr, _Dim>, T> > \
    CONSTEXPR auto _NAME(const TScalarVectorExpr<_Expr, _Dim>& a, T b) { \
        using component_type = std::decay_t<decltype(_FUNC(a.template get<0>(), b))>; \
        return TScalarVectorUnaryOp(a, [b](auto x) CONSTEXPR NOEXCEPT -> _Result { return _FUNC(x, b); }); \
    } \
    template <typename _Lhs, size_t _Dim, typename _Rhs> \
    CONSTEXPR auto _NAME(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) { \
        using component_type = std::decay_t<decltype(_FUNC(lhs.template get<0>(), rhs.template get<0>()))>; \
        return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT { return _FUNC(a, b); }); \
    }

#define PPE_SCALARVECTOR_BINARYFUNC_ALIAS_DEF(_NAME) PPE_SCALARVECTOR_BINARYFUNC_DEF(_NAME, _NAME)

PPE_SCALARVECTOR_BINARYFUNC_ALIAS_DEF(Hypot)

PPE_SCALARVECTOR_BINARYFUNC_ALIAS_DEF(Max)
PPE_SCALARVECTOR_BINARYFUNC_ALIAS_DEF(Min)

PPE_SCALARVECTOR_BINARYFUNC_ALIAS_DEF(FMod)
PPE_SCALARVECTOR_BINARYFUNC_ALIAS_DEF(Pow)

PPE_SCALARVECTOR_BINARYFUNC_ALIAS_DEF(Step)

#undef PPE_SCALARVECTOR_BINARYFUNC_ALIAS_DEF
#undef PPE_SCALARVECTOR_BINARYFUNC_DEF
//----------------------------------------------------------------------------
// Ternary functions :
//----------------------------------------------------------------------------
#define PPE_SCALARVECTOR_TERNARYFUNC_DEF(_NAME, _FUNC) \
    template <typename T, typename _B, size_t _Dim, typename _C, typename _Result = TScalarComponent<TScalarVectorExpr<_B, _Dim>, T> > \
    CONSTEXPR auto _NAME(T a, const TScalarVectorExpr<_B, _Dim>& b, const TScalarVectorExpr<_C, _Dim>& c) { \
        using component_type = std::decay_t<decltype(_FUNC(a, b.template get<0>(), c.template get<0>()))>; \
        return TScalarVectorBinaryOp(b, c, [a](auto y, auto z) CONSTEXPR NOEXCEPT -> _Result { return _FUNC(a, y, z); }); \
    } \
    template <typename _A, size_t _Dim, typename T, typename _C, typename _Result = TScalarComponent<TScalarVectorExpr<_A, _Dim>, T> > \
    CONSTEXPR auto _NAME(const TScalarVectorExpr<_A, _Dim>& a, T b, const TScalarVectorExpr<_C, _Dim>& c) { \
        using component_type = std::decay_t<decltype(_FUNC(a.template get<0>(), b, c.template get<0>()))>; \
        return TScalarVectorBinaryOp(a, c, [b](auto x, auto z) CONSTEXPR NOEXCEPT -> _Result { return _FUNC(x, b, z); }); \
    } \
    template <typename _A, size_t _Dim, typename _B, typename T, typename _Result = TScalarComponent<TScalarVectorExpr<_A, _Dim>, T> > \
    CONSTEXPR auto _NAME(const TScalarVectorExpr<_A, _Dim>& a, const TScalarVectorExpr<_B, _Dim>& b, T c) { \
        using component_type = std::decay_t<decltype(_FUNC(a.template get<0>(), b.template get<0>(), c))>; \
        return TScalarVectorBinaryOp(a, b, [c](auto x, auto y) CONSTEXPR NOEXCEPT -> _Result { return _FUNC(x, y, c); }); \
    } \
    template <typename T, typename _C, size_t _Dim, typename _Result = TScalarComponent<TScalarVectorExpr<_C, _Dim>, T> > \
    CONSTEXPR auto _NAME(T a, T b, const TScalarVectorExpr<_C, _Dim>& c) { \
        using component_type = std::decay_t<decltype(_FUNC(a, b, c.template get<0>()))>; \
        return TScalarVectorUnaryOp(c, [a, b](auto z) CONSTEXPR NOEXCEPT -> _Result { return _FUNC(a, b, z); }); \
    } \
    template <typename T, typename _B, size_t _Dim, typename _Result = TScalarComponent<TScalarVectorExpr<_B, _Dim>, T> > \
    CONSTEXPR auto _NAME(T a, const TScalarVectorExpr<_B, _Dim>& b, T c) { \
        using component_type = std::decay_t<decltype(_FUNC(a, b.template get<0>(), c))>; \
        return TScalarVectorUnaryOp(b, [a, c](auto y) CONSTEXPR NOEXCEPT -> _Result { return _FUNC(a, y, c); }); \
    } \
    template <typename _A, size_t _Dim, typename T, typename _Result = TScalarComponent<TScalarVectorExpr<_A, _Dim>, T> > \
    CONSTEXPR auto _NAME(const TScalarVectorExpr<_A, _Dim>& a, T b, T c) { \
        using component_type = std::decay_t<decltype(_FUNC(a.template get<0>(), b, c))>; \
        return TScalarVectorUnaryOp(a, [b, c](auto x) CONSTEXPR NOEXCEPT -> _Result { return _FUNC(x, b, c); }); \
    } \
    template <typename _A, size_t _Dim, typename _B, typename _C> \
    CONSTEXPR auto _NAME(const TScalarVectorExpr<_A, _Dim>& a, const TScalarVectorExpr<_B, _Dim>& b, const TScalarVectorExpr<_C, _Dim>& c) { \
        using component_type = std::decay_t<decltype(_FUNC(a.template get<0>(), b.template get<0>(), c.template get<0>()))>; \
        return TScalarVectorTernaryOp(a, b, c, [](auto x, auto y, auto z) CONSTEXPR NOEXCEPT { return _FUNC(x, y, z); }); \
    }

#define PPE_SCALARVECTOR_TERNARYFUNC_ALIAS_DEF(_NAME) PPE_SCALARVECTOR_TERNARYFUNC_DEF(_NAME, _NAME)

template <typename _A, typename _B, typename _C, typename _Op>
struct TScalarVectorTernaryOp : TScalarVectorExpr<TScalarVectorTernaryOp<_A, _B, _C, _Op>, _A::Dim> {
    using TScalarVectorExpr<TScalarVectorTernaryOp<_A, _B, _C, _Op>, _A::Dim>::Dim;
    STATIC_ASSERT(_A::Dim == _B::Dim);
    STATIC_ASSERT(_A::Dim == _C::Dim);

    const _A& a;
    const _B& b;
    const _C& c;
    _Op op;

    FORCE_INLINE CONSTEXPR TScalarVectorTernaryOp(const _A& _a, const _B& _b, const _C& _c, _Op&& _op) NOEXCEPT
        : a(_a)
        , b(_b)
        , c(_c)
        , op(std::move(_op))
    {}

    using component_type = std::decay_t<decltype(
        std::declval<_Op>()(
            std::declval<const _A&>().template get<0>(),
            std::declval<const _B&>().template get<0>(),
            std::declval<const _C&>().template get<0>() )
    )>;

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR auto get() const NOEXCEPT {
        return op(a.template get<_Idx>(), b.template get<_Idx>(), c.template get<_Idx>());
    }
};

PPE_SCALARVECTOR_TERNARYFUNC_ALIAS_DEF(BiasScale)

PPE_SCALARVECTOR_TERNARYFUNC_ALIAS_DEF(Clamp)

PPE_SCALARVECTOR_TERNARYFUNC_ALIAS_DEF(Lerp)
PPE_SCALARVECTOR_TERNARYFUNC_ALIAS_DEF(LinearStep)
PPE_SCALARVECTOR_TERNARYFUNC_ALIAS_DEF(SLerp)

PPE_SCALARVECTOR_TERNARYFUNC_ALIAS_DEF(Smoothstep)
PPE_SCALARVECTOR_TERNARYFUNC_ALIAS_DEF(Smootherstep)

PPE_SCALARVECTOR_TERNARYFUNC_ALIAS_DEF(SMin)

#undef PPE_SCALARVECTOR_TERNARYFUNC_ALIAS_DEF
#undef PPE_SCALARVECTOR_TERNARYFUNC_DEF
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _A, size_t _Dim, typename _B, typename _C, typename U>
CONSTEXPR auto BarycentricLerp(
    const TScalarVectorExpr<_A, _Dim>& a,
    const TScalarVectorExpr<_B, _Dim>& b,
    const TScalarVectorExpr<_C, _Dim>& c,
    U f0, U f1, U f2 ) NOEXCEPT {
    return TScalarVectorTernaryOp(a, b, c, [f0, f1, f2](auto x, auto y, auto z) CONSTEXPR NOEXCEPT {
        return BarycentricLerp(x, y, z, f0, f1, f2);
    });
}
//----------------------------------------------------------------------------
template <typename _A, size_t _Dim, typename _B, typename _C, typename U>
CONSTEXPR auto BarycentricLerp(
    const TScalarVectorExpr<_A, _Dim>& a,
    const TScalarVectorExpr<_B, _Dim>& b,
    const TScalarVectorExpr<_C, _Dim>& c,
    const TScalarVector<U, 3>& f ) NOEXCEPT {
    return TScalarVectorTernaryOp(a, b, c, [&f](auto x, auto y, auto z) CONSTEXPR NOEXCEPT {
        return BarycentricLerp(x, y, z, f.x, f.y, f.z);
    });
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
bool NearlyEquals(const TScalarVector<float, _Dim>& a, const TScalarVector<float, _Dim>& b, float maxRelDiff = F_Epsilon) NOEXCEPT;
//----------------------------------------------------------------------------
TScalarVector<float, 2> SinCos(float angle) NOEXCEPT;
TScalarVector<double, 2> SinCos(double angle) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsINF(const TScalarVectorExpr<T, _Dim>& v) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsNAN(const TScalarVectorExpr<T, _Dim>& v) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsNANorINF(const TScalarVectorExpr<T, _Dim>& v) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UByte0255_to_Float01(const TScalarVector<u8, _Dim>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UByte0255_to_FloatM11(const TScalarVector<u8, _Dim>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u8, _Dim> Float01_to_UByte0255(const TScalarVector<float, _Dim>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u8, _Dim> FloatM11_to_UByte0255(const TScalarVector<float, _Dim>& value) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UShort065535_to_Float01(const TScalarVector<u16, _Dim>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UByte065535_to_FloatM11(const TScalarVector<u16, _Dim>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u16, _Dim> Float01_to_UShort065535(const TScalarVector<float, _Dim>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u16, _Dim> FloatM11_to_UShort065535(const TScalarVector<float, _Dim>& value) NOEXCEPT;
//----------------------------------------------------------------------------
TScalarVector<u8, 4> Float3M11_to_UByte4N(const TScalarVector<float, 3>& value) NOEXCEPT;
//----------------------------------------------------------------------------
TScalarVector<float, 3> UByte4N_to_Float3M11(const TScalarVector<u8, 4>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> Float01_to_FloatM11(const TScalarVector<float, _Dim>& v_01) NOEXCEPT;
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> FloatM11_to_Float01(const TScalarVector<float, _Dim>& v_M11) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void SwapEndiannessInPlace(TScalarVector<T, _Dim>* value) NOEXCEPT {
    FPlatformEndian::SwapInPlace(value->Data);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/ScalarVectorHelpers-inl.h"
