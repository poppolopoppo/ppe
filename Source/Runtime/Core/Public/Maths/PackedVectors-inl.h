#pragma once

#include "Maths/PackedVectors.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline void UX10Y10Z10W2N::Pack_Float01(const TScalarVector<float, 3>& xyz, u8 w) {
    Assert(0 <= xyz.x && 1 >= xyz.x);
    Assert(0 <= xyz.y && 1 >= xyz.y);
    Assert(0 <= xyz.z && 1 >= xyz.z);
    Assert(w < 4u);

    _data =  static_cast<u32>(xyz.x * 1023.0f)
        |   (static_cast<u32>(xyz.y * 1023.0f) << 10)
        |   (static_cast<u32>(xyz.z * 1023.0f) << 20)
        |   (w << 30);

#if USE_PPE_ASSERT
    TScalarVector<float, 3> tmp;
    tmp.x = static_cast<float>(_data & 1023) / 1023.0f;
    tmp.y = static_cast<float>((_data >> 10) & 1023) / 1023.0f;
    tmp.z = static_cast<float>((_data >> 20) & 1023) / 1023.0f;

    const u32 data2 =    static_cast<u32>(tmp.x * 1023.0f)
                    |   (static_cast<u32>(tmp.y * 1023.0f) << 10)
                    |   (static_cast<u32>(tmp.z * 1023.0f) << 20)
                    |   (w << 30);

    Assert(data2 == _data);
