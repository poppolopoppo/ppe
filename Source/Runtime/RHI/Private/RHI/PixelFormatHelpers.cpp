#include "stdafx.h"

#include "RHI/PixelFormatHelpers.h"


#include "Container/Array.h"
#include "Maths/PackingHelpers.h"
#include "Maths/PackedVectors.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <u32 _Bits, u32 _Offset>
u32 DecodeUIntScalar_(const TStaticArray<u32, 4>& data) NOEXCEPT {
    STATIC_ASSERT(_Bits <= 32);
    STATIC_ASSERT(_Bits + (_Offset & 31) <= 32);

    IF_CONSTEXPR( _Bits == 0 ) {
        (void)data;
        return 0;
    }
    else {
        CONSTEXPR const u32 mask = (~0u >> (32 - _Bits));
        CONSTEXPR const u32 offset = (_Offset % 32);
        CONSTEXPR const u32 index = (_Offset / 32);

        return ((data[index] >> offset) & mask);
    }
}
//----------------------------------------------------------------------------
template <u32 _Bits, u32 _Offset>
i32 DecodeIntScalar_(const TStaticArray<u32, 4>& data) NOEXCEPT {
    const u32 value = DecodeUIntScalar_<_Bits, _Offset>(data);

    IF_CONSTEXPR( _Bits == 0 )
        return 0;
    else IF_CONSTEXPR( _Bits == 32 )
        return static_cast<i32>(value);
    else {
        return (value >> (_Bits - 1)
            ? -static_cast<int>(value)
            : static_cast<int>(value) );
    }
}
//----------------------------------------------------------------------------
template <u32 _R, u32 _G, u32 _B, u32 _A>
void DecodeUIntVector_(FRgba32u* __restrict outv, FRawMemoryConst in) NOEXCEPT {
    STATIC_ASSERT(Meta::IsAlignedPow2(8, _R + _G + _B + _A)); // must be aligned on 8 bits boundary

    TStaticArray<u32, 4> bits;
    Assert((_R + _G + _B + _A) / 8 <= bits.size() * sizeof(u32));
    FPlatformMemory::Memcpy(bits.data(), in.data(), Min(sizeof(bits), (_R + _G + _B + _A + 7) / 8_size_t));

    outv->x = DecodeUIntScalar_<_R, 0>(bits);
    outv->y = DecodeUIntScalar_<_G, _R>(bits);
    outv->z = DecodeUIntScalar_<_B, _R + _G>(bits);
    outv->w = DecodeUIntScalar_<_A, _R + _G + _B>(bits);
}
//----------------------------------------------------------------------------
template <u32 _R, u32 _G, u32 _B, u32 _A>
void DecodeIntVector_(FRgba32i* __restrict outv, FRawMemoryConst in) NOEXCEPT {
    STATIC_ASSERT(Meta::IsAlignedPow2(8, _R + _G + _B + _A)); // must be aligned on 8 bits boundary

    TStaticArray<u32, 4> bits;
    Assert((_R + _G + _B + _A) / 8 <= bits.size() * sizeof(u32));
    FPlatformMemory::Memcpy(bits.data(), in.data(), Min(sizeof(bits), (_R + _G + _B + _A + 7) / 8_size_t));

    outv->x = DecodeIntScalar_<_R, 0>(bits);
    outv->y = DecodeIntScalar_<_G, _R>(bits);
    outv->z = DecodeIntScalar_<_B, _R + _G>(bits);
    outv->w = DecodeIntScalar_<_A, _R + _G + _B>(bits);
}
//----------------------------------------------------------------------------
template <u32 _R, u32 _G, u32 _B, u32 _A>
void DecodeUNormVector_(FRgba32f* __restrict outv, FRawMemoryConst in) NOEXCEPT {
    FRgba32u c;
    DecodeUIntVector_<_R, _G, _B, _A>(&c, in);

    outv->x = ScaleUNorm<_R>(c.x);
    outv->y = ScaleUNorm<_G>(c.y);
    outv->z = ScaleUNorm<_B>(c.z);
    outv->w = ScaleUNorm<_A>(c.w);
}
//----------------------------------------------------------------------------
template <u32 _R, u32 _G, u32 _B, u32 _A>
void DecodeSNormVector_(FRgba32f* __restrict outv, FRawMemoryConst in) NOEXCEPT {
    FRgba32i c;
    DecodeIntVector_<_R, _G, _B, _A>(&c, in);

    outv->x = ScaleSNorm<_R>(c.x);
    outv->y = ScaleSNorm<_G>(c.y);
    outv->z = ScaleSNorm<_B>(c.z);
    outv->w = ScaleSNorm<_A>(c.w);
}
//----------------------------------------------------------------------------
template <u32 _R, u32 _G, u32 _B, u32 _A>
void DecodeFloatVector_(FRgba32f* __restrict outv, FRawMemoryConst in) NOEXCEPT {
    IF_CONSTEXPR( _R == 16 ) {
        STATIC_ASSERT(Meta::IsAlignedPow2(16, _R + _G + _B + _A));

        TStaticArray<ushort, 4> bits;
        Assert((_R + _G + _B + _A) / 8 <= bits.size() * sizeof(ushort));
        FPlatformMemory::Memcpy(bits.data(), in.data(), Min(sizeof(bits), (_R + _G + _B + _A + 7) / 8_size_t));

        outv->x = FP16_to_FP32(bits[0]);
        outv->y = FP16_to_FP32(bits[(_R) / 16]);
        outv->z = FP16_to_FP32(bits[(_R + _G) / 16]);
        outv->w = FP16_to_FP32(bits[(_R + _G + _B) / 16]);
    }
    else IF_CONSTEXPR ( _R == 32 ) {
        STATIC_ASSERT(Meta::IsAlignedPow2(32, _R + _G + _B + _A));

        in.Cast<const float>().CopyTo(MakeView(outv->data));
    }
    else IF_CONSTEXPR( _R == 10 && _G == 10 && _B == 10 && _A == 2 ) {
        STATIC_ASSERT(sizeof(UX10Y10Z10W2N) == sizeof(u32));

        UX10Y10Z10W2N bits;
        FPlatformMemory::Memcpy(&bits, in.data(), sizeof(bits));

        *outv = bits.Unpack();
    }
    else IF_CONSTEXPR( _R == 11 && _G == 11 && _B == 10 && _A == 0 ) {
        STATIC_ASSERT(sizeof(UX11Y11Z10) == sizeof(u32));

        UX11Y11Z10 bits;
        FPlatformMemory::Memcpy(&bits, in.data(), sizeof(bits));

        *outv = float4(bits.Unpack(), 0);
    }
    else {
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <u32 _Bits, u32 _Offset>
void EncodeUIntScalar_(TStaticArray<u32, 4>* outp, u32 value) NOEXCEPT {
    STATIC_ASSERT(_Bits <= 32);
    STATIC_ASSERT(_Bits + (_Offset & 31) <= 32);

    IF_CONSTEXPR( _Bits == 0 ) {
        (void)outp;
        (void)value;
    }
    else {
        CONSTEXPR const u32 mask = (~0u >> (32 - _Bits));
        CONSTEXPR const u32 offset = (_Offset % 32);
        CONSTEXPR const u32 index = (_Offset / 32);

        (*outp)[index] |= (value & mask) << offset;
    }
}
//----------------------------------------------------------------------------
template <u32 _Bits, u32 _Offset>
void EncodeIntScalar_(TStaticArray<u32, 4>* outp, i32 value) NOEXCEPT {
    IF_CONSTEXPR( _Bits == 0 ) {
        return;
    } else IF_CONSTEXPR( _Bits == 32 ) {
        union { i32 i; u32 u; } bits{ value };
        EncodeUIntScalar_<_Bits, _Offset>(outp, bits.u);
    } else {
        EncodeUIntScalar_<_Bits, _Offset>(outp, value < 0 ? -value : value);
    }
}
//----------------------------------------------------------------------------
template <u32 _R, u32 _G, u32 _B, u32 _A>
void EncodeUIntVector_(FRawMemory outp, const FRgba32u& value) NOEXCEPT {
    STATIC_ASSERT(Meta::IsAlignedPow2(8, _R + _G + _B + _A)); // must be aligned on 8 bits boundary

    TStaticArray<u32, 4> bits;
    EncodeUIntScalar_<_R, 0>(&bits, value.x);
    EncodeUIntScalar_<_G, _R>(&bits, value.y);
    EncodeUIntScalar_<_B, _R + _G>(&bits, value.z);
    EncodeUIntScalar_<_A, _R + _G + _B>(&bits, value.w);

    MakeRawConstView(bits).CutBefore((_R + _G + _B + _A) / 8).CopyTo(outp);
}
//----------------------------------------------------------------------------
template <u32 _R, u32 _G, u32 _B, u32 _A>
void EncodeIntVector_(FRawMemory outp, const FRgba32i& value) NOEXCEPT {
    STATIC_ASSERT(Meta::IsAlignedPow2(8, _R + _G + _B + _A)); // must be aligned on 8 bits boundary

    TStaticArray<u32, 4> bits;
    EncodeIntScalar_<_R, 0>(&bits, value.x);
    EncodeIntScalar_<_G, _R>(&bits, value.y);
    EncodeIntScalar_<_B, _R + _G>(&bits, value.z);
    EncodeIntScalar_<_A, _R + _G + _B>(&bits, value.w);

    MakeRawConstView(bits).CutBefore((_R + _G + _B + _A) / 8).CopyTo(outp);
}
//----------------------------------------------------------------------------
template <u32 _R, u32 _G, u32 _B, u32 _A>
void EncodeUNormVector_(FRawMemory outp, const FRgba32f& value) NOEXCEPT {
    const FRgba32u c{
        UnscaleUNorm<_R>(value.x),
        UnscaleUNorm<_G>(value.y),
        UnscaleUNorm<_B>(value.z),
        UnscaleUNorm<_A>(value.w)
    };

    EncodeUIntVector_<_R, _G, _B, _A>(outp, c);
}
//----------------------------------------------------------------------------
template <u32 _R, u32 _G, u32 _B, u32 _A>
void EncodeSNormVector_(FRawMemory outp, const FRgba32f& value) NOEXCEPT {
    const FRgba32i c{
        UnscaleSNorm<_R>(value.x),
        UnscaleSNorm<_G>(value.y),
        UnscaleSNorm<_B>(value.z),
        UnscaleSNorm<_A>(value.w)
    };

    EncodeIntVector_<_R, _G, _B, _A>(outp, c);
}
//----------------------------------------------------------------------------
template <u32 _R, u32 _G, u32 _B, u32 _A>
void EncodeFloatVector_(FRawMemory outp, const FRgba32f& value) NOEXCEPT {
    IF_CONSTEXPR( _R == 16 ) {
        STATIC_ASSERT(Meta::IsAlignedPow2(16, _R + _G + _B + _A));

        TStaticArray<ushort, 4> bits{};
        bits[0] = FP32_to_FP16(value.x);
        bits[(_R) / 16] = FP32_to_FP16(value.y);
        bits[(_R + _G) / 16] = FP32_to_FP16(value.z);
        bits[(_R + _G + _B) / 16] = FP32_to_FP16(value.w);

        MakeRawConstView(bits).CutBefore((_R + _G + _B + _A) / 8).CopyTo(outp);
    }
    else IF_CONSTEXPR ( _R == 32 ) {
        STATIC_ASSERT(Meta::IsAlignedPow2(32, _R + _G + _B + _A));

        TStaticArray<float, 4> bits{};
        bits[0] = value.x;
        bits[(_R) / 32] = value.y;
        bits[(_R + _G) / 32] = value.z;
        bits[(_R + _G + _B) / 32] = value.w;

        MakeRawConstView(bits).CutBefore((_R + _G + _B + _A) / 8).CopyTo(outp);
    }
    else IF_CONSTEXPR( _R == 10 && _G == 10 && _B == 10 && _A == 2 ) {
        STATIC_ASSERT(sizeof(UX10Y10Z10W2N) == sizeof(u32));

        const UX10Y10Z10W2N bits{ value };
        MakeRawConstView(bits).CopyTo(outp);
    }
    else IF_CONSTEXPR( _R == 11 && _G == 11 && _B == 10 && _A == 0 ) {
        STATIC_ASSERT(sizeof(UX11Y11Z10) == sizeof(u32));

        const UX11Y11Z10 bits{ value.xyz };
        MakeRawConstView(bits).CopyTo(outp);
    }
    else {
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPixelFormatEncoding EPixelFormat_Encoding(EPixelFormat format, EImageAspect aspect) NOEXCEPT {
    UNUSED(aspect);
    switch (format) {

    /// packed vectors

    case EPixelFormat::RGBA4_UNorm:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            4 * 4,
            &DecodeUNormVector_<4, 4, 4, 4>,
            &DecodeIntVector_<4, 4, 4, 4>,
            &DecodeUIntVector_<4, 4, 4, 4>,
            &EncodeUNormVector_<4, 4, 4, 4>,
            &EncodeIntVector_<4, 4, 4, 4>,
            &EncodeUIntVector_<4, 4, 4, 4>,
        };

    case EPixelFormat::RGB5_A1_UNorm:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            3 * 5 + 1,
            &DecodeUNormVector_<5, 5, 5, 1>,
            &DecodeIntVector_<5, 5, 5, 1>,
            &DecodeUIntVector_<5, 5, 5, 1>,
            &EncodeUNormVector_<5, 5, 5, 1>,
            &EncodeIntVector_<5, 5, 5, 1>,
            &EncodeUIntVector_<5, 5, 5, 1>,
        };

    case EPixelFormat::RGB_5_6_5_UNorm:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            5 + 6 + 5,
            &DecodeUNormVector_<5, 6, 5, 0>,
            &DecodeIntVector_<5, 6, 5, 0>,
            &DecodeUIntVector_<5, 6, 5, 0>,
            &EncodeUNormVector_<5, 6, 5, 0>,
            &EncodeIntVector_<5, 6, 5, 0>,
            &EncodeUIntVector_<5, 6, 5, 0>,
        };

    case EPixelFormat::RGB10_A2_UNorm:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            3 * 10 + 2,
            &DecodeUNormVector_<10, 10, 10, 2>,
            &DecodeIntVector_<10, 10, 10, 2>,
            &DecodeUIntVector_<10, 10, 10, 2>,
            &EncodeUNormVector_<10, 10, 10, 2>,
            &EncodeIntVector_<10, 10, 10, 2>,
            &EncodeUIntVector_<10, 10, 10, 2>,
        };
    case EPixelFormat::RGB10_A2u:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            3 * 10 + 2,
            nullptr,
            &DecodeIntVector_<10, 10, 10, 2>,
            &DecodeUIntVector_<10, 10, 10, 2>,
            nullptr,
            &EncodeIntVector_<10, 10, 10, 2>,
            &EncodeUIntVector_<10, 10, 10, 2>,
        };

    case EPixelFormat::RGB_11_11_10f:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            2 * 11 + 10,
            &DecodeFloatVector_<11, 11, 10, 0>,
            nullptr,
            nullptr,
            &EncodeFloatVector_<11, 11, 10, 0>,
            nullptr,
            nullptr,
        };

    /// int 8-bits

    case EPixelFormat::sRGB8 :
    case EPixelFormat::sBGR8 :
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            3 * 8,
            &DecodeUNormVector_<8, 8, 8, 0>,
            &DecodeIntVector_<8, 8, 8, 0>,
            &DecodeUIntVector_<8, 8, 8, 0>,
            &EncodeUNormVector_<8, 8, 8, 0>,
            &EncodeIntVector_<8, 8, 8, 0>,
            &EncodeUIntVector_<8, 8, 8, 0>,
        };

    case EPixelFormat::sRGB8_A8 :
    case EPixelFormat::sBGR8_A8 :
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            4 * 8,
            &DecodeUNormVector_<8, 8, 8, 8>,
            &DecodeIntVector_<8, 8, 8, 8>,
            &DecodeUIntVector_<8, 8, 8, 8>,
            &EncodeUNormVector_<8, 8, 8, 8>,
            &EncodeIntVector_<8, 8, 8, 8>,
            &EncodeUIntVector_<8, 8, 8, 8>,
        };

    case EPixelFormat::Depth16:
    case EPixelFormat::Depth24:
    case EPixelFormat::Depth32f:
        Assert_NoAssume(EImageAspect::Depth == aspect);
        AssertNotImplemented(); // #TODO ?

    case EPixelFormat::Depth16_Stencil8:
    case EPixelFormat::Depth24_Stencil8:
    case EPixelFormat::Depth32F_Stencil8:
        Assert_NoAssume(EImageAspect::Depth == aspect || EImageAspect::Stencil == aspect);
        AssertNotImplemented(); // #TODO ?

    case EPixelFormat::R8_SNorm:
    case EPixelFormat::R8_UNorm:
    case EPixelFormat::R8i:
    case EPixelFormat::R8u:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            1 * 8,
            (
                format == EPixelFormat::R8_UNorm ? &DecodeUNormVector_<8,0,0,0> :
                format == EPixelFormat::R8_SNorm ? &DecodeSNormVector_<8,0,0,0> :
                nullptr ),
            &DecodeIntVector_<8,0,0,0>,
            &DecodeUIntVector_<8,0,0,0>,
            (
                format == EPixelFormat::R8_UNorm ? &EncodeUNormVector_<8,0,0,0> :
                format == EPixelFormat::R8_SNorm ? &EncodeSNormVector_<8,0,0,0> :
                nullptr ),
            &EncodeIntVector_<8,0,0,0>,
            &EncodeUIntVector_<8,0,0,0>,
        };

    case EPixelFormat::RG8_SNorm:
    case EPixelFormat::RG8_UNorm:
    case EPixelFormat::RG8i:
    case EPixelFormat::RG8u:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            2 * 8,
            (
                format == EPixelFormat::RG8_UNorm ? &DecodeUNormVector_<8,8,0,0> :
                format == EPixelFormat::RG8_SNorm ? &DecodeSNormVector_<8,8,0,0> :
                nullptr  ),
            &DecodeIntVector_<8,8,0,0>,
            &DecodeUIntVector_<8,8,0,0>,
            (
                format == EPixelFormat::RG8_UNorm ? &EncodeUNormVector_<8,8,0,0> :
                format == EPixelFormat::RG8_SNorm ? &EncodeSNormVector_<8,8,0,0> :
                nullptr  ),
            &EncodeIntVector_<8,8,0,0>,
            &EncodeUIntVector_<8,8,0,0>,
        };

    case EPixelFormat::RGB8_SNorm:
    case EPixelFormat::RGB8_UNorm:
    case EPixelFormat::BGR8_UNorm:
    case EPixelFormat::RGB8i:
    case EPixelFormat::RGB8u:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            3 * 8,
            (
                format == EPixelFormat::RGB8_SNorm ? &DecodeSNormVector_<8,8,8,0> :
                format == EPixelFormat::RGB8_UNorm ? &DecodeUNormVector_<8,8,8,0> :
                format == EPixelFormat::BGR8_UNorm ? &DecodeUNormVector_<8,8,8,0> :
                nullptr  ),
            &DecodeIntVector_<8,8,8,0>,
            &DecodeUIntVector_<8,8,8,0>,
            (
                format == EPixelFormat::RGB8_SNorm ? &EncodeSNormVector_<8,8,8,0> :
                format == EPixelFormat::RGB8_UNorm ? &EncodeUNormVector_<8,8,8,0> :
                format == EPixelFormat::BGR8_UNorm ? &EncodeUNormVector_<8,8,8,0> :
                nullptr  ),
            &EncodeIntVector_<8,8,8,0>,
            &EncodeUIntVector_<8,8,8,0>,
        };

    case EPixelFormat::RGBA8_SNorm:
    case EPixelFormat::RGBA8_UNorm:
    case EPixelFormat::BGRA8_UNorm:
    case EPixelFormat::RGBA8i:
    case EPixelFormat::RGBA8u:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            4 * 8,
            (
                format == EPixelFormat::RGBA8_SNorm ? &DecodeSNormVector_<8,8,8,8> :
                format == EPixelFormat::RGBA8_UNorm ? &DecodeUNormVector_<8,8,8,8> :
                format == EPixelFormat::BGRA8_UNorm ? &DecodeUNormVector_<8,8,8,8> :
                nullptr  ),
            &DecodeIntVector_<8,8,8,8>,
            &DecodeUIntVector_<8,8,8,8>,
            (
                format == EPixelFormat::RGBA8_SNorm ? &EncodeSNormVector_<8,8,8,8> :
                format == EPixelFormat::RGBA8_UNorm ? &EncodeUNormVector_<8,8,8,8> :
                format == EPixelFormat::BGRA8_UNorm ? &EncodeUNormVector_<8,8,8,8> :
                nullptr  ),
            &EncodeIntVector_<8,8,8,8>,
            &EncodeUIntVector_<8,8,8,8>,
        };

    /// int 16-bits

    case EPixelFormat::R16_SNorm:
    case EPixelFormat::R16_UNorm:
    case EPixelFormat::R16i:
    case EPixelFormat::R16u:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            1 * 16,
            (
                format == EPixelFormat::R16_UNorm ? &DecodeUNormVector_<16,0,0,0> :
                format == EPixelFormat::R16_SNorm ? &DecodeSNormVector_<16,0,0,0> :
                nullptr ),
            &DecodeIntVector_<16,0,0,0>,
            &DecodeUIntVector_<16,0,0,0>,
            (
                format == EPixelFormat::R16_UNorm ? &EncodeUNormVector_<16,0,0,0> :
                format == EPixelFormat::R16_SNorm ? &EncodeSNormVector_<16,0,0,0> :
                nullptr ),
            &EncodeIntVector_<16,0,0,0>,
            &EncodeUIntVector_<16,0,0,0>,
        };

    case EPixelFormat::RG16_SNorm:
    case EPixelFormat::RG16_UNorm:
    case EPixelFormat::RG16i:
    case EPixelFormat::RG16u:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            2 * 16,
            (
                format == EPixelFormat::RG16_UNorm ? &DecodeUNormVector_<16,16,0,0> :
                format == EPixelFormat::RG16_SNorm ? &DecodeSNormVector_<16,16,0,0> :
                nullptr  ),
            &DecodeIntVector_<16,16,0,0>,
            &DecodeUIntVector_<16,16,0,0>,
            (
                format == EPixelFormat::RG16_UNorm ? &EncodeUNormVector_<16,16,0,0> :
                format == EPixelFormat::RG16_SNorm ? &EncodeSNormVector_<16,16,0,0> :
                nullptr  ),
            &EncodeIntVector_<16,16,0,0>,
            &EncodeUIntVector_<16,16,0,0>,
        };

    case EPixelFormat::RGB16_SNorm:
    case EPixelFormat::RGB16_UNorm:
    case EPixelFormat::RGB16i:
    case EPixelFormat::RGB16u:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            3 * 16,
            (
                format == EPixelFormat::RGB16_SNorm ? &DecodeSNormVector_<16,16,16,0> :
                format == EPixelFormat::RGB16_UNorm ? &DecodeUNormVector_<16,16,16,0> :
                nullptr  ),
            &DecodeIntVector_<16,16,16,0>,
            &DecodeUIntVector_<16,16,16,0>,
            (
                format == EPixelFormat::RGB16_SNorm ? &EncodeSNormVector_<16,16,16,0> :
                format == EPixelFormat::RGB16_UNorm ? &EncodeUNormVector_<16,16,16,0> :
                nullptr  ),
            &EncodeIntVector_<16,16,16,0>,
            &EncodeUIntVector_<16,16,16,0>,
        };

    case EPixelFormat::RGBA16_SNorm:
    case EPixelFormat::RGBA16_UNorm:
    case EPixelFormat::RGBA16i:
    case EPixelFormat::RGBA16u:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            4 * 16,
            (
                format == EPixelFormat::RGBA16_SNorm ? &DecodeSNormVector_<16,16,16,16> :
                format == EPixelFormat::RGBA16_UNorm ? &DecodeUNormVector_<16,16,16,16> :
                nullptr  ),
            &DecodeIntVector_<16,16,16,16>,
            &DecodeUIntVector_<16,16,16,16>,
            (
                format == EPixelFormat::RGBA16_SNorm ? &EncodeSNormVector_<16,16,16,16> :
                format == EPixelFormat::RGBA16_UNorm ? &EncodeUNormVector_<16,16,16,16> :
                nullptr  ),
            &EncodeIntVector_<16,16,16,16>,
            &EncodeUIntVector_<16,16,16,16>,
        };

    /// int 32-bits

    case EPixelFormat::R32i:
    case EPixelFormat::R32u:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            1 * 32,
            nullptr,
            &DecodeIntVector_<32,0,0,0>,
            &DecodeUIntVector_<32,0,0,0>,
            nullptr,
            &EncodeIntVector_<32,0,0,0>,
            &EncodeUIntVector_<32,0,0,0>,
        };

    case EPixelFormat::RG32i:
    case EPixelFormat::RG32u:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            2 * 32,
            nullptr,
            &DecodeIntVector_<32,32,0,0>,
            &DecodeUIntVector_<32,32,0,0>,
            nullptr,
            &EncodeIntVector_<32,32,0,0>,
            &EncodeUIntVector_<32,32,0,0>,
        };

    case EPixelFormat::RGB32i:
    case EPixelFormat::RGB32u:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            3 * 32,
            nullptr,
            &DecodeIntVector_<32,32,32,0>,
            &DecodeUIntVector_<32,32,32,0>,
            nullptr,
            &EncodeIntVector_<32,32,32,0>,
            &EncodeUIntVector_<32,32,32,0>,
        };

    case EPixelFormat::RGBA32i:
    case EPixelFormat::RGBA32u:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            4 * 32,
            nullptr,
            &DecodeIntVector_<32,32,32,32>,
            &DecodeUIntVector_<32,32,32,32>,
            nullptr,
            &EncodeIntVector_<32,32,32,32>,
            &EncodeUIntVector_<32,32,32,32>,
        };

    // half-floats

    case EPixelFormat::R16f:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            1 * 16,
            &DecodeFloatVector_<16,0,0,0>,
            nullptr,
            nullptr,
            &EncodeFloatVector_<16,0,0,0>,
            nullptr,
            nullptr,
       };

    case EPixelFormat::RG16f:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            2 * 16,
            &DecodeFloatVector_<16,16,0,0>,
            nullptr,
            nullptr,
            &EncodeFloatVector_<16,16,0,0>,
            nullptr,
            nullptr,
       };

    case EPixelFormat::RGB16f:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            3 * 16,
            &DecodeFloatVector_<16,16,16,0>,
            nullptr,
            nullptr,
            &EncodeFloatVector_<16,16,16,0>,
            nullptr,
            nullptr,
       };

    case EPixelFormat::RGBA16f:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            4 * 16,
            &DecodeFloatVector_<16,16,16,16>,
            nullptr,
            nullptr,
            &EncodeFloatVector_<16,16,16,16>,
            nullptr,
            nullptr,
       };

    // floats

    case EPixelFormat::R32f:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            1 * 32,
            &DecodeFloatVector_<32,0,0,0>,
            nullptr,
            nullptr,
            &EncodeFloatVector_<32,0,0,0>,
            nullptr,
            nullptr,
       };

    case EPixelFormat::RG32f:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            2 * 32,
            &DecodeFloatVector_<32,32,0,0>,
            nullptr,
            nullptr,
            &EncodeFloatVector_<32,32,0,0>,
            nullptr,
            nullptr,
       };

    case EPixelFormat::RGB32f:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            3 * 32,
            &DecodeFloatVector_<32,32,32,0>,
            nullptr,
            nullptr,
            Default, // #TODO
            Default, // #TODO
            Default, // #TODO
       };

    case EPixelFormat::RGBA32f:
        Assert_NoAssume(EImageAspect::Color == aspect);
        return FPixelFormatEncoding{
            4 * 32,
            &DecodeFloatVector_<32,32,32,32>,
            nullptr,
            nullptr,
            &EncodeFloatVector_<32,32,32,32>,
            nullptr,
            nullptr,
       };

    // macro-blocks

    case EPixelFormat::BC1_RGB8_UNorm:
    case EPixelFormat::BC1_sRGB8:
    case EPixelFormat::BC1_RGB8_A1_UNorm:
    case EPixelFormat::BC1_sRGB8_A1:
    case EPixelFormat::BC2_RGBA8_UNorm:
    case EPixelFormat::BC2_sRGB8_A8:
    case EPixelFormat::BC3_RGBA8_UNorm:
    case EPixelFormat::BC3_sRGB8:
    case EPixelFormat::BC4_R8_SNorm:
    case EPixelFormat::BC4_R8_UNorm:
    case EPixelFormat::BC5_RG8_SNorm:
    case EPixelFormat::BC5_RG8_UNorm:
    case EPixelFormat::BC7_RGBA8_UNorm:
    case EPixelFormat::BC7_sRGB8_A8:
    case EPixelFormat::BC6H_RGB16F:
    case EPixelFormat::BC6H_RGB16UF:
    case EPixelFormat::ETC2_RGB8_UNorm:
    case EPixelFormat::ECT2_sRGB8:
    case EPixelFormat::ETC2_RGB8_A1_UNorm:
    case EPixelFormat::ETC2_sRGB8_A1:
    case EPixelFormat::ETC2_RGBA8_UNorm:
    case EPixelFormat::ETC2_sRGB8_A8:
    case EPixelFormat::EAC_R11_SNorm:
    case EPixelFormat::EAC_R11_UNorm:
    case EPixelFormat::EAC_RG11_SNorm:
    case EPixelFormat::EAC_RG11_UNorm:
    case EPixelFormat::ASTC_RGBA_4x4:
    case EPixelFormat::ASTC_RGBA_5x4:
    case EPixelFormat::ASTC_RGBA_5x5:
    case EPixelFormat::ASTC_RGBA_6x5:
    case EPixelFormat::ASTC_RGBA_6x6:
    case EPixelFormat::ASTC_RGBA_8x5:
    case EPixelFormat::ASTC_RGBA_8x6:
    case EPixelFormat::ASTC_RGBA_8x8:
    case EPixelFormat::ASTC_RGBA_10x5:
    case EPixelFormat::ASTC_RGBA_10x6:
    case EPixelFormat::ASTC_RGBA_10x8:
    case EPixelFormat::ASTC_RGBA_10x10:
    case EPixelFormat::ASTC_RGBA_12x10:
    case EPixelFormat::ASTC_RGBA_12x12:
    case EPixelFormat::ASTC_sRGB8_A8_4x4:
    case EPixelFormat::ASTC_sRGB8_A8_5x4:
    case EPixelFormat::ASTC_sRGB8_A8_5x5:
    case EPixelFormat::ASTC_sRGB8_A8_6x5:
    case EPixelFormat::ASTC_sRGB8_A8_6x6:
    case EPixelFormat::ASTC_sRGB8_A8_8x5:
    case EPixelFormat::ASTC_sRGB8_A8_8x6:
    case EPixelFormat::ASTC_sRGB8_A8_8x8:
    case EPixelFormat::ASTC_sRGB8_A8_10x5:
    case EPixelFormat::ASTC_sRGB8_A8_10x6:
    case EPixelFormat::ASTC_sRGB8_A8_10x8:
    case EPixelFormat::ASTC_sRGB8_A8_10x10:
    case EPixelFormat::ASTC_sRGB8_A8_12x10:
    case EPixelFormat::ASTC_sRGB8_A8_12x12:
        Assert_NoAssume(EImageAspect::Color == aspect);
        AssertNotImplemented(); // #TODO encode/decode block compression ?

    case EPixelFormat::_Count:
    case EPixelFormat::Unknown:
        AssertNotReached(); // internal use only
    }

    AssertNotReached(); // new format missing in switch ?
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
