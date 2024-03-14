#pragma once

#include "RHI_fwd.h"

#include "RHI/ResourceEnums.h"

#include "Maths/Range.h"
#include "Meta/Enum.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct EPixelValueType {
    enum ETypes : u8 {
        SFloat          = 1 << 0,
        UFloat          = 1 << 1,
        UNorm           = 1 << 2,
        SNorm           = 1 << 3,
        Int             = 1 << 4,
        UInt            = 1 << 5,
        Depth           = 1 << 6,
        Stencil         = 1 << 7,
    };
    ENUM_FLAGS_FRIEND(ETypes);

    static CONSTEXPR ETypes AnyFloat{ u8(SFloat)|u8(UFloat)|u8(UNorm)|u8(SNorm) };
    static CONSTEXPR ETypes DepthStencil{ u8(Depth)|u8(Stencil) };

    enum EFlags : u8 {
        sRGB            = 1 << 0,
    };
    ENUM_FLAGS_FRIEND(EFlags);

    ETypes Types{ Zero };
    EFlags Flags{ Zero };

    friend hash_t hash_value(EPixelValueType value) NOEXCEPT {
        return hash_as_pod(value);
    }

    CONSTEXPR EPixelValueType operator +(ETypes type) const { return {Types + type, Flags}; }
    CONSTEXPR EPixelValueType operator -(ETypes type) const { return {Types - type, Flags}; }
    CONSTEXPR EPixelValueType operator |(ETypes type) const { return {Types | type, Flags}; }

    CONSTEXPR EPixelValueType& operator +=(ETypes type) { Types = (Types + type); return (*this); }
    CONSTEXPR EPixelValueType& operator -=(ETypes type) { Types = (Types - type); return (*this); }
    CONSTEXPR EPixelValueType& operator |=(ETypes type) { Types = (Types | type); return (*this); }

    CONSTEXPR bool operator &(ETypes type) const { return (Types & type); }
    CONSTEXPR bool operator ^(ETypes type) const { return (Types ^ type); }

    CONSTEXPR EPixelValueType operator +(EFlags flag) const { return {Types, Flags + flag}; }
    CONSTEXPR EPixelValueType operator -(EFlags flag) const { return {Types, Flags - flag}; }
    CONSTEXPR EPixelValueType operator |(EFlags flag) const { return {Types, Flags | flag}; }

    CONSTEXPR EPixelValueType& operator +=(EFlags flag) { Flags = (Flags + flag); return (*this); }
    CONSTEXPR EPixelValueType& operator -=(EFlags flag) { Flags = (Flags - flag); return (*this); }
    CONSTEXPR EPixelValueType& operator |=(EFlags flag) { Flags = (Flags | flag); return (*this); }

    CONSTEXPR bool operator &(EFlags flag) const { return (Flags & flag); }
    CONSTEXPR bool operator ^(EFlags flag) const { return (Flags ^ flag); }

    CONSTEXPR bool operator ==(EPixelValueType o) const { return (Types == o.Types && Flags == o.Flags); }
    CONSTEXPR bool operator !=(EPixelValueType o) const { return (not operator ==(o)); }
};
STATIC_ASSERT(sizeof(EPixelValueType) == sizeof(u16));
inline CONSTEXPR EPixelValueType EPixelValueType_DepthStencil{ EPixelValueType::DepthStencil, Zero } ;
//----------------------------------------------------------------------------
struct FPixelFormatInfo {
    EPixelFormat Format{ Default };
    EImageAspect AspectMask{ Default };
    EPixelValueType ValueType{ Default };
    uint2 BlockDim{ 1 };
    u32 BitsPerBlock0{ 0 }; // color / depth / etc.
    u32 BitsPerBlock1{ 0 }; // stencil
    u32 Channels{ 0 };

    FPixelFormatInfo() = default;

    CONSTEXPR FPixelFormatInfo(EPixelFormat fmt, u32 bpp, u32 channels, EPixelValueType type, EImageAspect aspect = EImageAspect::Color) NOEXCEPT
        : Format(fmt), AspectMask(aspect), ValueType(type), BitsPerBlock0(bpp), Channels(channels)
    {}

    CONSTEXPR FPixelFormatInfo(EPixelFormat fmt, u32 bpp, const uint2& dim, u32 channels, EPixelValueType type, EImageAspect aspect = EImageAspect::Color) NOEXCEPT
        : Format(fmt), AspectMask(aspect), ValueType(type), BlockDim(dim), BitsPerBlock0(bpp), Channels(channels)
    {}

    CONSTEXPR FPixelFormatInfo(EPixelFormat fmt, const uint2& bppDepthStencil, EPixelValueType type = EPixelValueType_DepthStencil, EImageAspect aspect = EImageAspect_DepthStencil) NOEXCEPT
        : Format(fmt), AspectMask(aspect), ValueType(type), BitsPerBlock0(bppDepthStencil.x), BitsPerBlock1(bppDepthStencil.y)
    {}

    CONSTEXPR bool IsColor() const { return not (ValueType.Types ^ EPixelValueType::DepthStencil); }
    CONSTEXPR bool IsDepth() const { return (ValueType.Types == EPixelValueType::Depth); }
    CONSTEXPR bool IsDepthStencil() const { return (ValueType.Types == EPixelValueType::DepthStencil); }
    CONSTEXPR bool IsStencil() const { return (ValueType.Types == EPixelValueType::Stencil); }
    CONSTEXPR bool IsSRGB() const { return ValueType.Flags ^ EPixelValueType::sRGB; }

