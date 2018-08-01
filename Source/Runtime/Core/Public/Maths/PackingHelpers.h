#pragma once

#include "Core.h"

#include "Maths/Packing_fwd.h"

#include "IO/TextWriter_fwd.h"
#include "Maths/MathHelpers.h"
#include "Memory/HashFunctions.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Generalist float quantization
//----------------------------------------------------------------------------
template <typename T, size_t _Bits>
constexpr T QuantizeCeil(float value, float vmin, float vmax) {
    constexpr T GNumSlices = (1 << _Bits);
    constexpr T GMaxValue = (GNumSlices - 1);
    return Min(T(CeilToInt(LinearStep(value, vmin, vmax) * GNumSlices)), GMaxValue);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Bits>
constexpr T QuantizeFloor(float value, float vmin, float vmax) {
    constexpr T GNumSlices = (1 << _Bits);
    constexpr T GMaxValue = (GNumSlices - 1);
    return Min(T(FloorToInt(LinearStep(value, vmin, vmax) * GNumSlices)), GMaxValue);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Bits>
constexpr T QuantizeRound(float value, float vmin, float vmax) {
    constexpr T GNumSlices = (1 << _Bits);
    constexpr T GMaxValue = (GNumSlices - 1);
    return Min(T(RoundToInt(LinearStep(value, vmin, vmax) * GNumSlices)), GMaxValue);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Bits>
constexpr float Unquantize(T quantized, float vmin, float vmax) {
    constexpr T GMaxValue = ((1 << _Bits) - 1);
    constexpr float GMaxValueF = float(GMaxValue);
    Assert(vmin < vmax);
    return Lerp(vmin, vmax, Min(1.f, quantized / GMaxValueF));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE float UByte0255_to_Float01(u8 value);
//----------------------------------------------------------------------------
FORCE_INLINE float UByte0255_to_FloatM11(u8 value);
//----------------------------------------------------------------------------
FORCE_INLINE u8 Float01_to_UByte0255(float value);
//----------------------------------------------------------------------------
FORCE_INLINE u8 FloatM11_to_UByte0255(float value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE float ShortM3276832767_to_FloatM11(i16 value);
//----------------------------------------------------------------------------
FORCE_INLINE float UShort065535_to_Float01(u16 value);
//----------------------------------------------------------------------------
FORCE_INLINE float UShort065535_to_FloatM11(u16 value);
//----------------------------------------------------------------------------
FORCE_INLINE i16 FloatM11_to_ShortM3276832767(float value);
//----------------------------------------------------------------------------
FORCE_INLINE u16 Float01_to_UShort065535(float value);
//----------------------------------------------------------------------------
FORCE_INLINE u16 FloatM11_to_UShort065535(float value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE float UWord_to_Float01(u32 value);
//----------------------------------------------------------------------------
FORCE_INLINE float UWord_to_FloatM11(u32 value);
//----------------------------------------------------------------------------
FORCE_INLINE u32 Float01_to_UWord(float value);
//----------------------------------------------------------------------------
FORCE_INLINE u32 FloatM11_to_UWord(float value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API float FP16_to_FP32(u16 value);
//----------------------------------------------------------------------------
PPE_API u16 FP32_to_FP16(float value);
//----------------------------------------------------------------------------
struct FHalfFloat {
    u16 _data;

    FHalfFloat() {}
    explicit FHalfFloat(Meta::FForceInit) : _data(0) {}
    ~FHalfFloat() {}

    FORCE_INLINE operator u16 () const { return _data; }
    FORCE_INLINE explicit FHalfFloat(u16 data) : _data(data) {}
    FORCE_INLINE FHalfFloat& operator =(u16 data) { _data = data; return *this; }

    FORCE_INLINE operator float () const { return Unpack(); }
    FORCE_INLINE FHalfFloat(float value) { Pack(value); }
    FORCE_INLINE FHalfFloat& operator =(float value) { Pack(value); return *this; }

    FORCE_INLINE void Pack(float value);
    FORCE_INLINE float Unpack() const;

    FORCE_INLINE bool operator ==(const FHalfFloat& other) const { return _data == other._data; }
    FORCE_INLINE bool operator !=(const FHalfFloat& other) const { return !operator ==(other); }

    FORCE_INLINE bool operator <(const FHalfFloat& other) const { return Unpack() < other.Unpack(); }
    FORCE_INLINE bool operator >=(const FHalfFloat& other) const { return !operator <(other); }

    static FHalfFloat DefaultValue() { return FHalfFloat(u16(0)); }
    static FHalfFloat Epsilon() { return FHalfFloat(u16(0x0001)); }
    static FHalfFloat MaxValue() { return FHalfFloat(u16(0x7bff)); }
    static FHalfFloat MinValue() { return FHalfFloat(u16(0xfbff)); }
    static FHalfFloat Nan() { return FHalfFloat(u16(0xfe00)); }
    static FHalfFloat NegativeInf() { return FHalfFloat(u16(0xfc00)); }
    static FHalfFloat PositiveInf() { return FHalfFloat(u16(0x7c00)); }

    static FHalfFloat One() { return FHalfFloat(u16(0x3c00)); }
    static FHalfFloat MinusOne() { return FHalfFloat(u16(0x0)); }
    static FHalfFloat Zero() { return FHalfFloat(u16(0xbc00)); }

    PPE_API static bool IsConvertible(float value);

    inline friend hash_t hash_value(const FHalfFloat& h) { return hash_as_pod(h._data); }
};
//----------------------------------------------------------------------------
PPE_API FHalfFloat Lerp(const FHalfFloat v0, const FHalfFloat v1, float f);
PPE_API FHalfFloat BarycentricLerp(const FHalfFloat v0, const FHalfFloat v1, const FHalfFloat v2, float f0, float f1, float f2);
//----------------------------------------------------------------------------
template <>
struct TNumericLimits< FHalfFloat > {
    STATIC_CONST_INTEGRAL(bool, is_integer, false);
    STATIC_CONST_INTEGRAL(bool, is_modulo,  false);
    STATIC_CONST_INTEGRAL(bool, is_signed,  true);

    static FHalfFloat DefaultValue() { return FHalfFloat::DefaultValue(); }
    static FHalfFloat Epsilon() { return FHalfFloat::Epsilon(); }
    static FHalfFloat Inf() { return FHalfFloat::PositiveInf(); }
    static FHalfFloat MaxValue() { return FHalfFloat::MaxValue(); }
    static FHalfFloat MinValue() { return FHalfFloat::MinValue(); }
    static FHalfFloat Nan() { return FHalfFloat::Nan(); }
    static FHalfFloat Zero() { return FHalfFloat::Zero(); }
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FHalfFloat)
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const FHalfFloat& half) {
    return oss << half.Unpack();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TUNormTraits {};
template <>
struct TUNormTraits<u8> {
    static float Normalize(u8 value) { return UByte0255_to_Float01(value); }
    static u8 Denormalize(float value) { return Float01_to_UByte0255(value); }
};
template <>
struct TUNormTraits<u16> {
    static float Normalize(u16 value) { return UShort065535_to_Float01(value); }
    static u16 Denormalize(float value) { return Float01_to_UShort065535(value); }
};
template <>
struct TUNormTraits<u32> {
    static float Normalize(u32 value) { return UWord_to_Float01(value); }
    static u32 Denormalize(float value) { return Float01_to_UWord(value); }
};
//----------------------------------------------------------------------------
template <typename T>
struct TSNormTraits {};
template <>
struct TSNormTraits<u8> {
    static float Normalize(u8 value) { return UByte0255_to_FloatM11(value); }
    static u8 Denormalize(float value) { return FloatM11_to_UByte0255(value); }
};
template <>
struct TSNormTraits<u16> {
    static float Normalize(u16 value) { return UShort065535_to_FloatM11(value); }
    static u16 Denormalize(float value) { return FloatM11_to_UShort065535(value); }
};
template <>
struct TSNormTraits<u32> {
    static float Normalize(u32 value) { return UWord_to_FloatM11(value); }
    static u32 Denormalize(float value) { return FloatM11_to_UWord(value); }
};
//----------------------------------------------------------------------------
template <typename T, typename _Traits = TUNormTraits<T> >
struct TBasicNorm {
    template <typename T2, typename _Traits2>
    friend struct TBasicNorm;

    typedef _Traits traits_type;

    T _data;

    FORCE_INLINE TBasicNorm() {}
    FORCE_INLINE explicit TBasicNorm(Meta::FForceInit) : _data(T(0)) {}
    FORCE_INLINE ~TBasicNorm() {}

    FORCE_INLINE operator T() const { return _data; }
    FORCE_INLINE TBasicNorm(T data) : _data(data) {}
    FORCE_INLINE TBasicNorm& operator =(T data) { _data = data; return *this; }

    template <typename U>
    FORCE_INLINE TBasicNorm(U data) : _data(checked_cast<T>(data)) {}

    operator float () const { return Normalized(); }
    TBasicNorm(float value) : _data(traits_type::Denormalize(value)) {}
    TBasicNorm& operator =(float value) { _data = traits_type::Denormalize(value); return *this; }

    FORCE_INLINE TBasicNorm(const TBasicNorm& other) : _data(other._data) {}
    FORCE_INLINE TBasicNorm& operator =(const TBasicNorm& other) { _data = other._data; return *this; }

    template <typename U>
    TBasicNorm(const TBasicNorm<U>& other) : TBasicNorm(other.Normalized()) {}
    template <typename U>
    TBasicNorm& operator =(const TBasicNorm<U>& other) { _data = traits_type::Denormalize(other.Normalized()); return *this; }

    float Normalized() const { return traits_type::Normalize(_data); }
    void SetNormalized(float value) { _data = traits_type::Denormalize(value); }

    static TBasicNorm DefaultValue() { return TBasicNorm(T(0)); }

    FORCE_INLINE bool operator ==(const TBasicNorm& other) const { return _data == other._data; }
    FORCE_INLINE bool operator !=(const TBasicNorm& other) const { return !operator ==(other); }

    FORCE_INLINE bool operator <(const TBasicNorm& other) const { return _data < other._data; }
    FORCE_INLINE bool operator >=(const TBasicNorm& other) const { return !operator <(other); }

    FORCE_INLINE bool operator >(const TBasicNorm& other) const { return _data > other._data; }
    FORCE_INLINE bool operator <=(const TBasicNorm& other) const { return !operator >(other); }

    inline friend hash_t hash_value(const TBasicNorm& n) { return hash_as_pod(n._data); }
};
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
TBasicNorm<T, _Traits> BarycentricLerp(const TBasicNorm<T, _Traits>& v0, const TBasicNorm<T, _Traits>& v1, const TBasicNorm<T, _Traits>& v2, float f0, float f1, float f2) {
    return BarycentricLerp(v0.Normalized(), v1.Normalized(), v2.Normalized(), f0, f1, f2);
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
TBasicNorm<T, _Traits> Lerp(const TBasicNorm<T, _Traits>& v0, const TBasicNorm<T, _Traits>& v1, float f) {
    return Lerp(v0.Normalized(), v1.Normalized(), f);
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
struct TNumericLimits< TBasicNorm<T, _Traits> > {
    typedef TBasicNorm<T, _Traits> value_type;
    typedef TNumericLimits<T> scalar_type;

    STATIC_CONST_INTEGRAL(bool, is_integer, scalar_type::is_integer);
    STATIC_CONST_INTEGRAL(bool, is_modulo,  scalar_type::is_modulo);
    STATIC_CONST_INTEGRAL(bool, is_signed,  scalar_type::is_signed);

    static const value_type DefaultValue() { return value_type{ scalar_type::DefaultValue() }; }
    static const value_type Epsilon() { return value_type{ scalar_type::Epsilon() }; }
    static const value_type Inf() { return value_type{ scalar_type::Inf() }; }
    static const value_type MaxValue() { return value_type{ scalar_type::MaxValue() }; }
    static const value_type MinValue() { return value_type{ scalar_type::MinValue() }; }
    static const value_type Nan() { return value_type{ scalar_type::Nan() }; }
    static const value_type Zero() { return value_type{ scalar_type::Zero() }; }
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(TBasicNorm<T COMMA _Traits>, typename T, typename _Traits)
//----------------------------------------------------------------------------
template <typename _Char, typename T, typename _Traits>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TBasicNorm<T, _Traits>& packed) {
    return oss << packed.Normalized();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/PackingHelpers-inl.h"
