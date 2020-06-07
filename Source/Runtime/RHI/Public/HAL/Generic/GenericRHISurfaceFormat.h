#pragma once

#include "RHI_fwd.h"
#include "HAL/PlatformMaths.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericPixelFormat {
    R4G4_UNORM = 0,
    R4G4B4A4_UNORM,
    B4G4R4A4_UNORM,
    R5G6B5_UNORM,
    B5G6R5_UNORM,
    R5G5B5A1_UNORM,
    B5G5R5A1_UNORM,
    A1R5G5B5_UNORM,
    R8_UNORM,
    R8_SNORM,
    R8_UINT,
    R8_SINT,
    R8_SRGB,
    R8G8_UNORM,
    R8G8_SNORM,
    R8G8_USCALED,
    R8G8_SSCALED,
    R8G8_UINT,
    R8G8_SINT,
    R8G8_SRGB,
    R8G8B8A8_UNORM,
    R8G8B8A8_SNORM,
    R8G8B8A8_USCALED,
    R8G8B8A8_SSCALED,
    R8G8B8A8_UINT,
    R8G8B8A8_SINT,
    R8G8B8A8_SRGB,
    B8G8R8A8_UNORM,
    B8G8R8A8_SNORM,
    B8G8R8A8_USCALED,
    B8G8R8A8_SSCALED,
    B8G8R8A8_UINT,
    B8G8R8A8_SINT,
    B8G8R8A8_SRGB,
    A2R10G10B10_UNORM,
    A2R10G10B10_SNORM,
    A2R10G10B10_USCALED,
    A2R10G10B10_SSCALED,
    A2R10G10B10_UINT,
    A2R10G10B10_SINT,
    A2B10G10R10_UNORM,
    A2B10G10R10_SNORM,
    A2B10G10R10_USCALED,
    A2B10G10R10_SSCALED,
    A2B10G10R10_UINT,
    A2B10G10R10_SINT,
    R16_UNORM,
    R16_SNORM,
    R16_USCALED,
    R16_SSCALED,
    R16_UINT,
    R16_SINT,
    R16_SFLOAT,
    R16G16_UNORM,
    R16G16_SNORM,
    R16G16_USCALED,
    R16G16_SSCALED,
    R16G16_UINT,
    R16G16_SINT,
    R16G16_SFLOAT,
    R16G16B16A16_UNORM,
    R16G16B16A16_SNORM,
    R16G16B16A16_USCALED,
    R16G16B16A16_SSCALED,
    R16G16B16A16_UINT,
    R16G16B16A16_SINT,
    R16G16B16A16_SFLOAT,
    R32_UINT,
    R32_SINT,
    R32_SFLOAT,
    R32G32_UINT,
    R32G32_SINT,
    R32G32_SFLOAT,
    R32G32B32A32_UINT,
    R32G32B32A32_SINT,
    R32G32B32A32_SFLOAT,
    R64_UINT,
    R64_SINT,
    R64_SFLOAT,
    R64G64_UINT,
    R64G64_SINT,
    R64G64_SFLOAT,
    R64G64B64A64_UINT,
    R64G64B64A64_SINT,
    R64G64B64A64_SFLOAT,
    B10G11R11_UFLOAT,
    E5B9G9R9_UFLOAT,
    D16_UNORM,
    D32_SFLOAT,
    D16_UNORM_S8_UINT,
    D24_UNORM_S8_UINT,
    BC1_RGB_UNORM,
    BC1_RGB_SRGB,
    BC1_RGBA_UNORM,
    BC1_RGBA_SRGB,
    BC2_UNORM,
    BC2_SRGB,
    BC3_UNORM,
    BC3_SRGB,
    BC4_UNORM,
    BC4_SNORM,
    BC5_UNORM,
    BC5_SNORM,
    BC6H_UFLOAT,
    BC6H_SFLOAT,
    BC7_UNORM,
    BC7_SRGB,
};
//----------------------------------------------------------------------------
enum class EGenericColorSpace {
    PASS_THROUGH = 0,
    SRGB_NONLINEAR, // most common historically
    DISPLAY_P3_NONLINEAR,
    EXTENDED_SRGB_LINEAR,
    DISPLAY_P3_LINEAR,
    DCI_P3_NONLINEAR,
    BT709_LINEAR,
    BT709_NONLINEAR,
    BT2020_LINEAR,
    HDR10_ST2084,
    DOLBYVISION,
    HDR10_HLG,
    ADOBERGB_LINEAR,
    ADOBERGB_NONLINEAR,
    EXTENDED_SRGB_NONLINEAR,
    DISPLAY_NATIVE_AMD,
};
//----------------------------------------------------------------------------
struct FGenericPixelInfo {

