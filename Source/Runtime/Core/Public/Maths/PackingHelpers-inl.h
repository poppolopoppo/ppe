#pragma once

#include "Maths/PackingHelpers.h"

#include "Maths/MathHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float UByte0255_to_Float01(u8 value) {
    return Saturate(value / 255.0f);
}
//----------------------------------------------------------------------------
float UByte0255_to_FloatM11(u8 value) {
    return Clamp((int(value) - 127) / 127.0f, -1.f, 1.f);
}
//----------------------------------------------------------------------------
u8 Float01_to_UByte0255(float value) {
    Assert(0 <= value && 1 >= value);
    return static_cast<u8>(Min(255u, (u32)(value * 256.0f)));
}
//----------------------------------------------------------------------------
u8 FloatM11_to_UByte0255(float value) {
    Assert(-1 <= value && 1 >= value);
    return static_cast<u8>((value + 1) * 127.0f);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float ShortM3276832767_to_FloatM11(i16 value) {
    return Clamp(float(value) / (value <= 0 ? 32768 : 32767), -1.f, 1.f);
}
//----------------------------------------------------------------------------
float UShort065535_to_Float01(u16 value) {
    return Saturate(value / 65535.0f);
}
//----------------------------------------------------------------------------
float UShort065535_to_FloatM11(u16 value) {
    return Clamp((int(value) - 32767) / 32767.0f, 1.f, 1.f);
}
//----------------------------------------------------------------------------
i16 FloatM11_to_ShortM3276832767(float value) {
    Assert(-1 <= value && 1 >= value);
    return static_cast<i16>(value <= 0
        ? CeilToInt(value * 32768)
        : FloorToInt(value * 32767) );
}
//----------------------------------------------------------------------------
u16 Float01_to_UShort065535(float value) {
    Assert(0 <= value && 1 >= value);
    return static_cast<u16>(Min(65355u, (u32)(value * 65356.0f)));
}
//----------------------------------------------------------------------------
u16 FloatM11_to_UShort065535(float value) {
    Assert(-1 <= value && 1 >= value);
    return static_cast<u16>((value + 1) * 32767.0f);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float UWord_to_Float01(u32 value) {
    return Saturate(value / static_cast<float>(UINT32_MAX));
}
//----------------------------------------------------------------------------
float UWord_to_FloatM11(u32 value) {
    return Clamp((value - static_cast<float>(UINT32_MAX >> 1)) / static_cast<float>(UINT32_MAX >> 1), -1.f, 1.f);
}
//----------------------------------------------------------------------------
u32 Float01_to_UWord(float value) {
    Assert(0 <= value && 1 >= value);
    return static_cast<u8>(Min(4294967295u, (u32)(value * 4294967296.0f)));
}
//----------------------------------------------------------------------------
u32 FloatM11_to_UWord(float value) {
    Assert(-1 <= value && 1 >= value);
    return static_cast<u32>((value + 1) * static_cast<float>(UINT32_MAX >> 1));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4723) // potential divide by 0
//----------------------------------------------------------------------------
template <u32 _Bits>
float ScaleUNorm(u32 value) {
    STATIC_ASSERT(_Bits <= 32);
    FP32 bits{ 0 };
    bits.Exponent = (127 + _Bits);
    return Clamp(static_cast<float>(value) / (bits.f - 1.0f), 0.0f, 1.0f);
}
//----------------------------------------------------------------------------
template <u32 _Bits>
float ScaleSNorm(i32 value) {
    STATIC_ASSERT(_Bits <= 32);
    FP32 bits{ 0 };
    bits.Exponent = (126 + _Bits);
    return Clamp(static_cast<float>(value) / (bits.f - 1.0f), -1.0f, 1.0f);
}
//----------------------------------------------------------------------------
template <u32 _Bits>
u32 UnscaleUNorm(float value) {
    STATIC_ASSERT(_Bits <= 32);
    FP32 bits{ 0 };
    bits.Exponent = (127 + _Bits);
    return static_cast<u32>(value * (bits.f - 1.0f));
}
//----------------------------------------------------------------------------
template <u32 _Bits>
i32 UnscaleSNorm(float value) {
    STATIC_ASSERT(_Bits <= 32);
    FP32 bits{ 0 };
    bits.Exponent = (126 + _Bits);
    Assert(bits.f != 1.0f);
    return static_cast<i32>(value * (bits.f - 1.0f));
}
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FHalfFloat::Pack(float value) {
    Assert_NoAssume(IsConvertible(value));
    _data = FP32_to_FP16(value);
}
//----------------------------------------------------------------------------
float FHalfFloat::Unpack() const {
    const float f = FP16_to_FP32(_data);
    Assert_NoAssume(std::isfinite(f));
    return f;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