    CONSTEXPR u32 BitsPerBlock(EImageAspect aspect) const {
        Assert(AspectMask ^ aspect);
        return (aspect != EImageAspect::Stencil ? BitsPerBlock0 : BitsPerBlock1);
    }
    CONSTEXPR u32 BitsPerPixel(EImageAspect aspect) const {
        return (BitsPerBlock(aspect) / (BlockDim.x * BlockDim.y));
    }

    PPE_RHI_API size_t NumBlocks(const uint3& dimensions) const NOEXCEPT;
    PPE_RHI_API size_t NumBlocks(const uint3& dimensions, u32 numMips, u32 numSlices = 1) const NOEXCEPT;

    inline size_t SizeInBytes(EImageAspect aspect, const uint3& dimensions) const {
        return ((NumBlocks(dimensions) * BitsPerBlock(aspect)) / 8);
    }
    inline size_t SizeInBytes(EImageAspect aspect, const uint3& dimensions, u32 numMips, u32 numSlices = 1) const {
        return ((NumBlocks(dimensions, numMips, numSlices) * BitsPerBlock(aspect)) / 8);
    }

    PPE_RHI_API FBytesRange MipRange(const uint3& dimensions, u32 mipBias, u32 numMips = 1) const NOEXCEPT;

    inline size_t RowPitch(EImageAspect aspect, const uint3& dimensions) const NOEXCEPT {
        return SizeInBytes(aspect, { dimensions.x, BlockDim.y, 1 }, 1);
    }
    inline size_t SlicePitch(EImageAspect aspect, const uint3& dimensions, u32 numMips) const NOEXCEPT {
        return SizeInBytes(aspect, dimensions, numMips);
    }

    inline FBytesRange SliceRange(EImageAspect aspect, const uint3& dimensions, u32 numMips, u32 sliceIndex) const NOEXCEPT {
        const size_t pitch = SlicePitch(aspect, dimensions, numMips);
        return { pitch * sliceIndex, pitch * (sliceIndex + 1) };
    }

    PPE_RHI_API static uint3 NextMipDimensions(const uint3& dimensions) NOEXCEPT;
    PPE_RHI_API static u32 FullMipCount(const uint3& dimensions, const uint2& blockDim) NOEXCEPT;

    inline u32 FullMipCount(const uint3& dimensions) const { return FullMipCount(dimensions, BlockDim); }
};
//----------------------------------------------------------------------------
PPE_RHI_API FPixelFormatInfo EPixelFormat_Infos(EPixelFormat fmt) NOEXCEPT;
//----------------------------------------------------------------------------
inline u32 EPixelFormat_BitsPerPixel(EPixelFormat fmt, EImageAspect aspect) {
    return EPixelFormat_Infos(fmt).BitsPerPixel(aspect);
}
//----------------------------------------------------------------------------
inline EImageAspect EPixelFormat_ToImageAspect(EPixelFormat fmt) {
    const auto info = EPixelFormat_Infos(fmt);
    const EImageAspect color = (not (info.ValueType ^ EPixelValueType::DepthStencil) ? EImageAspect::Color : Default);
    const EImageAspect depth = (info.ValueType ^ EPixelValueType::Depth ? EImageAspect::Depth : Default);
    const EImageAspect stencil = (info.ValueType ^ EPixelValueType::Stencil ? EImageAspect::Stencil : Default);
    return (color | depth | stencil);
}
//----------------------------------------------------------------------------
// IsXXX
//----------------------------------------------------------------------------
inline bool EPixelFormat_IsDepth(EPixelFormat fmt) {
    return EPixelFormat_Infos(fmt).IsDepth();
}
//----------------------------------------------------------------------------
inline bool EPixelFormat_IsStencil(EPixelFormat fmt) {
    return EPixelFormat_Infos(fmt).IsStencil();
}
//----------------------------------------------------------------------------
inline bool EPixelFormat_IsDepthStencil(EPixelFormat fmt) {
    return EPixelFormat_Infos(fmt).IsDepthStencil();
}
//----------------------------------------------------------------------------
inline bool EPixelFormat_IsColor(EPixelFormat fmt) {
    return EPixelFormat_Infos(fmt).IsColor();
}
//----------------------------------------------------------------------------
// HasXXX
//----------------------------------------------------------------------------
inline bool EPixelFormat_HasDepth(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType & EPixelValueType::Depth);
}
//----------------------------------------------------------------------------
inline bool EPixelFormat_HasStencil(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType & EPixelValueType::Stencil);
}
//----------------------------------------------------------------------------
inline bool EPixelFormat_HasDepthOrStencil(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType ^ EPixelValueType::DepthStencil);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FPixelFormatEncoding {
    u32 BitsPerPixel{ 0 };

    FPixelDecodeRGBA32f DecodeRGBA32f{ nullptr };
    FPixelDecodeRGBA32i DecodeRGBA32i{ nullptr };
    FPixelDecodeRGBA32u DecodeRGBA32u{ nullptr };

    FPixelEncodeRGBA32f EncodeRGBA32f{ nullptr };
    FPixelEncodeRGBA32i EncodeRGBA32i{ nullptr };
    FPixelEncodeRGBA32u EncodeRGBA32u{ nullptr };
};
//----------------------------------------------------------------------------
PPE_RHI_API FPixelFormatEncoding EPixelFormat_Encoding(EPixelFormat format, EImageAspect aspect) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