    enum EChannel {
        R         = 0,
        RG        = 1,
        RGB       = 2,
        RGBA      = 3,
        BGR       = 4,
        BGRA      = 5,
        ARGB      = 6,
        ABGR      = 7,
        EBGR      = 8,
        RA        = 9,
    };

    enum ENumericFormat {
        UNORM     = 0,
        SNORM     = 1,
        USCALED   = 2,
        SSCALED   = 3,
        UINT      = 4,
        SINT      = 5,
        UFLOAT    = 6,
        SFLOAT    = 7,
        SRGB      = 8,
    };

    u32 UID                    : 8;
    u32 BlockWindowLog2        : 2; // 2^BlockExp * 2^BlockExp
    u32 BlockNumBitsLog2       : 4; // 8^BlockNumBitsExp
    u32 Channels               : 4;
    u32 NumericFormat          : 4;
    //                          22

    CONSTEXPR FGenericPixelInfo() = default;
    CONSTEXPR FGenericPixelInfo(
        u32 uid,
        u32 blockWindowLog2,
        u32 blockNumBitsLog2,
        EChannel channels,
        ENumericFormat numericFormat ) NOEXCEPT
    :   UID(uid)
    ,   Channels(channels)
    ,   BlockWindowLog2(blockWindowLog2)
    ,   BlockNumBitsLog2(blockNumBitsLog2)
    ,   NumericFormat(numericFormat)
    {}

    CONSTEXPR u32 Pitch() const { return (BlockNumBits() >> 3); }
    CONSTEXPR u32 BlockWindow() const { return (1 << BlockWindowLog2); }
    CONSTEXPR u32 BlockNumBits() const { return (1 << (3 + BlockNumBitsLog2)); }
    CONSTEXPR u32 BlockNumPixels() const { return (1 << (BlockWindowLog2 + BlockWindowLog2)); }
    CONSTEXPR u32 BitsPerPixel() const { return (BlockNumBits() / BlockNumPixels()); }

    void ImageSize(
        size_t* pNumStrides,
        size_t* pStrideInBytes,
        size_t width, size_t height ) const NOEXCEPT;
    size_t ImageSizeInBytes(
        size_t width, size_t height,
        size_t numLevels,
        size_t numSlices = 1,
        size_t levelOffset = 0 ) const NOEXCEPT;

    size_t Texture1DSizeInBytes(
        size_t length,
        size_t numLevels = 1,
        size_t levelOffset = 0 ) const NOEXCEPT {
        return ImageSizeInBytes(length, 1, numLevels, 1, levelOffset);
    }
    size_t Texture2DSizeInBytes(
        size_t width, size_t height,
        size_t numLevels = 1,
        size_t levelOffset = 0 ) const NOEXCEPT {
        return ImageSizeInBytes(width, height, numLevels, 1, levelOffset);
    }
    size_t Texture3DSizeInBytes(
        size_t width, size_t height, size_t depth,
        size_t numLevels = 1,
        size_t levelOffset = 0 ) const NOEXCEPT {
        return ImageSizeInBytes(width, height, numLevels, depth, levelOffset);
    }
    size_t TextureCubeSizeInBytes(
        size_t width, size_t height,
        size_t numLevels = 1,
        size_t levelOffset = 0 ) const NOEXCEPT {
        return ImageSizeInBytes(width, height, numLevels, 6, levelOffset);
    }

    static FGenericPixelInfo FromFormat(EGenericPixelFormat format) NOEXCEPT;

};
//----------------------------------------------------------------------------
struct FGenericSurfaceFormat {
    using EPixelFormat = EGenericPixelFormat;
    using EColorSpace = EGenericColorSpace;
    using FPixelInfo = FGenericPixelInfo;

    EPixelFormat Format;
    EColorSpace ColorSpace;

    FGenericPixelInfo PixelInfo() const { return FGenericPixelInfo::FromFormat(Format); }

    static size_t BestAvailable(const TMemoryView<const FGenericSurfaceFormat>& surfaceFormats) NOEXCEPT;

    friend bool operator ==(const FGenericSurfaceFormat& lhs, const FGenericSurfaceFormat& rhs) NOEXCEPT {
        return (lhs.Format == rhs.Format && lhs.ColorSpace == rhs.ColorSpace);
    }
    friend bool operator !=(const FGenericSurfaceFormat& lhs, const FGenericSurfaceFormat& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
