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
//////////////////////////////////////////////////////////////////////////////
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
template <size_t _Dim>
TScalarVector<FHalfFloat, _Dim> HalfPack(const TScalarVector<float, _Dim>& value) {
    TScalarVector<FHalfFloat, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i].Pack(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> HalfUnpack(const TScalarVector<FHalfFloat, _Dim>& value) {
    TScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = value._data[i].Unpack();
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<TUNorm<T>, _Dim> UNormPack(const TScalarVector<float, _Dim>& value) {
    TScalarVector<TUNorm<T>, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i].SetNormalized(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<TSNorm<T>, _Dim> SNormPack(const TScalarVector<float, _Dim>& value) {
    TScalarVector<TSNorm<T>, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i].SetNormalized(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename T, size_t _Dim>
TScalarVector<float, _Dim> NormUnpack(const TScalarVector<TBasicNorm<T, _Traits>, _Dim>& value) {
    TScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = value._data[i].Normalized();
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
