#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHISurfaceFormat.h"
#include "HAL/Vulkan/VulkanRHIFormat.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_RHI_API FVulkanPixelInfo : FGenericPixelInfo {
    using FGenericPixelInfo::EChannel;
    using FGenericPixelInfo::ENumericFormat;

    using FGenericPixelInfo::Pitch;
    using FGenericPixelInfo::BlockWindow;
    using FGenericPixelInfo::BlockNumBits;
    using FGenericPixelInfo::BlockNumPixels;
    using FGenericPixelInfo::BitsPerPixel;

    using FGenericPixelInfo::ImageSize;
    using FGenericPixelInfo::ImageSizeInBytes;
    using FGenericPixelInfo::Texture1DSizeInBytes;
    using FGenericPixelInfo::Texture2DSizeInBytes;
    using FGenericPixelInfo::Texture3DSizeInBytes;
    using FGenericPixelInfo::TextureCubeSizeInBytes;

    static FVulkanPixelInfo FromFormat(EVulkanPixelFormat format) NOEXCEPT;
};
//----------------------------------------------------------------------------
struct PPE_RHI_API FVulkanSurfaceFormat {
    using EPixelFormat = EVulkanPixelFormat;
    using EColorSpace = EVulkanColorSpace;
    using FPixelInfo = FVulkanPixelInfo;

    EPixelFormat Format;
    EColorSpace ColorSpace;

    FVulkanPixelInfo PixelInfo() const { return FVulkanPixelInfo::FromFormat(Format); }

    static size_t BestAvailable(const TMemoryView<const FVulkanSurfaceFormat>& surfaceFormats) NOEXCEPT;

    static FVulkanSurfaceFormat B8G8R8A8_sRGB_NonLinear() NOEXCEPT {
        return FVulkanSurfaceFormat{
            EPixelFormat::B8G8R8A8_SRGB,
            EColorSpace::SRGB_NONLINEAR
        };
    }
	static FVulkanSurfaceFormat B10G11R11_Linear() NOEXCEPT {
		return FVulkanSurfaceFormat{
			EPixelFormat::B10G11R11_UFLOAT,
			EColorSpace::EXTENDED_SRGB_LINEAR
		};
	}

    static FVulkanSurfaceFormat Default_NonHDR() NOEXCEPT {
        return B8G8R8A8_sRGB_NonLinear();
    }
	static FVulkanSurfaceFormat Default_Linear() NOEXCEPT {
		return B10G11R11_Linear();
	}

    friend bool operator ==(const FVulkanSurfaceFormat& lhs, const FVulkanSurfaceFormat& rhs) NOEXCEPT {
        return (lhs.Format == rhs.Format && lhs.ColorSpace == rhs.ColorSpace);
    }
    friend bool operator !=(const FVulkanSurfaceFormat& lhs, const FVulkanSurfaceFormat& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
