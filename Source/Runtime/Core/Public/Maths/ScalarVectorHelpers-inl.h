#pragma once

#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> GridSnap(const Meta::TDontDeduce<TScalarVector<T, _Dim>>& location, T grid) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return details::MakeScalarVector(GridSnap(location.template get<idx>(), grid)...);
    });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> GridSnap(const Meta::TDontDeduce<TScalarVector<T, _Dim>>& location, const TScalarVector<T, _Dim>& grid) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return details::MakeScalarVector(GridSnap(location.template get<idx>(), grid.template get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim>
bool IsNormalized(const TScalarVector<float, _Dim>& v, float epsilon/* = F_Epsilon */) NOEXCEPT {
    return Abs(1.0f - LengthSq(v)) < epsilon;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
bool NearlyEquals(const TScalarVector<float, _Dim>& a, const Meta::TDontDeduce<TScalarVector<float, _Dim>>& b, float maxRelDiff/* = F_Epsilon */) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT{
        return (NearlyEquals(a.template get<idx>(), b.template get<idx>(), maxRelDiff) && ...);
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
bool IsINF(const TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT{
        return (IsINF(v.template get<idx>()) || ...);
    });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
bool IsNAN(const TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT{
        return (IsNAN(v.template get<idx>()) || ...);
    });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
bool IsNANorINF(const TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT{
        return (IsNANorINF(v.template get<idx>()) || ...);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<float, _Dim> UByte0255_to_Float01(const TScalarVectorExpr<u8, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return details::MakeScalarVector(UByte0255_to_Float01(value.template get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<float, _Dim> UByte0255_to_FloatM11(const TScalarVectorExpr<u8, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return details::MakeScalarVector(UByte0255_to_FloatM11(value.template get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<u8, _Dim> Float01_to_UByte0255(const TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return details::MakeScalarVector(Float01_to_UByte0255(value.template get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<u8, _Dim> FloatM11_to_UByte0255(const TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return details::MakeScalarVector(FloatM11_to_UByte0255(value.template get<idx>())...);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<float, _Dim> UShort065535_to_Float01(const TScalarVectorExpr<u16, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return details::MakeScalarVector(UShort065535_to_Float01(value.template get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<float, _Dim> UByte065535_to_FloatM11(const TScalarVectorExpr<u16, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return details::MakeScalarVector(UByte065535_to_FloatM11(value.template get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<u16, _Dim> Float01_to_UShort065535(const TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return details::MakeScalarVector(Float01_to_UShort065535(value.template get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<u16, _Dim> FloatM11_to_UShort065535(const TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return details::MakeScalarVector(FloatM11_to_UShort065535(value.template get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<float, _Dim> Float01_to_FloatM11(const TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<float, _Dim>{
        return details::MakeScalarVector(Float01_to_FloatM11(value.template get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim, typename _Expr>
TScalarVector<float, _Dim> FloatM11_to_Float01(const TScalarVectorExpr<float, _Dim, _Expr>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<float, _Dim> {
        return details::MakeScalarVector(FloatM11_to_Float01(value.template get<idx>())...);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Expr>
TScalarVector<u8, 4> Float3M11_to_UByte4N(const TScalarVectorExpr<float, 3, _Expr>& value) NOEXCEPT {
    return { FloatM11_to_UByte0255(value.xyz), 0_u8 };
}
//----------------------------------------------------------------------------
template <typename _Expr>
TScalarVector<float, 3> UByte4N_to_Float3M11(const TScalarVectorExpr<u8, 4, _Expr>& value) NOEXCEPT {
    return Normalize(UByte0255_to_FloatM11(value.xyz));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
