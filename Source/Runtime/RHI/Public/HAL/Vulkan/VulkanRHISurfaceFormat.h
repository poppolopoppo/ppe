#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHISurfaceFormat.h"

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

    using FGenericPixelInfo::FromFormat;
};
//----------------------------------------------------------------------------
struct PPE_RHI_API FVulkanSurfaceFormat : FGenericSurfaceFormat {
    using EPixelFormat = EVulkanPixelFormat;
    using EColorSpace = EVulkanColorSpace;
    using FPixelInfo = FVulkanPixelInfo;

    using FGenericSurfaceFormat::Format;
    using FGenericSurfaceFormat::ColorSpace;

    FVulkanPixelInfo PixelInfo() const NOEXCEPT {
        return FVulkanPixelInfo{ FGenericSurfaceFormat::PixelInfo() };
    }

    static size_t BestAvailable(const TMemoryView<const FVulkanSurfaceFormat>& surfaceFormats) NOEXCEPT {
        return FGenericSurfaceFormat::BestAvailable(surfaceFormats.Cast<const FVulkanSurfaceFormat>());
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
