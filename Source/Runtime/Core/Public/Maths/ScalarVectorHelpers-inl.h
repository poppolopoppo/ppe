#pragma once

#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> GridSnap(const Meta::TDontDeduce<TScalarVector<T, _Dim>>& location, T grid) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<T, _Dim> {
        return { GridSnap(location.template get<idx>(), grid)... };
    });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> GridSnap(const Meta::TDontDeduce<TScalarVector<T, _Dim>>& location, const TScalarVector<T, _Dim>& grid) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT->TScalarVector<T, _Dim>{
        return { GridSnap(location.template get<idx>(), grid.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim>
bool IsNormalized(const TScalarVector<float, _Dim>& v, float epsilon/* = F_Epsilon */) NOEXCEPT {
    return Abs(1.0f - LengthSq(v)) < epsilon;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
bool NearlyEquals(const TScalarVector<float, _Dim>& a, const TScalarVector<float, _Dim>& b, float maxRelDiff/* = F_Epsilon */) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT{
        return (NearlyEquals(a.template get<idx>(), b.template get<idx>(), maxRelDiff) && ...);
    });
}
//----------------------------------------------------------------------------
inline TScalarVector<float, 2> SinCos(float angle) NOEXCEPT {
    return TScalarVector<float, 2>(
        FPlatformMaths::Sin(angle),
        FPlatformMaths::Cos(angle) );
}
//----------------------------------------------------------------------------
inline TScalarVector<double, 2> SinCos(double angle) NOEXCEPT {
    return TScalarVector<double, 2>(
        std::sin(angle),
        std::cos(angle) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsINF(const TScalarVectorExpr<T, _Dim>& v) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT{
        return (IsINF(v.template get<idx>()) || ...);
    });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsNAN(const TScalarVectorExpr<T, _Dim>& v) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT{
        return (IsNAN(v.template get<idx>()) || ...);
    });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsNANorINF(const TScalarVectorExpr<T, _Dim>& v) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT{
        return (IsNANorINF(v.template get<idx>()) || ...);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UByte0255_to_Float01(const TScalarVector<u8, _Dim>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT ->TScalarVector<float, _Dim> {
        return { UByte0255_to_Float01(value.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UByte0255_to_FloatM11(const TScalarVector<u8, _Dim>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT ->TScalarVector<float, _Dim> {
        return { UByte0255_to_FloatM11(value.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u8, _Dim> Float01_to_UByte0255(const TScalarVector<float, _Dim>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<u8, _Dim> {
        return { Float01_to_UByte0255(value.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u8, _Dim> FloatM11_to_UByte0255(const TScalarVector<float, _Dim>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<u8, _Dim> {
        return { FloatM11_to_UByte0255(value.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UShort065535_to_Float01(const TScalarVector<u16, _Dim>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<float, _Dim> {
        return { UShort065535_to_Float01(value.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UByte065535_to_FloatM11(const TScalarVector<u16, _Dim>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<float, _Dim> {
        return { UByte065535_to_FloatM11(value.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u16, _Dim> Float01_to_UShort065535(const TScalarVector<float, _Dim>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<u16, _Dim> {
        return { Float01_to_UShort065535(value.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u16, _Dim> FloatM11_to_UShort065535(const TScalarVector<float, _Dim>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<u16, _Dim> {
        return { FloatM11_to_UShort065535(value.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> Float01_to_FloatM11(const TScalarVector<float, _Dim>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<float, _Dim>{
        return { Float01_to_FloatM11(value.template get<idx>())... };
        });
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> FloatM11_to_Float01(const TScalarVector<float, _Dim>& value) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<float, _Dim> {
        return { FloatM11_to_Float01(value.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline TScalarVector<u8, 4> Float3M11_to_UByte4N(const TScalarVector<float, 3>& value) NOEXCEPT {
    return {
        FloatM11_to_UByte0255(value.x),
        FloatM11_to_UByte0255(value.y),
        FloatM11_to_UByte0255(value.z),
        0 };
}
//----------------------------------------------------------------------------
inline TScalarVector<float, 3> UByte4N_to_Float3M11(const TScalarVector<u8, 4>& value) NOEXCEPT {
    const TScalarVector<float, 3> result(
        UByte0255_to_FloatM11(value.x),
        UByte0255_to_FloatM11(value.y),
        UByte0255_to_FloatM11(value.z) );
    return Normalize(result);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
