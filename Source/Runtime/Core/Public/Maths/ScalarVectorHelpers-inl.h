#pragma once

#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <u32 _Dim>
bool IsNormalized(const TScalarVector<float, _Dim>& v, float epsilon/* = Epsilon */) NOEXCEPT {
    return Abs(1.0f - LengthSq(v)) < epsilon;
}
//----------------------------------------------------------------------------
template <u32 _Dim, typename _A, typename _B>
bool NearlyEquals(const details::TScalarVectorExpr<float, _Dim, _A>& a, const details::TScalarVectorExpr<float, _Dim, _B>& b, float maxRelDiff/* = Epsilon */) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT{
        return (NearlyEquals(a.template Get<idx>(), b.template Get<idx>(), maxRelDiff) && ...);
    });
}
//----------------------------------------------------------------------------
inline TScalarVector<float, 2> SinCos(float angle) NOEXCEPT {
    return {
        FPlatformMaths::Sin(angle),
        FPlatformMaths::Cos(angle) };
}
//----------------------------------------------------------------------------
inline TScalarVector<double, 2> SinCos(double angle) NOEXCEPT {
    return {
        std::sin(angle),
        std::cos(angle) };
}
//----------------------------------------------------------------------------
template <typename _Expr>
TScalarVector<float, 2> OctahedralNormalEncode(const details::TScalarVectorExpr<float, 3, _Expr>& n/* [-1,1], should be normalized */) NOEXCEPT {
    // https://www.shadertoy.com/view/NsfBWf
    Assert_NoAssume(IsNormalized(n));
    float2 p{ n.template Get<0>(), n.template Get<1>() };
    p /= Abs(n).HSum();
    return ((n.template Get<2>() <= 0.f) ? ( (1.f - Abs(p.yx)) * SignNotZero(p) ) : p);
}
//----------------------------------------------------------------------------
template <typename _Expr>
TScalarVector<float, 3> OctahedralNormalDecode(const details::TScalarVectorExpr<float, 2, _Expr>& v/* [-1,1] */) NOEXCEPT {
    // https://www.shadertoy.com/view/NsfBWf
    float3 n{ v, 1.f - Abs(v.template Get<0>()) - Abs(v.template Get<1>()) };
    if (n.z < 0)
        n.xy = float2((1.f - Abs(n.yx)) * SignNotZero(n.xy));
    return Normalize(n);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Expr>
bool IsINF(const details::TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT{
        return (IsINF(v.template Get<idx>()) || ...);
    });
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Expr>
bool IsNAN(const details::TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT{
        return (IsNAN(v.template Get<idx>()) || ...);
    });
}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Expr>
bool IsNANorINF(const details::TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT{
        return (IsNANorINF(v.template Get<idx>()) || ...);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<float, _Dim> UByte0255_to_Float01(const details::TScalarVectorExpr<u8, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return TScalarVector<float, _Dim>(UByte0255_to_Float01(value.template Get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<float, _Dim> UByte0255_to_FloatM11(const details::TScalarVectorExpr<u8, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return TScalarVector<float, _Dim>(UByte0255_to_FloatM11(value.template Get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<u8, _Dim> Float01_to_UByte0255(const details::TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return TScalarVector<u8, _Dim>(Float01_to_UByte0255(value.template Get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<u8, _Dim> FloatM11_to_UByte0255(const details::TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return TScalarVector<u8, _Dim>(FloatM11_to_UByte0255(value.template Get<idx>())...);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<float, _Dim> UShort065535_to_Float01(const details::TScalarVectorExpr<u16, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return TScalarVector<float, _Dim>(UShort065535_to_Float01(value.template Get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<float, _Dim> UByte065535_to_FloatM11(const details::TScalarVectorExpr<u16, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return TScalarVector<float, _Dim>(UByte065535_to_FloatM11(value.template Get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<u16, _Dim> Float01_to_UShort065535(const details::TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return TScalarVector<u16, _Dim>(Float01_to_UShort065535(value.template Get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<u16, _Dim> FloatM11_to_UShort065535(const details::TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return TScalarVector<u16, _Dim>(FloatM11_to_UShort065535(value.template Get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<float, _Dim> Float01_to_FloatM11(const details::TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<float, _Dim>{
        return TScalarVector<float, _Dim>(Float01_to_FloatM11(value.template Get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <u32 _Dim, typename _Expr>
TScalarVector<float, _Dim> FloatM11_to_Float01(const details::TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<float, _Dim> {
        return TScalarVector<float, _Dim>(FloatM11_to_Float01(value.template Get<idx>())...);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Expr>
TScalarVector<u8, 4> Float3M11_to_UByte4N(const details::TScalarVectorExpr<float, 3, _Expr>& value) NOEXCEPT {
    return { FloatM11_to_UByte0255(value.xyz), 0_u8 };
}
//----------------------------------------------------------------------------
template <typename _Expr>
TScalarVector<float, 3> UByte4N_to_Float3M11(const details::TScalarVectorExpr<u8, 4, _Expr>& value) NOEXCEPT {
    return Normalize(UByte0255_to_FloatM11(value.xyz));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
