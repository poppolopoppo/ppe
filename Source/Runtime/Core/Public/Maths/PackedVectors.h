#pragma once

#include "Core.h"

#include "Maths/Packing_fwd.h"
#include "Maths/PackingHelpers.h"

#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct UX10Y10Z10W2N {
    union {
        u32 _data;
        struct {
            u32 _x : 10;
            u32 _y : 10;
            u32 _z : 10;
            u32 _w : 2;
        };
    };

    FORCE_INLINE UX10Y10Z10W2N() {}
    FORCE_INLINE explicit UX10Y10Z10W2N(Meta::FForceInit) : _data(0) {}

    UX10Y10Z10W2N(u32 data) : _data(data) {}
    UX10Y10Z10W2N(const float3& value) { Pack(value); }
    UX10Y10Z10W2N(const float4& value) { Pack(value); }
    ~UX10Y10Z10W2N() {}

    FORCE_INLINE void Pack_Float01(const float3& xyz, u8 w);
    FORCE_INLINE void Unpack_Float01(float3& xyz) const;
    FORCE_INLINE void Unpack_Float01(float3& xyz, u8& w) const;

    FORCE_INLINE void Pack_FloatM11(const float3& xyz, u8 w);
    FORCE_INLINE void Unpack_FloatM11(float3& xyz) const;
    FORCE_INLINE void Unpack_FloatM11(float3& xyz, u8& w) const;

    void Pack(const float3& v) { Pack({ v, 0 }); }
    void Pack(const float4& v);
    float4 Unpack() const;

    static UX10Y10Z10W2N DefaultValue() { return UX10Y10Z10W2N(u32(0)); }

    operator float3 () const { return Unpack().xyz; }
    operator float4 () const { return Unpack(); }

    FORCE_INLINE bool operator ==(const UX10Y10Z10W2N& other) const { return _data == other._data; }
    FORCE_INLINE bool operator !=(const UX10Y10Z10W2N& other) const { return !operator ==(other); }

    inline friend hash_t hash_value(const UX10Y10Z10W2N& v) { return hash_as_pod(v._data); }
};
//----------------------------------------------------------------------------
UX10Y10Z10W2N BarycentricLerp(const UX10Y10Z10W2N& v0, const UX10Y10Z10W2N& v1, const UX10Y10Z10W2N& v2, float f0, float f1, float f2);
UX10Y10Z10W2N BarycentricLerp(const UX10Y10Z10W2N& v0, const UX10Y10Z10W2N& v1, const UX10Y10Z10W2N& v2, const float3& uvw);
//----------------------------------------------------------------------------
UX10Y10Z10W2N Lerp(const UX10Y10Z10W2N& v0, const UX10Y10Z10W2N& v1, float f);
//----------------------------------------------------------------------------
UX10Y10Z10W2N Float01_to_UX10Y10Z10W2N(const float3& xyz, u8 w);
UX10Y10Z10W2N Float01_to_UX10Y10Z10W2N(float x, float y, float z, u8 w);
//----------------------------------------------------------------------------
UX10Y10Z10W2N FloatM11_to_UX10Y10Z10W2N(const float3& xyz, u8 w);
UX10Y10Z10W2N FloatM11_to_UX10Y10Z10W2N(float x, float y, float z, u8 w);
//----------------------------------------------------------------------------
UX10Y10Z10W2N Quaternion_to_UX10Y10Z10W2N(const class FQuaternion& quaternion);
FQuaternion   UX10Y10Z10W2N_to_Quaternion(const struct UX10Y10Z10W2N& packed);
//----------------------------------------------------------------------------
template <>
struct TNumericLimits< UX10Y10Z10W2N > {
    typedef UX10Y10Z10W2N value_type;
    typedef TNumericLimits<float> scalar_type;

    STATIC_CONST_INTEGRAL(bool, is_integer, scalar_type::is_integer);
    STATIC_CONST_INTEGRAL(bool, is_modulo,  scalar_type::is_modulo);
    STATIC_CONST_INTEGRAL(bool, is_signed,  scalar_type::is_signed);

    static value_type DefaultValue() { return FloatM11_to_UX10Y10Z10W2N(0.0f,0.0f,0.0f,0); }
    static value_type Epsilon() { return FloatM11_to_UX10Y10Z10W2N(1.0f/1024,1.0f/1024,1.0f/1024,0); }
    static value_type Inf() { return FloatM11_to_UX10Y10Z10W2N(1.0f,1.0f,1.0f,0); }
    static value_type MaxValue() { return FloatM11_to_UX10Y10Z10W2N(1.0f,1.0f,1.0f,2); }
    static value_type MinValue() { return FloatM11_to_UX10Y10Z10W2N(-1.0f,-1.0f,-1.0f,0); }
    static value_type Nan() { return FloatM11_to_UX10Y10Z10W2N(1.0f,1.0f,1.0f,0); }
    static value_type Zero() { return FloatM11_to_UX10Y10Z10W2N(0.0f,0.0f,0.0f,0); }
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(UX10Y10Z10W2N)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<FHalfFloat, _Dim> HalfPack(const TScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> HalfUnpack(const TScalarVector<FHalfFloat, _Dim>& value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<TUNorm<T>, _Dim> UNormPack(const TScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<TSNorm<T>, _Dim> SNormPack(const TScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
template <typename _Traits, typename T, size_t _Dim>
TScalarVector<float, _Dim> NormUnpack(const TScalarVector<TBasicNorm<T, _Traits>, _Dim>& value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/PackedVectors-inl.h"
