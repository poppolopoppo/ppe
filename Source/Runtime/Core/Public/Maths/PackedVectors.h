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
            u32 _w : 2;
            u32 _z : 10;
            u32 _y : 10;
            u32 _x : 10;
        };
    };

    FORCE_INLINE UX10Y10Z10W2N() NOEXCEPT {}
    FORCE_INLINE explicit UX10Y10Z10W2N(Meta::FForceInit) : _data(0) {}

    UX10Y10Z10W2N(u32 data) : _data(data) {}
    UX10Y10Z10W2N(const float3& value) { Pack(value); }
    UX10Y10Z10W2N(const float4& value) { Pack(value); }
    ~UX10Y10Z10W2N() = default;

    FORCE_INLINE void Pack_Float01(const float3& xyz, u8 w);
    FORCE_INLINE void Unpack_Float01(float3& xyz) const;
    FORCE_INLINE void Unpack_Float01(float3& xyz, u8& w) const;

    FORCE_INLINE void Pack_FloatM11(const float3& xyz, u8 w);
    FORCE_INLINE void Unpack_FloatM11(float3& xyz) const;
    FORCE_INLINE void Unpack_FloatM11(float3& xyz, u8& w) const;

    void Pack(const float3& v) { Pack({ v, 0 }); }
    void Pack(const float4& v);
    float4 Unpack() const;

    static UX10Y10Z10W2N DefaultValue() { return UX10Y10Z10W2N(Meta::ForceInit); }

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
struct UX11Y11Z10 {
    union {
        u32 _data;
        struct {
            u32 _z : 10;
            u32 _y : 11;
            u32 _x : 11;
        };
        struct {
            // blue
            u32 _b_m : 5;
            u32 _b_e : 5;
            // green
            u32 _g_m : 6;
            u32 _g_e : 5;
            // blue
            u32 _r_m : 6;
            u32 _r_e : 5;
        };
    };

    FORCE_INLINE UX11Y11Z10() NOEXCEPT {}
    FORCE_INLINE explicit UX11Y11Z10(Meta::FForceInit) : _data(0) {}

    UX11Y11Z10(u32 data) : _data(data) {}
    UX11Y11Z10(const float3& value) { Pack(value); }
    ~UX11Y11Z10() = default;

    void Pack(const float3& v);
    float3 Unpack() const;

    static UX11Y11Z10 DefaultValue() { return UX11Y11Z10(Meta::ForceInit); }

    operator float3 () const { return Unpack(); }

    FORCE_INLINE bool operator ==(const UX11Y11Z10& other) const { return _data == other._data; }
    FORCE_INLINE bool operator !=(const UX11Y11Z10& other) const { return not operator ==(other); }

    inline friend hash_t hash_value(const UX11Y11Z10& v) { return hash_as_pod(v._data); }
};
//----------------------------------------------------------------------------
UX11Y11Z10 BarycentricLerp(const UX11Y11Z10& v0, const UX11Y11Z10& v1, const UX11Y11Z10& v2, float f0, float f1, float f2);
UX11Y11Z10 BarycentricLerp(const UX11Y11Z10& v0, const UX11Y11Z10& v1, const UX11Y11Z10& v2, const float3& uvw);
//----------------------------------------------------------------------------
UX11Y11Z10 Lerp(const UX11Y11Z10& v0, const UX11Y11Z10& v1, float f);
//----------------------------------------------------------------------------
UX11Y11Z10 Float_to_UX11Y11Z10(const float3& xyz);
UX11Y11Z10 Float_to_UX11Y11Z10(float x, float y, float z);
//----------------------------------------------------------------------------
template <>
struct TNumericLimits< UX11Y11Z10 > {
    typedef UX11Y11Z10 value_type;
    typedef TNumericLimits<float> scalar_type;

    STATIC_CONST_INTEGRAL(bool, is_integer, scalar_type::is_integer);
    STATIC_CONST_INTEGRAL(bool, is_modulo,  scalar_type::is_modulo);
    STATIC_CONST_INTEGRAL(bool, is_signed,  scalar_type::is_signed);

    static value_type DefaultValue() { return Float_to_UX11Y11Z10(0.0f,0.0f,0.0f); }
    static value_type Epsilon() { return Float_to_UX11Y11Z10(1.0f/1024,1.0f/1024,1.0f/1024); }
    static value_type Inf() { return Float_to_UX11Y11Z10(1.0f,1.0f,1.0f); }
    static value_type MaxValue() { return Float_to_UX11Y11Z10(1.0f,1.0f,1.0f); }
    static value_type MinValue() { return Float_to_UX11Y11Z10(-1.0f,-1.0f,-1.0f); }
    static value_type Nan() { return Float_to_UX11Y11Z10(1.0f,1.0f,1.0f); }
    static value_type Zero() { return Float_to_UX11Y11Z10(0.0f,0.0f,0.0f); }
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(UX11Y11Z10)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u16, _Dim> FP32_to_FP16(const TScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> FP16_to_FP32(const TScalarVector<u16, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> HalfUnpack(const TScalarVector<FHalfFloat, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<FHalfFloat, _Dim> HalfPack(const TScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> HalfUnpack(const TScalarVector<FHalfFloat, _Dim>& value);
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
template <u32 _Bits, typename T, size_t _Dim>
TScalarVector<float, _Dim> ScaleUNorm(const TScalarVector<u32, _Dim>& value);
//----------------------------------------------------------------------------
template <u32 _Bits, typename T, size_t _Dim>
TScalarVector<float, _Dim> ScaleSNorm(const TScalarVector<i32, _Dim>& value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/PackedVectors-inl.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct is_basic_norm<TScalarVector<T, _Dim>> : is_basic_norm<T> {};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct is_snorm<TScalarVector<T, _Dim>> : is_snorm<T> {};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct is_unorm<TScalarVector<T, _Dim>> : is_unorm<T> {};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct is_packed_integral<TScalarVector<T, _Dim>> : is_packed_integral<T> {};
template <>
struct is_packed_integral<UX11Y11Z10> : std::bool_constant<true> {};
template <>
struct is_packed_integral<UX10Y10Z10W2N> : std::bool_constant<true> {};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE

#ifndef EXPORT_PPE_RUNTIME_CORE_SCALARVECTOR
namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TSNorm<u8>, 2>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TSNorm<u8>, 4>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TUNorm<u8>, 2>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TUNorm<u8>, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TSNorm<u16>, 2>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TSNorm<u16>, 4>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TUNorm<u16>, 2>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TUNorm<u16>, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TSNorm<u32>, 2>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TSNorm<u32>, 3>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TSNorm<u32>, 4>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TUNorm<u32>, 2>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TUNorm<u32>, 3>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<TUNorm<u32>, 4>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
#endif //!EXPORT_PPE_RUNTIME_CORE_SCALARVECTOR
