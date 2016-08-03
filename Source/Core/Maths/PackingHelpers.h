#pragma once

#include "Core/Core.h"

#include "Core/Maths/Packing/Packing_fwd.h"

#include "Core/Memory/HashFunctions.h"

#include <iosfwd>

namespace Core {
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
FORCE_INLINE float UShort065535_to_Float01(u16 value);
//----------------------------------------------------------------------------
FORCE_INLINE float UShort065535_to_FloatM11(u16 value);
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
float FP16_to_FP32(u16 value);
//----------------------------------------------------------------------------
u16 FP32_to_FP16(float value);
//----------------------------------------------------------------------------
struct HalfFloat {
    u16 _data;

    HalfFloat() {}
    ~HalfFloat() {}

    FORCE_INLINE operator u16 () const { return _data; }
    FORCE_INLINE explicit HalfFloat(u16 data) : _data(data) {}
    FORCE_INLINE HalfFloat& operator =(u16 data) { _data = data; return *this; }

    FORCE_INLINE operator float () const { return Unpack(); }
    FORCE_INLINE HalfFloat(float value) { Pack(value); }
    FORCE_INLINE HalfFloat& operator =(float value) { Pack(value); return *this; }

    FORCE_INLINE void Pack(float value);
    FORCE_INLINE float Unpack() const;

    FORCE_INLINE bool operator ==(const HalfFloat& other) const { return _data == other._data; }
    FORCE_INLINE bool operator !=(const HalfFloat& other) const { return !operator ==(other); }

    FORCE_INLINE bool operator <(const HalfFloat& other) const { return Unpack() < other.Unpack(); }
    FORCE_INLINE bool operator >=(const HalfFloat& other) const { return !operator <(other); }

    static HalfFloat DefaultValue() { return HalfFloat(u16(0)); }
    static HalfFloat Epsilon() { return HalfFloat(u16(0x0001)); }
    static HalfFloat MaxValue() { return HalfFloat(u16(0x7bff)); }
    static HalfFloat MinValue() { return HalfFloat(u16(0xfbff)); }
    static HalfFloat Nan() { return HalfFloat(u16(0xfe00)); }
    static HalfFloat NegativeInf() { return HalfFloat(u16(0xfc00)); }
    static HalfFloat PositiveInf() { return HalfFloat(u16(0x7c00)); }

    static HalfFloat One() { return HalfFloat(u16(0x3c00)); }
    static HalfFloat MinusOne() { return HalfFloat(u16(0x0)); }
    static HalfFloat Zero() { return HalfFloat(u16(0xbc00)); }

    static bool IsConvertible(float value);

    inline friend hash_t hash_value(const HalfFloat& h) { return hash_as_pod(h._data); }
};
//----------------------------------------------------------------------------
template <>
struct NumericLimits< HalfFloat > {
    STATIC_CONST_INTEGRAL(bool, is_integer, false);
    STATIC_CONST_INTEGRAL(bool, is_modulo,  false);
    STATIC_CONST_INTEGRAL(bool, is_signed,  true);

    static HalfFloat DefaultValue() { return HalfFloat::DefaultValue(); }
    static HalfFloat Epsilon() { return HalfFloat::Epsilon(); }
    static HalfFloat Inf() { return HalfFloat::PositiveInf(); }
    static HalfFloat MaxValue() { return HalfFloat::MaxValue(); }
    static HalfFloat MinValue() { return HalfFloat::MinValue(); }
    static HalfFloat Nan() { return HalfFloat::Nan(); }
    static HalfFloat Zero() { return HalfFloat::Zero(); }
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const HalfFloat& half) {
    return oss << half.Unpack();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct UNormTraits {};
template <>
struct UNormTraits<u8> {
    static float Normalize(u8 value) { return UByte0255_to_Float01(value); }
    static u8 Denormalize(float value) { return Float01_to_UByte0255(value); }
};
template <>
struct UNormTraits<u16> {
    static float Normalize(u16 value) { return UShort065535_to_Float01(value); }
    static u16 Denormalize(float value) { return Float01_to_UShort065535(value); }
};
template <>
struct UNormTraits<u32> {
    static float Normalize(u32 value) { return UWord_to_Float01(value); }
    static u32 Denormalize(float value) { return Float01_to_UWord(value); }
};
//----------------------------------------------------------------------------
template <typename T>
struct SNormTraits {};
template <>
struct SNormTraits<u8> {
    static float Normalize(u8 value) { return UByte0255_to_FloatM11(value); }
    static u8 Denormalize(float value) { return FloatM11_to_UByte0255(value); }
};
template <>
struct SNormTraits<u16> {
    static float Normalize(u16 value) { return UShort065535_to_FloatM11(value); }
    static u16 Denormalize(float value) { return FloatM11_to_UShort065535(value); }
};
template <>
struct SNormTraits<u32> {
    static float Normalize(u32 value) { return UWord_to_FloatM11(value); }
    static u32 Denormalize(float value) { return FloatM11_to_UWord(value); }
};
//----------------------------------------------------------------------------
template <typename T, typename _Traits = UNormTraits<T> >
struct BasicNorm {
    template <typename T2, typename _Traits2>
    friend struct BasicNorm;

    typedef _Traits traits_type;

    T _data;

    FORCE_INLINE BasicNorm() {}
    FORCE_INLINE ~BasicNorm() {}

    FORCE_INLINE operator T() const { return _data; }
    FORCE_INLINE BasicNorm(T data) : _data(data) {}
    FORCE_INLINE BasicNorm& operator =(T data) { _data = data; return *this; }

    template <typename U>
    FORCE_INLINE BasicNorm(U data) : _data(checked_cast<T>(data)) {}

    operator float () const { return Normalized(); }
    BasicNorm(float value) : _data(traits_type::Denormalize(value)) {}
    BasicNorm& operator =(float value) { _data = traits_type::Denormalize(value); return *this; }

    FORCE_INLINE BasicNorm(const BasicNorm& other) : _data(other._data) {}
    FORCE_INLINE BasicNorm& operator =(const BasicNorm& other) { _data = other._data; return *this; }

    template <typename U>
    BasicNorm(const BasicNorm<U>& other) : BasicNorm(other.Normalized()) {}
    template <typename U>
    BasicNorm& operator =(const BasicNorm<U>& other) { _data = traits_type::Denormalize(other.Normalized()); return *this; }

    float Normalized() const { return traits_type::Normalize(_data); }
    void SetNormalized(float value) { _data = traits_type::Denormalize(value); }

    static BasicNorm DefaultValue() { return BasicNorm(T(0)); }

    FORCE_INLINE bool operator ==(const BasicNorm& other) const { return _data == other._data; }
    FORCE_INLINE bool operator !=(const BasicNorm& other) const { return !operator ==(other); }

    FORCE_INLINE bool operator <(const BasicNorm& other) const { return _data < other._data; }
    FORCE_INLINE bool operator >=(const BasicNorm& other) const { return !operator <(other); }

    FORCE_INLINE bool operator >(const BasicNorm& other) const { return _data > other._data; }
    FORCE_INLINE bool operator <=(const BasicNorm& other) const { return !operator >(other); }

    inline friend hash_t hash_value(const BasicNorm& n) { return hash_as_pod(n._data); }
};
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
struct NumericLimits< BasicNorm<T, _Traits> > {
    typedef BasicNorm<T, _Traits> value_type;
    typedef NumericLimits<T> scalar_type;

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Packing/PackingHelpers-inl.h"
