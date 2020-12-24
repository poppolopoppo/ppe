#include "stdafx.h"

#include "HAL/Vulkan/VulkanRHISurfaceFormat.h"

#ifdef RHI_VULKAN

#include "Allocator/Alloca.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static CONSTEXPR EGenericColorSpace VkToGeneric_(EVulkanColorSpace vk) {
    switch (vk) {
    case EVulkanColorSpace::PASS_THROUGH: return EGenericColorSpace::PASS_THROUGH;
    case EVulkanColorSpace::SRGB_NONLINEAR: return EGenericColorSpace::SRGB_NONLINEAR;
    case EVulkanColorSpace::DISPLAY_P3_NONLINEAR: return EGenericColorSpace::DISPLAY_P3_NONLINEAR;
    case EVulkanColorSpace::EXTENDED_SRGB_LINEAR: return EGenericColorSpace::EXTENDED_SRGB_LINEAR;
    case EVulkanColorSpace::DISPLAY_P3_LINEAR: return EGenericColorSpace::DISPLAY_P3_LINEAR;
    case EVulkanColorSpace::DCI_P3_NONLINEAR: return EGenericColorSpace::DCI_P3_NONLINEAR;
    case EVulkanColorSpace::BT709_LINEAR: return EGenericColorSpace::BT709_LINEAR;
    case EVulkanColorSpace::BT709_NONLINEAR: return EGenericColorSpace::BT709_NONLINEAR;
    case EVulkanColorSpace::BT2020_LINEAR: return EGenericColorSpace::BT2020_LINEAR;
    case EVulkanColorSpace::HDR10_ST2084: return EGenericColorSpace::HDR10_ST2084;
    case EVulkanColorSpace::DOLBYVISION: return EGenericColorSpace::DOLBYVISION;
    case EVulkanColorSpace::HDR10_HLG: return EGenericColorSpace::HDR10_HLG;
    case EVulkanColorSpace::ADOBERGB_LINEAR: return EGenericColorSpace::ADOBERGB_LINEAR;
    case EVulkanColorSpace::ADOBERGB_NONLINEAR: return EGenericColorSpace::ADOBERGB_NONLINEAR;
    case EVulkanColorSpace::EXTENDED_SRGB_NONLINEAR: return EGenericColorSpace::EXTENDED_SRGB_NONLINEAR;
    case EVulkanColorSpace::DISPLAY_NATIVE_AMD: return EGenericColorSpace::DISPLAY_NATIVE_AMD;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
static CONSTEXPR EGenericPixelFormat VkToGeneric_(EVulkanPixelFormat vk) {
    switch (vk) {
    case EVulkanPixelFormat::R4G4_UNORM: return EGenericPixelFormat::R4G4_UNORM;
    case EVulkanPixelFormat::R4G4B4A4_UNORM: return EGenericPixelFormat::R4G4B4A4_UNORM;
    case EVulkanPixelFormat::B4G4R4A4_UNORM: return EGenericPixelFormat::B4G4R4A4_UNORM;
    case EVulkanPixelFormat::R5G6B5_UNORM: return EGenericPixelFormat::R5G6B5_UNORM;
    case EVulkanPixelFormat::B5G6R5_UNORM: return EGenericPixelFormat::B5G6R5_UNORM;
    case EVulkanPixelFormat::R5G5B5A1_UNORM: return EGenericPixelFormat::R5G5B5A1_UNORM;
    case EVulkanPixelFormat::B5G5R5A1_UNORM: return EGenericPixelFormat::B5G5R5A1_UNORM;
    case EVulkanPixelFormat::A1R5G5B5_UNORM: return EGenericPixelFormat::A1R5G5B5_UNORM;
    case EVulkanPixelFormat::R8_UNORM: return EGenericPixelFormat::R8_UNORM;
    case EVulkanPixelFormat::R8_SNORM: return EGenericPixelFormat::R8_SNORM;
    case EVulkanPixelFormat::R8_UINT: return EGenericPixelFormat::R8_UINT;
    case EVulkanPixelFormat::R8_SINT: return EGenericPixelFormat::R8_SINT;
    case EVulkanPixelFormat::R8_SRGB: return EGenericPixelFormat::R8_SRGB;
    case EVulkanPixelFormat::R8G8_UNORM: return EGenericPixelFormat::R8G8_UNORM;
    case EVulkanPixelFormat::R8G8_SNORM: return EGenericPixelFormat::R8G8_SNORM;
    case EVulkanPixelFormat::R8G8_USCALED: return EGenericPixelFormat::R8G8_USCALED;
    case EVulkanPixelFormat::R8G8_SSCALED: return EGenericPixelFormat::R8G8_SSCALED;
    case EVulkanPixelFormat::R8G8_UINT: return EGenericPixelFormat::R8G8_UINT;
    case EVulkanPixelFormat::R8G8_SINT: return EGenericPixelFormat::R8G8_SINT;
    case EVulkanPixelFormat::R8G8_SRGB: return EGenericPixelFormat::R8G8_SRGB;
    case EVulkanPixelFormat::R8G8B8A8_UNORM: return EGenericPixelFormat::R8G8B8A8_UNORM;
    case EVulkanPixelFormat::R8G8B8A8_SNORM: return EGenericPixelFormat::R8G8B8A8_SNORM;
    case EVulkanPixelFormat::R8G8B8A8_USCALED: return EGenericPixelFormat::R8G8B8A8_USCALED;
    case EVulkanPixelFormat::R8G8B8A8_SSCALED: return EGenericPixelFormat::R8G8B8A8_SSCALED;
    case EVulkanPixelFormat::R8G8B8A8_UINT: return EGenericPixelFormat::R8G8B8A8_UINT;
    case EVulkanPixelFormat::R8G8B8A8_SINT: return EGenericPixelFormat::R8G8B8A8_SINT;
    case EVulkanPixelFormat::R8G8B8A8_SRGB: return EGenericPixelFormat::R8G8B8A8_SRGB;
    case EVulkanPixelFormat::B8G8R8A8_UNORM: return EGenericPixelFormat::B8G8R8A8_UNORM;
    case EVulkanPixelFormat::B8G8R8A8_SNORM: return EGenericPixelFormat::B8G8R8A8_SNORM;
    case EVulkanPixelFormat::B8G8R8A8_USCALED: return EGenericPixelFormat::B8G8R8A8_USCALED;
    case EVulkanPixelFormat::B8G8R8A8_SSCALED: return EGenericPixelFormat::B8G8R8A8_SSCALED;
    case EVulkanPixelFormat::B8G8R8A8_UINT: return EGenericPixelFormat::B8G8R8A8_UINT;
    case EVulkanPixelFormat::B8G8R8A8_SINT: return EGenericPixelFormat::B8G8R8A8_SINT;
    case EVulkanPixelFormat::B8G8R8A8_SRGB: return EGenericPixelFormat::B8G8R8A8_SRGB;
    case EVulkanPixelFormat::A2R10G10B10_UNORM: return EGenericPixelFormat::A2R10G10B10_UNORM;
    case EVulkanPixelFormat::A2R10G10B10_SNORM: return EGenericPixelFormat::A2R10G10B10_SNORM;
    case EVulkanPixelFormat::A2R10G10B10_USCALED: return EGenericPixelFormat::A2R10G10B10_USCALED;
    case EVulkanPixelFormat::A2R10G10B10_SSCALED: return EGenericPixelFormat::A2R10G10B10_SSCALED;
    case EVulkanPixelFormat::A2R10G10B10_UINT: return EGenericPixelFormat::A2R10G10B10_UINT;
    case EVulkanPixelFormat::A2R10G10B10_SINT: return EGenericPixelFormat::A2R10G10B10_SINT;
    case EVulkanPixelFormat::A2B10G10R10_UNORM: return EGenericPixelFormat::A2B10G10R10_UNORM;
    case EVulkanPixelFormat::A2B10G10R10_SNORM: return EGenericPixelFormat::A2B10G10R10_SNORM;
    case EVulkanPixelFormat::A2B10G10R10_USCALED: return EGenericPixelFormat::A2B10G10R10_USCALED;
    case EVulkanPixelFormat::A2B10G10R10_SSCALED: return EGenericPixelFormat::A2B10G10R10_SSCALED;
    case EVulkanPixelFormat::A2B10G10R10_UINT: return EGenericPixelFormat::A2B10G10R10_UINT;
    case EVulkanPixelFormat::A2B10G10R10_SINT: return EGenericPixelFormat::A2B10G10R10_SINT;
    case EVulkanPixelFormat::R16_UNORM: return EGenericPixelFormat::R16_UNORM;
    case EVulkanPixelFormat::R16_SNORM: return EGenericPixelFormat::R16_SNORM;
    case EVulkanPixelFormat::R16_USCALED: return EGenericPixelFormat::R16_USCALED;
    case EVulkanPixelFormat::R16_SSCALED: return EGenericPixelFormat::R16_SSCALED;
    case EVulkanPixelFormat::R16_UINT: return EGenericPixelFormat::R16_UINT;
    case EVulkanPixelFormat::R16_SINT: return EGenericPixelFormat::R16_SINT;
    case EVulkanPixelFormat::R16_SFLOAT: return EGenericPixelFormat::R16_SFLOAT;
    case EVulkanPixelFormat::R16G16_UNORM: return EGenericPixelFormat::R16G16_UNORM;
    case EVulkanPixelFormat::R16G16_SNORM: return EGenericPixelFormat::R16G16_SNORM;
    case EVulkanPixelFormat::R16G16_USCALED: return EGenericPixelFormat::R16G16_USCALED;
    case EVulkanPixelFormat::R16G16_SSCALED: return EGenericPixelFormat::R16G16_SSCALED;
    case EVulkanPixelFormat::R16G16_UINT: return EGenericPixelFormat::R16G16_UINT;
    case EVulkanPixelFormat::R16G16_SINT: return EGenericPixelFormat::R16G16_SINT;
    case EVulkanPixelFormat::R16G16_SFLOAT: return EGenericPixelFormat::R16G16_SFLOAT;
    case EVulkanPixelFormat::R16G16B16A16_UNORM: return EGenericPixelFormat::R16G16B16A16_UNORM;
    case EVulkanPixelFormat::R16G16B16A16_SNORM: return EGenericPixelFormat::R16G16B16A16_SNORM;
    case EVulkanPixelFormat::R16G16B16A16_USCALED: return EGenericPixelFormat::R16G16B16A16_USCALED;
    case EVulkanPixelFormat::R16G16B16A16_SSCALED: return EGenericPixelFormat::R16G16B16A16_SSCALED;
    case EVulkanPixelFormat::R16G16B16A16_UINT: return EGenericPixelFormat::R16G16B16A16_UINT;
    case EVulkanPixelFormat::R16G16B16A16_SINT: return EGenericPixelFormat::R16G16B16A16_SINT;
    case EVulkanPixelFormat::R16G16B16A16_SFLOAT: return EGenericPixelFormat::R16G16B16A16_SFLOAT;
    case EVulkanPixelFormat::R32_UINT: return EGenericPixelFormat::R32_UINT;
    case EVulkanPixelFormat::R32_SINT: return EGenericPixelFormat::R32_SINT;
    case EVulkanPixelFormat::R32_SFLOAT: return EGenericPixelFormat::R32_SFLOAT;
    case EVulkanPixelFormat::R32G32_UINT: return EGenericPixelFormat::R32G32_UINT;
    case EVulkanPixelFormat::R32G32_SINT: return EGenericPixelFormat::R32G32_SINT;
    case EVulkanPixelFormat::R32G32_SFLOAT: return EGenericPixelFormat::R32G32_SFLOAT;
    case EVulkanPixelFormat::R32G32B32A32_UINT: return EGenericPixelFormat::R32G32B32A32_UINT;
    case EVulkanPixelFormat::R32G32B32A32_SINT: return EGenericPixelFormat::R32G32B32A32_SINT;
    case EVulkanPixelFormat::R32G32B32A32_SFLOAT: return EGenericPixelFormat::R32G32B32A32_SFLOAT;
    case EVulkanPixelFormat::R64_UINT: return EGenericPixelFormat::R64_UINT;
    case EVulkanPixelFormat::R64_SINT: return EGenericPixelFormat::R64_SINT;
    case EVulkanPixelFormat::R64_SFLOAT: return EGenericPixelFormat::R64_SFLOAT;
    case EVulkanPixelFormat::R64G64_UINT: return EGenericPixelFormat::R64G64_UINT;
    case EVulkanPixelFormat::R64G64_SINT: return EGenericPixelFormat::R64G64_SINT;
    case EVulkanPixelFormat::R64G64_SFLOAT: return EGenericPixelFormat::R64G64_SFLOAT;
    case EVulkanPixelFormat::R64G64B64A64_UINT: return EGenericPixelFormat::R64G64B64A64_UINT;
    case EVulkanPixelFormat::R64G64B64A64_SINT: return EGenericPixelFormat::R64G64B64A64_SINT;
    case EVulkanPixelFormat::R64G64B64A64_SFLOAT: return EGenericPixelFormat::R64G64B64A64_SFLOAT;
    case EVulkanPixelFormat::B10G11R11_UFLOAT: return EGenericPixelFormat::B10G11R11_UFLOAT;
    case EVulkanPixelFormat::E5B9G9R9_UFLOAT: return EGenericPixelFormat::E5B9G9R9_UFLOAT;
    case EVulkanPixelFormat::D16_UNORM: return EGenericPixelFormat::D16_UNORM;
    case EVulkanPixelFormat::D32_SFLOAT: return EGenericPixelFormat::D32_SFLOAT;
    case EVulkanPixelFormat::D16_UNORM_S8_UINT: return EGenericPixelFormat::D16_UNORM_S8_UINT;
    case EVulkanPixelFormat::D24_UNORM_S8_UINT: return EGenericPixelFormat::D24_UNORM_S8_UINT;
    case EVulkanPixelFormat::BC1_RGB_UNORM: return EGenericPixelFormat::BC1_RGB_UNORM;
    case EVulkanPixelFormat::BC1_RGB_SRGB: return EGenericPixelFormat::BC1_RGB_SRGB;
    case EVulkanPixelFormat::BC1_RGBA_UNORM: return EGenericPixelFormat::BC1_RGBA_UNORM;
    case EVulkanPixelFormat::BC1_RGBA_SRGB: return EGenericPixelFormat::BC1_RGBA_SRGB;
    case EVulkanPixelFormat::BC2_UNORM: return EGenericPixelFormat::BC2_UNORM;
    case EVulkanPixelFormat::BC2_SRGB: return EGenericPixelFormat::BC2_SRGB;
    case EVulkanPixelFormat::BC3_UNORM: return EGenericPixelFormat::BC3_UNORM;
    case EVulkanPixelFormat::BC3_SRGB: return EGenericPixelFormat::BC3_SRGB;
    case EVulkanPixelFormat::BC4_UNORM: return EGenericPixelFormat::BC4_UNORM;
    case EVulkanPixelFormat::BC4_SNORM: return EGenericPixelFormat::BC4_SNORM;
    case EVulkanPixelFormat::BC5_UNORM: return EGenericPixelFormat::BC5_UNORM;
    case EVulkanPixelFormat::BC5_SNORM: return EGenericPixelFormat::BC5_SNORM;
    case EVulkanPixelFormat::BC6H_UFLOAT: return EGenericPixelFormat::BC6H_UFLOAT;
    case EVulkanPixelFormat::BC6H_SFLOAT: return EGenericPixelFormat::BC6H_SFLOAT;
    case EVulkanPixelFormat::BC7_UNORM: return EGenericPixelFormat::BC7_UNORM;
    case EVulkanPixelFormat::BC7_SRGB: return EGenericPixelFormat::BC7_SRGB;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanPixelInfo FVulkanPixelInfo::FromFormat(EVulkanPixelFormat format) NOEXCEPT {
    return FVulkanPixelInfo{ FGenericPixelInfo::FromFormat(VkToGeneric_(format)) };
}
//----------------------------------------------------------------------------
size_t FVulkanSurfaceFormat::BestAvailable(const TMemoryView<const FVulkanSurfaceFormat>& surfaceFormats) NOEXCEPT {
    STACKLOCAL_POD_ARRAY(FGenericSurfaceFormat, genericFormats, surfaceFormats.size());
    forrange(i, 0, surfaceFormats.size()) {
        genericFormats[i] = FGenericSurfaceFormat{
            VkToGeneric_(surfaceFormats[i].Format),
            VkToGeneric_(surfaceFormats[i].ColorSpace)
        };
    }
    return FGenericSurfaceFormat::BestAvailable(genericFormats);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
