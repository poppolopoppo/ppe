#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#include "HAL/Generic/GenericRHIFormat.h"

#include "HAL/PlatformMaths.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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

    static FGenericSurfaceFormat B8G8R8A8_sRGB_NonLinear() NOEXCEPT {
        return FGenericSurfaceFormat{
            EGenericPixelFormat::B8G8R8A8_SRGB,
            EGenericColorSpace::SRGB_NONLINEAR
        };
    }
	static FGenericSurfaceFormat B10G11R11_Linear() NOEXCEPT {
		return FGenericSurfaceFormat{
			EGenericPixelFormat::B10G11R11_UFLOAT,
			EGenericColorSpace::EXTENDED_SRGB_LINEAR
		};
	}

    static FGenericSurfaceFormat Default_NonHDR() NOEXCEPT {
        return B8G8R8A8_sRGB_NonLinear();
    }
	static FGenericSurfaceFormat Default_Linear() NOEXCEPT {
		return B10G11R11_Linear();
	}

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