#endif
}
//----------------------------------------------------------------------------
inline void UX10Y10Z10W2N::Unpack_Float01(TScalarVector<float, 3>& xyz) const {
    xyz.x = static_cast<float>(_data & 1023) / 1023.0f;
    xyz.y = static_cast<float>((_data >> 10) & 1023) / 1023.0f;
    xyz.z = static_cast<float>((_data >> 20) & 1023) / 1023.0f;
}
//----------------------------------------------------------------------------
inline void UX10Y10Z10W2N::Unpack_Float01(TScalarVector<float, 3>& xyz, u8& w) const {
    Unpack_Float01(xyz);
    w = static_cast<int>((_data >> 30) & 3);
}
//----------------------------------------------------------------------------
inline void UX10Y10Z10W2N::Pack_FloatM11(const TScalarVector<float, 3>& xyz, u8 w) {
    Pack_Float01(xyz * 0.5f + 0.5f, w);
}
//----------------------------------------------------------------------------
inline void UX10Y10Z10W2N::Unpack_FloatM11(TScalarVector<float, 3>& xyz) const {
    Unpack_Float01(xyz);
    xyz = xyz * 2.f - 1.f;
}
//----------------------------------------------------------------------------
inline void UX10Y10Z10W2N::Unpack_FloatM11(TScalarVector<float, 3>& xyz, u8& w) const {
    Unpack_FloatM11(xyz);
    w = static_cast<u8>((_data >> 30) & 3);
}
//----------------------------------------------------------------------------
inline void UX10Y10Z10W2N::Pack(const TScalarVector<float, 4>& v) {
    const u8 w = u8((v.w * 0.5f + 0.5f)*3);
    Pack_FloatM11(v.xyz, w);
}
//----------------------------------------------------------------------------
inline TScalarVector<float, 4> UX10Y10Z10W2N::Unpack() const {
    u8 w;
    TScalarVector<float, 3> xyz;
    Unpack_FloatM11(xyz, w);
    return TScalarVector<float, 4>(xyz, w/1.5f - 1.f);
}
//----------------------------------------------------------------------------
inline UX10Y10Z10W2N BarycentricLerp(const UX10Y10Z10W2N& v0, const UX10Y10Z10W2N& v1, const UX10Y10Z10W2N& v2, float f0, float f1, float f2) {
    return BarycentricLerp(v0.Unpack(), v1.Unpack(), v2.Unpack(), f0, f1, f2);
}
//----------------------------------------------------------------------------
inline UX10Y10Z10W2N BarycentricLerp(const UX10Y10Z10W2N& v0, const UX10Y10Z10W2N& v1, const UX10Y10Z10W2N& v2, const float3& uvw) {
    return BarycentricLerp(v0, v1, v2, uvw.x, uvw.y, uvw.z);
}
//----------------------------------------------------------------------------
inline UX10Y10Z10W2N Lerp(const UX10Y10Z10W2N& v0, const UX10Y10Z10W2N& v1, float f) {
    return float4{ Lerp(v0.Unpack(), v1.Unpack(), f) };
}
//----------------------------------------------------------------------------
inline UX10Y10Z10W2N Float01_to_UX10Y10Z10W2N(const TScalarVector<float, 3>& xyz, u8 w) {
    UX10Y10Z10W2N packed;
    packed.Pack_Float01(xyz, w);
    return packed;
}
//----------------------------------------------------------------------------
inline UX10Y10Z10W2N Float01_to_UX10Y10Z10W2N(float x, float y, float z, u8 w) {
    return Float01_to_UX10Y10Z10W2N(float3(x, y, z), w);
}
//----------------------------------------------------------------------------
inline UX10Y10Z10W2N FloatM11_to_UX10Y10Z10W2N(const TScalarVector<float, 3>& xyz, u8 w) {
    UX10Y10Z10W2N packed;
    packed.Pack_FloatM11(xyz, w);
    return packed;
}
//----------------------------------------------------------------------------
inline UX10Y10Z10W2N FloatM11_to_UX10Y10Z10W2N(float x, float y, float z, u8 w) {
    return FloatM11_to_UX10Y10Z10W2N(float3(x, y, z), w);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline void UX11Y11Z10::Pack(const float3& xyz) {
    FP32 bits;

    bits.f = xyz.x;
    Assert(bits.f >= 0.0f);
    _r_m = bits.Mantissa >> (23 - 6);
    _r_e = bits.Exponent - (127 - 15);

    bits.f = xyz.y;
    Assert(bits.f >= 0.0f);
    _g_m = bits.Mantissa >> (23 - 6);
    _g_e = bits.Exponent - (127 - 15);

    bits.f = xyz.z;
    Assert(bits.f >= 0.0f);
    _b_m = bits.Mantissa >> (23 - 5);
    _b_e = bits.Exponent - (127 - 15);
}
//----------------------------------------------------------------------------
inline float3 UX11Y11Z10::Unpack() const {
    FP32 bits;
    bits.Sign = 0;

    float3 result;

    bits.Mantissa = _r_m << (23 - 6);
    bits.Exponent = _r_e + (127 - 15);
    result.x = bits.f;

    bits.Mantissa = _g_m << (23 - 6);
    bits.Exponent = _g_e + (127 - 15);
    result.y = bits.f;

    bits.Mantissa = _b_m << (23 - 5);
    bits.Exponent = _b_e + (127 - 15);
    result.z = bits.f;

    return result;
}
//----------------------------------------------------------------------------
inline UX11Y11Z10 BarycentricLerp(const UX11Y11Z10& v0, const UX11Y11Z10& v1, const UX11Y11Z10& v2, float f0, float f1, float f2) {
    return Float_to_UX11Y11Z10(BarycentricLerp(
        v0.Unpack(),
        v1.Unpack(),
        v2.Unpack(),
        f0, f1, f2 ));
}
//----------------------------------------------------------------------------
inline UX11Y11Z10 BarycentricLerp(const UX11Y11Z10& v0, const UX11Y11Z10& v1, const UX11Y11Z10& v2, const float3& uvw) {
    return BarycentricLerp(v0, v1, v2, uvw.x, uvw.y, uvw.z);
}
//----------------------------------------------------------------------------
inline UX11Y11Z10 Lerp(const UX11Y11Z10& v0, const UX11Y11Z10& v1, float f) {
    return Float_to_UX11Y11Z10(Lerp(
        v0.Unpack(),
        v1.Unpack(),
        f ));
}
//----------------------------------------------------------------------------
inline UX11Y11Z10 Float_to_UX11Y11Z10(const float3& xyz) {
    UX11Y11Z10 result;
    result.Pack(xyz);
    return result;
}
//----------------------------------------------------------------------------
inline UX11Y11Z10 Float_to_UX11Y11Z10(float x, float y, float z) {
    return Float_to_UX11Y11Z10(float3{ x, y, z });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u16, _Dim> FP32_to_FP16(const TScalarVector<float, _Dim>& value) {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT -> TScalarVector<u16, _Dim> {
        return { FP32_to_FP16(value.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> FP16_to_FP32(const TScalarVector<u16, _Dim>& value) {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT -> TScalarVector<float, _Dim> {
        return { FP16_to_FP32(value.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<FHalfFloat, _Dim> HalfPack(const TScalarVector<float, _Dim>& value) {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT -> TScalarVector<FHalfFloat, _Dim> {
        return { FHalfFloat{ value.template get<idx>() }... };
    });
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> HalfUnpack(const TScalarVector<FHalfFloat, _Dim>& value) {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT -> TScalarVector<float, _Dim> {
        return { value.template get<idx>().Unpack()... };
    });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<TUNorm<T>, _Dim> UNormPack(const TScalarVector<float, _Dim>& value) {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT -> TScalarVector<TUNorm<T>, _Dim> {
        return { TUNorm<T>{ value.template get<idx>() }... };
    });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<TSNorm<T>, _Dim> SNormPack(const TScalarVector<float, _Dim>& value) {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT -> TScalarVector<TSNorm<T>, _Dim> {
        return { TSNorm<T>{ value.template get<idx>() }... };
    });
}
//----------------------------------------------------------------------------
template <typename _Traits, typename T, size_t _Dim>
TScalarVector<float, _Dim> NormUnpack(const TScalarVector<TBasicNorm<T, _Traits>, _Dim>& value) {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT -> TScalarVector<float, _Dim> {
        return { value.template get<idx>().Normalized()... };
    });
}
//----------------------------------------------------------------------------
template <u32 _Bits, typename T, size_t _Dim>
TScalarVector<float, _Dim> ScaleUNorm(const TScalarVector<u32, _Dim>& value) {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT -> TScalarVector<float, _Dim> {
        return { ScaleUNorm<_Bits>(value.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
template <u32 _Bits, typename T, size_t _Dim>
TScalarVector<float, _Dim> ScaleSNorm(const TScalarVector<i32, _Dim>& value) {
    return Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT -> TScalarVector<float, _Dim> {
        return { ScaleSNorm<_Bits>(value.template get<idx>())... };
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
