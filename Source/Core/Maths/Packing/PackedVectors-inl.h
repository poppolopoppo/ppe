#pragma once

#include "Core/Maths/Packing/PackedVectors.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline void UX10Y10Z10W2N::Pack_Float01(const ScalarVector<float, 3>& xyz, u8 w) {
    Assert(0 <= xyz.x() && 1 >= xyz.x());
    Assert(0 <= xyz.y() && 1 >= xyz.y());
    Assert(0 <= xyz.z() && 1 >= xyz.z());
    Assert(w < 4u);

    _data =  static_cast<u32>(xyz.x() * 1023.0f)
        |   (static_cast<u32>(xyz.y() * 1023.0f) << 10)
        |   (static_cast<u32>(xyz.z() * 1023.0f) << 20)
        |   (w << 30);
}
//----------------------------------------------------------------------------
inline void UX10Y10Z10W2N::Unpack_Float01(ScalarVector<float, 3>& xyz) const {
    xyz.x() = static_cast<float>(_data & 1023) / 1023.0f;
    xyz.y() = static_cast<float>((_data >> 10) & 1023) / 1023.0f;
    xyz.z() = static_cast<float>((_data >> 20) & 1023) / 1023.0f;
}
//----------------------------------------------------------------------------
inline void UX10Y10Z10W2N::Unpack_Float01(ScalarVector<float, 3>& xyz, u8& w) const {
    Unpack_Float01(xyz);
    w = static_cast<int>((_data >> 30) & 3);
}
//----------------------------------------------------------------------------
inline void UX10Y10Z10W2N::Pack_FloatM11(const ScalarVector<float, 3>& xyz, u8 w) {
    Assert(-1 <= xyz.x() && 1 >= xyz.x());
    Assert(-1 <= xyz.y() && 1 >= xyz.y());
    Assert(-1 <= xyz.z() && 1 >= xyz.z());
    Assert(w < 4u);

    _data =  static_cast<u32>((xyz.x() + 1) * 511.0f)
        |   (static_cast<u32>((xyz.y() + 1) * 511.0f) << 10)
        |   (static_cast<u32>((xyz.z() + 1) * 511.0f) << 20)
        |   (w << 30);
}
//----------------------------------------------------------------------------
inline void UX10Y10Z10W2N::Unpack_FloatM11(ScalarVector<float, 3>& xyz) const {
    xyz.x() = (static_cast<float>(_data & 1023) - 511.0f) / 511.0f;
    xyz.y() = (static_cast<float>((_data >> 10) & 1023) - 511.0f) / 511.0f;
    xyz.z() = (static_cast<float>((_data >> 20) & 1023) - 511.0f) / 511.0f;
}
//----------------------------------------------------------------------------
inline void UX10Y10Z10W2N::Unpack_FloatM11(ScalarVector<float, 3>& xyz, u8& w) const {
    Unpack_FloatM11(xyz);
    w = static_cast<u8>((_data >> 30) & 3);
}
//----------------------------------------------------------------------------
inline UX10Y10Z10W2N Float01_to_UX10Y10Z10W2N(const ScalarVector<float, 3>& xyz, u8 w) {
    UX10Y10Z10W2N packed;
    packed.Pack_Float01(xyz, w);
    return packed;
}
//----------------------------------------------------------------------------
inline UX10Y10Z10W2N Float01_to_UX10Y10Z10W2N(float x, float y, float z, u8 w) {
    return Float01_to_UX10Y10Z10W2N(float3(x, y, z), w);
}
//----------------------------------------------------------------------------
inline UX10Y10Z10W2N FloatM11_to_UX10Y10Z10W2N(const ScalarVector<float, 3>& xyz, u8 w) {
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
ScalarVector<HalfFloat, _Dim> HalfPack(const ScalarVector<float, _Dim>& value) {
    ScalarVector<HalfFloat, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i].Pack(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> HalfUnpack(const ScalarVector<HalfFloat, _Dim>& value) {
    ScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = value._data[i].Unpack();
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<UNorm<T>, _Dim> UNormPack(const ScalarVector<float, _Dim>& value) {
    ScalarVector<UNorm<T>, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i].SetNormalized(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<SNorm<T>, _Dim> SNormPack(const ScalarVector<float, _Dim>& value) {
    ScalarVector<SNorm<T>, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i].SetNormalized(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename T, size_t _Dim>
ScalarVector<float, _Dim> NormUnpack(const ScalarVector<BasicNorm<T, _Traits>, _Dim>& value) {
    ScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = value._data[i].Normalized();
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
