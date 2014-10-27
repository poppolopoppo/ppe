#pragma once

#include "Core/Core.h"

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
    FORCE_INLINE HalfFloat(u16 data) : _data(data) {}
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

    static const HalfFloat Epsilon;
    static const HalfFloat MaxValue;
    static const HalfFloat MinValue;
    static const HalfFloat Nan;
    static const HalfFloat NegativeInf;
    static const HalfFloat PositiveInf;
    static const HalfFloat Default;

    static const HalfFloat One;
    static const HalfFloat MinusOne;
    static const HalfFloat Zero;

    static bool IsConvertible(float value);
};
//----------------------------------------------------------------------------
typedef HalfFloat half;
//----------------------------------------------------------------------------
template <>
struct NumericLimits<HalfFloat> {
    static const HalfFloat Epsilon;
    static const HalfFloat Inf;
    static const HalfFloat MaxValue;
    static const HalfFloat MinValue;
    static const HalfFloat Nan;
    static const HalfFloat Default;
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

    FORCE_INLINE bool operator ==(const BasicNorm& other) const { return _data == other._data; }
    FORCE_INLINE bool operator !=(const BasicNorm& other) const { return !operator ==(other); }

    FORCE_INLINE bool operator <(const BasicNorm& other) const { return _data < other._data; }
    FORCE_INLINE bool operator >=(const BasicNorm& other) const { return !operator <(other); }

    FORCE_INLINE bool operator >(const BasicNorm& other) const { return _data > other._data; }
    FORCE_INLINE bool operator <=(const BasicNorm& other) const { return !operator >(other); }
};
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
struct NumericLimits< BasicNorm<T, _Traits> > {
    static const BasicNorm<T, _Traits> Epsilon;
    static const BasicNorm<T, _Traits> Inf;
    static const BasicNorm<T, _Traits> MaxValue;
    static const BasicNorm<T, _Traits> MinValue;
    static const BasicNorm<T, _Traits> Nan;
    static const BasicNorm<T, _Traits> Default;
};
template <typename T, typename _Traits>
const BasicNorm<T, _Traits> NumericLimits< BasicNorm<T, _Traits> >::Epsilon(NumericLimits<T>::Epsilon);
template <typename T, typename _Traits>
const BasicNorm<T, _Traits> NumericLimits< BasicNorm<T, _Traits> >::Inf(NumericLimits<T>::Inf);
template <typename T, typename _Traits>
const BasicNorm<T, _Traits> NumericLimits< BasicNorm<T, _Traits> >::MaxValue(NumericLimits<T>::MaxValue);
template <typename T, typename _Traits>
const BasicNorm<T, _Traits> NumericLimits< BasicNorm<T, _Traits> >::MinValue(NumericLimits<T>::MinValue);
template <typename T, typename _Traits>
const BasicNorm<T, _Traits> NumericLimits< BasicNorm<T, _Traits> >::Nan(NumericLimits<T>::Nan);
template <typename T, typename _Traits>
const BasicNorm<T, _Traits> NumericLimits< BasicNorm<T, _Traits> >::Default(NumericLimits<T>::Default);
//----------------------------------------------------------------------------
template <typename T>
using UNorm = BasicNorm<T, UNormTraits<T> >;
typedef UNorm<u8> ubyten;
typedef UNorm<u16> ushortn;
typedef UNorm<u32> uwordn;
//----------------------------------------------------------------------------
template <typename T>
using SNorm = BasicNorm<T, SNormTraits<T> >;
typedef SNorm<u8> byten;
typedef SNorm<u16> shortn;
typedef SNorm<u32> wordn;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Packing/PackingHelpers-inl.h"
