#pragma once

#include "HAL/Vulkan/VulkanInterop.h"

#include "HAL/Vulkan/VulkanRHIDevice.h"
#include "HAL/Vulkan/VulkanRHISurfaceFormat.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline CONSTEXPR VkColorSpaceKHR FVulkanInterop::ColorSpace_to_VkColorSpaceKHR(EVulkanColorSpace colorSpace) NOEXCEPT {
    switch (colorSpace) {
    case EVulkanColorSpace::PASS_THROUGH: return VK_COLOR_SPACE_PASS_THROUGH_EXT;
    case EVulkanColorSpace::SRGB_NONLINEAR: return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    case EVulkanColorSpace::DISPLAY_P3_NONLINEAR: return VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT;
    case EVulkanColorSpace::EXTENDED_SRGB_LINEAR: return VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT;
    case EVulkanColorSpace::DISPLAY_P3_LINEAR: return VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT;
    case EVulkanColorSpace::DCI_P3_NONLINEAR: return VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT;
    case EVulkanColorSpace::BT709_LINEAR: return VK_COLOR_SPACE_BT709_LINEAR_EXT;
    case EVulkanColorSpace::BT709_NONLINEAR: return VK_COLOR_SPACE_BT709_NONLINEAR_EXT;
    case EVulkanColorSpace::BT2020_LINEAR: return VK_COLOR_SPACE_BT2020_LINEAR_EXT;
    case EVulkanColorSpace::HDR10_ST2084: return VK_COLOR_SPACE_HDR10_ST2084_EXT;
    case EVulkanColorSpace::DOLBYVISION: return VK_COLOR_SPACE_DOLBYVISION_EXT;
    case EVulkanColorSpace::HDR10_HLG: return VK_COLOR_SPACE_HDR10_HLG_EXT;
    case EVulkanColorSpace::ADOBERGB_LINEAR: return VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT;
    case EVulkanColorSpace::ADOBERGB_NONLINEAR: return VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT;
    case EVulkanColorSpace::EXTENDED_SRGB_NONLINEAR: return VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT;
    case EVulkanColorSpace::DISPLAY_NATIVE_AMD: return VK_COLOR_SPACE_DISPLAY_NATIVE_AMD;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
inline CONSTEXPR EVulkanColorSpace FVulkanInterop::VkColorSpaceKHR_to_ColorSpace(VkColorSpaceKHR vkColorSpace) NOEXCEPT {
    switch (vkColorSpace) {
    case VK_COLOR_SPACE_PASS_THROUGH_EXT: return EVulkanColorSpace::PASS_THROUGH;
    case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: return EVulkanColorSpace::SRGB_NONLINEAR;
    case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT: return EVulkanColorSpace::DISPLAY_P3_NONLINEAR;
    case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT: return EVulkanColorSpace::EXTENDED_SRGB_LINEAR;
    case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT: return EVulkanColorSpace::DISPLAY_P3_LINEAR;
    case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT: return EVulkanColorSpace::DCI_P3_NONLINEAR;
    case VK_COLOR_SPACE_BT709_LINEAR_EXT: return EVulkanColorSpace::BT709_LINEAR;
    case VK_COLOR_SPACE_BT709_NONLINEAR_EXT: return EVulkanColorSpace::BT709_NONLINEAR;
    case VK_COLOR_SPACE_BT2020_LINEAR_EXT: return EVulkanColorSpace::BT2020_LINEAR;
    case VK_COLOR_SPACE_HDR10_ST2084_EXT: return EVulkanColorSpace::HDR10_ST2084;
    case VK_COLOR_SPACE_DOLBYVISION_EXT: return EVulkanColorSpace::DOLBYVISION;
    case VK_COLOR_SPACE_HDR10_HLG_EXT: return EVulkanColorSpace::HDR10_HLG;
    case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT: return EVulkanColorSpace::ADOBERGB_LINEAR;
    case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT: return EVulkanColorSpace::ADOBERGB_NONLINEAR;
    case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT: return EVulkanColorSpace::EXTENDED_SRGB_NONLINEAR;
    case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD: return EVulkanColorSpace::DISPLAY_NATIVE_AMD;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
inline CONSTEXPR VkFormat FVulkanInterop::PixelFormat_to_VkFormat(EVulkanPixelFormat pixelFormat) NOEXCEPT {
    switch (pixelFormat) {
    case EVulkanPixelFormat::R4G4_UNORM: return VK_FORMAT_R4G4_UNORM_PACK8;
    case EVulkanPixelFormat::R4G4B4A4_UNORM: return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
    case EVulkanPixelFormat::B4G4R4A4_UNORM: return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
    case EVulkanPixelFormat::R5G6B5_UNORM: return VK_FORMAT_R5G6B5_UNORM_PACK16;
    case EVulkanPixelFormat::B5G6R5_UNORM: return VK_FORMAT_B5G6R5_UNORM_PACK16;
    case EVulkanPixelFormat::R5G5B5A1_UNORM: return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
    case EVulkanPixelFormat::B5G5R5A1_UNORM: return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
    case EVulkanPixelFormat::A1R5G5B5_UNORM: return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
    case EVulkanPixelFormat::R8_UNORM: return VK_FORMAT_R8_UNORM;
    case EVulkanPixelFormat::R8_SNORM: return VK_FORMAT_R8_SNORM;
    case EVulkanPixelFormat::R8_UINT: return VK_FORMAT_R8_UINT;
    case EVulkanPixelFormat::R8_SINT: return VK_FORMAT_R8_SINT;
    case EVulkanPixelFormat::R8_SRGB: return VK_FORMAT_R8_SRGB;
    case EVulkanPixelFormat::R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
    case EVulkanPixelFormat::R8G8_SNORM: return VK_FORMAT_R8G8_SNORM;
    case EVulkanPixelFormat::R8G8_USCALED: return VK_FORMAT_R8G8_USCALED;
    case EVulkanPixelFormat::R8G8_SSCALED: return VK_FORMAT_R8G8_SSCALED;
    case EVulkanPixelFormat::R8G8_UINT: return VK_FORMAT_R8G8_UINT;
    case EVulkanPixelFormat::R8G8_SINT: return VK_FORMAT_R8G8_SINT;
    case EVulkanPixelFormat::R8G8_SRGB: return VK_FORMAT_R8G8_SRGB;
    case EVulkanPixelFormat::R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
    case EVulkanPixelFormat::R8G8B8A8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;
    case EVulkanPixelFormat::R8G8B8A8_USCALED: return VK_FORMAT_R8G8B8A8_USCALED;
    case EVulkanPixelFormat::R8G8B8A8_SSCALED: return VK_FORMAT_R8G8B8A8_SSCALED;
    case EVulkanPixelFormat::R8G8B8A8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
    case EVulkanPixelFormat::R8G8B8A8_SINT: return VK_FORMAT_R8G8B8A8_SINT;
    case EVulkanPixelFormat::R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
    case EVulkanPixelFormat::B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
    case EVulkanPixelFormat::B8G8R8A8_SNORM: return VK_FORMAT_B8G8R8A8_SNORM;
    case EVulkanPixelFormat::B8G8R8A8_USCALED: return VK_FORMAT_B8G8R8A8_USCALED;
    case EVulkanPixelFormat::B8G8R8A8_SSCALED: return VK_FORMAT_B8G8R8A8_SSCALED;
    case EVulkanPixelFormat::B8G8R8A8_UINT: return VK_FORMAT_B8G8R8A8_UINT;
    case EVulkanPixelFormat::B8G8R8A8_SINT: return VK_FORMAT_B8G8R8A8_SINT;
    case EVulkanPixelFormat::B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
    case EVulkanPixelFormat::A2R10G10B10_UNORM: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    case EVulkanPixelFormat::A2R10G10B10_SNORM: return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
    case EVulkanPixelFormat::A2R10G10B10_USCALED: return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
    case EVulkanPixelFormat::A2R10G10B10_SSCALED: return VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
    case EVulkanPixelFormat::A2R10G10B10_UINT: return VK_FORMAT_A2R10G10B10_UINT_PACK32;
    case EVulkanPixelFormat::A2R10G10B10_SINT: return VK_FORMAT_A2R10G10B10_SINT_PACK32;
    case EVulkanPixelFormat::A2B10G10R10_UNORM: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    case EVulkanPixelFormat::A2B10G10R10_SNORM: return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
    case EVulkanPixelFormat::A2B10G10R10_USCALED: return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
    case EVulkanPixelFormat::A2B10G10R10_SSCALED: return VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
    case EVulkanPixelFormat::A2B10G10R10_UINT: return VK_FORMAT_A2B10G10R10_UINT_PACK32;
    case EVulkanPixelFormat::A2B10G10R10_SINT: return VK_FORMAT_A2B10G10R10_SINT_PACK32;
    case EVulkanPixelFormat::R16_UNORM: return VK_FORMAT_R16_UNORM;
    case EVulkanPixelFormat::R16_SNORM: return VK_FORMAT_R16_SNORM;
    case EVulkanPixelFormat::R16_USCALED: return VK_FORMAT_R16_USCALED;
    case EVulkanPixelFormat::R16_SSCALED: return VK_FORMAT_R16_SSCALED;
    case EVulkanPixelFormat::R16_UINT: return VK_FORMAT_R16_UINT;
    case EVulkanPixelFormat::R16_SINT: return VK_FORMAT_R16_SINT;
    case EVulkanPixelFormat::R16_SFLOAT: return VK_FORMAT_R16_SFLOAT;
    case EVulkanPixelFormat::R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
    case EVulkanPixelFormat::R16G16_SNORM: return VK_FORMAT_R16G16_SNORM;
    case EVulkanPixelFormat::R16G16_USCALED: return VK_FORMAT_R16G16_USCALED;
    case EVulkanPixelFormat::R16G16_SSCALED: return VK_FORMAT_R16G16_SSCALED;
    case EVulkanPixelFormat::R16G16_UINT: return VK_FORMAT_R16G16_UINT;
    case EVulkanPixelFormat::R16G16_SINT: return VK_FORMAT_R16G16_SINT;
    case EVulkanPixelFormat::R16G16_SFLOAT: return VK_FORMAT_R16G16_SFLOAT;
    case EVulkanPixelFormat::R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
    case EVulkanPixelFormat::R16G16B16A16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;
    case EVulkanPixelFormat::R16G16B16A16_USCALED: return VK_FORMAT_R16G16B16A16_USCALED;
    case EVulkanPixelFormat::R16G16B16A16_SSCALED: return VK_FORMAT_R16G16B16A16_SSCALED;
    case EVulkanPixelFormat::R16G16B16A16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
    case EVulkanPixelFormat::R16G16B16A16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
    case EVulkanPixelFormat::R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
    case EVulkanPixelFormat::R32_UINT: return VK_FORMAT_R32_UINT;
    case EVulkanPixelFormat::R32_SINT: return VK_FORMAT_R32_SINT;
    case EVulkanPixelFormat::R32_SFLOAT: return VK_FORMAT_R32_SFLOAT;
    case EVulkanPixelFormat::R32G32_UINT: return VK_FORMAT_R32G32_UINT;
    case EVulkanPixelFormat::R32G32_SINT: return VK_FORMAT_R32G32_SINT;
    case EVulkanPixelFormat::R32G32_SFLOAT: return VK_FORMAT_R32G32_SFLOAT;
    case EVulkanPixelFormat::R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
    case EVulkanPixelFormat::R32G32B32A32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
    case EVulkanPixelFormat::R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case EVulkanPixelFormat::R64_UINT: return VK_FORMAT_R64_UINT;
    case EVulkanPixelFormat::R64_SINT: return VK_FORMAT_R64_SINT;
    case EVulkanPixelFormat::R64_SFLOAT: return VK_FORMAT_R64_SFLOAT;
    case EVulkanPixelFormat::R64G64_UINT: return VK_FORMAT_R64G64_UINT;
    case EVulkanPixelFormat::R64G64_SINT: return VK_FORMAT_R64G64_SINT;
    case EVulkanPixelFormat::R64G64_SFLOAT: return VK_FORMAT_R64G64_SFLOAT;
    case EVulkanPixelFormat::R64G64B64A64_UINT: return VK_FORMAT_R64G64B64A64_UINT;
    case EVulkanPixelFormat::R64G64B64A64_SINT: return VK_FORMAT_R64G64B64A64_SINT;
    case EVulkanPixelFormat::R64G64B64A64_SFLOAT: return VK_FORMAT_R64G64B64A64_SFLOAT;
    case EVulkanPixelFormat::B10G11R11_UFLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case EVulkanPixelFormat::E5B9G9R9_UFLOAT: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
    case EVulkanPixelFormat::D16_UNORM: return VK_FORMAT_D16_UNORM;
    case EVulkanPixelFormat::D32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
    case EVulkanPixelFormat::D16_UNORM_S8_UINT: return VK_FORMAT_D16_UNORM_S8_UINT;
    case EVulkanPixelFormat::D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
    case EVulkanPixelFormat::BC1_RGB_UNORM: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case EVulkanPixelFormat::BC1_RGB_SRGB: return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
    case EVulkanPixelFormat::BC1_RGBA_UNORM: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case EVulkanPixelFormat::BC1_RGBA_SRGB: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case EVulkanPixelFormat::BC2_UNORM: return VK_FORMAT_BC2_UNORM_BLOCK;
    case EVulkanPixelFormat::BC2_SRGB: return VK_FORMAT_BC2_SRGB_BLOCK;
    case EVulkanPixelFormat::BC3_UNORM: return VK_FORMAT_BC3_UNORM_BLOCK;
    case EVulkanPixelFormat::BC3_SRGB: return VK_FORMAT_BC3_SRGB_BLOCK;
    case EVulkanPixelFormat::BC4_UNORM: return VK_FORMAT_BC4_UNORM_BLOCK;
    case EVulkanPixelFormat::BC4_SNORM: return VK_FORMAT_BC4_SNORM_BLOCK;
    case EVulkanPixelFormat::BC5_UNORM: return VK_FORMAT_BC5_UNORM_BLOCK;
    case EVulkanPixelFormat::BC5_SNORM: return VK_FORMAT_BC5_SNORM_BLOCK;
    case EVulkanPixelFormat::BC6H_UFLOAT: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case EVulkanPixelFormat::BC6H_SFLOAT: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case EVulkanPixelFormat::BC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
    case EVulkanPixelFormat::BC7_SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
inline CONSTEXPR EVulkanPixelFormat FVulkanInterop::VkFormat_to_PixelFormat(VkFormat vkFormat) NOEXCEPT {
    switch (vkFormat) {
    case VK_FORMAT_R4G4_UNORM_PACK8: return EVulkanPixelFormat::R4G4_UNORM;
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return EVulkanPixelFormat::R4G4B4A4_UNORM;
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16: return EVulkanPixelFormat::B4G4R4A4_UNORM;
    case VK_FORMAT_R5G6B5_UNORM_PACK16: return EVulkanPixelFormat::R5G6B5_UNORM;
    case VK_FORMAT_B5G6R5_UNORM_PACK16: return EVulkanPixelFormat::B5G6R5_UNORM;
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return EVulkanPixelFormat::R5G5B5A1_UNORM;
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16: return EVulkanPixelFormat::B5G5R5A1_UNORM;
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16: return EVulkanPixelFormat::A1R5G5B5_UNORM;
    case VK_FORMAT_R8_UNORM: return EVulkanPixelFormat::R8_UNORM;
    case VK_FORMAT_R8_SNORM: return EVulkanPixelFormat::R8_SNORM;
    case VK_FORMAT_R8_UINT: return EVulkanPixelFormat::R8_UINT;
    case VK_FORMAT_R8_SINT: return EVulkanPixelFormat::R8_SINT;
    case VK_FORMAT_R8_SRGB: return EVulkanPixelFormat::R8_SRGB;
    case VK_FORMAT_R8G8_UNORM: return EVulkanPixelFormat::R8G8_UNORM;
    case VK_FORMAT_R8G8_SNORM: return EVulkanPixelFormat::R8G8_SNORM;
    case VK_FORMAT_R8G8_USCALED: return EVulkanPixelFormat::R8G8_USCALED;
    case VK_FORMAT_R8G8_SSCALED: return EVulkanPixelFormat::R8G8_SSCALED;
    case VK_FORMAT_R8G8_UINT: return EVulkanPixelFormat::R8G8_UINT;
    case VK_FORMAT_R8G8_SINT: return EVulkanPixelFormat::R8G8_SINT;
    case VK_FORMAT_R8G8_SRGB: return EVulkanPixelFormat::R8G8_SRGB;
    case VK_FORMAT_R8G8B8A8_UNORM: return EVulkanPixelFormat::R8G8B8A8_UNORM;
    case VK_FORMAT_R8G8B8A8_SNORM: return EVulkanPixelFormat::R8G8B8A8_SNORM;
    case VK_FORMAT_R8G8B8A8_USCALED: return EVulkanPixelFormat::R8G8B8A8_USCALED;
    case VK_FORMAT_R8G8B8A8_SSCALED: return EVulkanPixelFormat::R8G8B8A8_SSCALED;
    case VK_FORMAT_R8G8B8A8_UINT: return EVulkanPixelFormat::R8G8B8A8_UINT;
    case VK_FORMAT_R8G8B8A8_SINT: return EVulkanPixelFormat::R8G8B8A8_SINT;
    case VK_FORMAT_R8G8B8A8_SRGB: return EVulkanPixelFormat::R8G8B8A8_SRGB;
    case VK_FORMAT_B8G8R8A8_UNORM: return EVulkanPixelFormat::B8G8R8A8_UNORM;
    case VK_FORMAT_B8G8R8A8_SNORM: return EVulkanPixelFormat::B8G8R8A8_SNORM;
    case VK_FORMAT_B8G8R8A8_USCALED: return EVulkanPixelFormat::B8G8R8A8_USCALED;
    case VK_FORMAT_B8G8R8A8_SSCALED: return EVulkanPixelFormat::B8G8R8A8_SSCALED;
    case VK_FORMAT_B8G8R8A8_UINT: return EVulkanPixelFormat::B8G8R8A8_UINT;
    case VK_FORMAT_B8G8R8A8_SINT: return EVulkanPixelFormat::B8G8R8A8_SINT;
    case VK_FORMAT_B8G8R8A8_SRGB: return EVulkanPixelFormat::B8G8R8A8_SRGB;
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return EVulkanPixelFormat::A2R10G10B10_UNORM;
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return EVulkanPixelFormat::A2R10G10B10_SNORM;
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32: return EVulkanPixelFormat::A2R10G10B10_USCALED;
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: return EVulkanPixelFormat::A2R10G10B10_SSCALED;
    case VK_FORMAT_A2R10G10B10_UINT_PACK32: return EVulkanPixelFormat::A2R10G10B10_UINT;
    case VK_FORMAT_A2R10G10B10_SINT_PACK32: return EVulkanPixelFormat::A2R10G10B10_SINT;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return EVulkanPixelFormat::A2B10G10R10_UNORM;
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32: return EVulkanPixelFormat::A2B10G10R10_SNORM;
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32: return EVulkanPixelFormat::A2B10G10R10_USCALED;
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: return EVulkanPixelFormat::A2B10G10R10_SSCALED;
    case VK_FORMAT_A2B10G10R10_UINT_PACK32: return EVulkanPixelFormat::A2B10G10R10_UINT;
    case VK_FORMAT_A2B10G10R10_SINT_PACK32: return EVulkanPixelFormat::A2B10G10R10_SINT;
    case VK_FORMAT_R16_UNORM: return EVulkanPixelFormat::R16_UNORM;
    case VK_FORMAT_R16_SNORM: return EVulkanPixelFormat::R16_SNORM;
    case VK_FORMAT_R16_USCALED: return EVulkanPixelFormat::R16_USCALED;
    case VK_FORMAT_R16_SSCALED: return EVulkanPixelFormat::R16_SSCALED;
    case VK_FORMAT_R16_UINT: return EVulkanPixelFormat::R16_UINT;
    case VK_FORMAT_R16_SINT: return EVulkanPixelFormat::R16_SINT;
    case VK_FORMAT_R16_SFLOAT: return EVulkanPixelFormat::R16_SFLOAT;
    case VK_FORMAT_R16G16_UNORM: return EVulkanPixelFormat::R16G16_UNORM;
    case VK_FORMAT_R16G16_SNORM: return EVulkanPixelFormat::R16G16_SNORM;
    case VK_FORMAT_R16G16_USCALED: return EVulkanPixelFormat::R16G16_USCALED;
    case VK_FORMAT_R16G16_SSCALED: return EVulkanPixelFormat::R16G16_SSCALED;
    case VK_FORMAT_R16G16_UINT: return EVulkanPixelFormat::R16G16_UINT;
    case VK_FORMAT_R16G16_SINT: return EVulkanPixelFormat::R16G16_SINT;
    case VK_FORMAT_R16G16_SFLOAT: return EVulkanPixelFormat::R16G16_SFLOAT;
    case VK_FORMAT_R16G16B16A16_UNORM: return EVulkanPixelFormat::R16G16B16A16_UNORM;
    case VK_FORMAT_R16G16B16A16_SNORM: return EVulkanPixelFormat::R16G16B16A16_SNORM;
    case VK_FORMAT_R16G16B16A16_USCALED: return EVulkanPixelFormat::R16G16B16A16_USCALED;
    case VK_FORMAT_R16G16B16A16_SSCALED: return EVulkanPixelFormat::R16G16B16A16_SSCALED;
    case VK_FORMAT_R16G16B16A16_UINT: return EVulkanPixelFormat::R16G16B16A16_UINT;
    case VK_FORMAT_R16G16B16A16_SINT: return EVulkanPixelFormat::R16G16B16A16_SINT;
    case VK_FORMAT_R16G16B16A16_SFLOAT: return EVulkanPixelFormat::R16G16B16A16_SFLOAT;
    case VK_FORMAT_R32_UINT: return EVulkanPixelFormat::R32_UINT;
    case VK_FORMAT_R32_SINT: return EVulkanPixelFormat::R32_SINT;
    case VK_FORMAT_R32_SFLOAT: return EVulkanPixelFormat::R32_SFLOAT;
    case VK_FORMAT_R32G32_UINT: return EVulkanPixelFormat::R32G32_UINT;
    case VK_FORMAT_R32G32_SINT: return EVulkanPixelFormat::R32G32_SINT;
    case VK_FORMAT_R32G32_SFLOAT: return EVulkanPixelFormat::R32G32_SFLOAT;
    case VK_FORMAT_R32G32B32A32_UINT: return EVulkanPixelFormat::R32G32B32A32_UINT;
    case VK_FORMAT_R32G32B32A32_SINT: return EVulkanPixelFormat::R32G32B32A32_SINT;
    case VK_FORMAT_R32G32B32A32_SFLOAT: return EVulkanPixelFormat::R32G32B32A32_SFLOAT;
    case VK_FORMAT_R64_UINT: return EVulkanPixelFormat::R64_UINT;
    case VK_FORMAT_R64_SINT: return EVulkanPixelFormat::R64_SINT;
    case VK_FORMAT_R64_SFLOAT: return EVulkanPixelFormat::R64_SFLOAT;
    case VK_FORMAT_R64G64_UINT: return EVulkanPixelFormat::R64G64_UINT;
    case VK_FORMAT_R64G64_SINT: return EVulkanPixelFormat::R64G64_SINT;
    case VK_FORMAT_R64G64_SFLOAT: return EVulkanPixelFormat::R64G64_SFLOAT;
    case VK_FORMAT_R64G64B64A64_UINT: return EVulkanPixelFormat::R64G64B64A64_UINT;
    case VK_FORMAT_R64G64B64A64_SINT: return EVulkanPixelFormat::R64G64B64A64_SINT;
    case VK_FORMAT_R64G64B64A64_SFLOAT: return EVulkanPixelFormat::R64G64B64A64_SFLOAT;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return EVulkanPixelFormat::B10G11R11_UFLOAT;
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return EVulkanPixelFormat::E5B9G9R9_UFLOAT;
    case VK_FORMAT_D16_UNORM: return EVulkanPixelFormat::D16_UNORM;
    case VK_FORMAT_D32_SFLOAT: return EVulkanPixelFormat::D32_SFLOAT;
    case VK_FORMAT_D16_UNORM_S8_UINT: return EVulkanPixelFormat::D16_UNORM_S8_UINT;
    case VK_FORMAT_D24_UNORM_S8_UINT: return EVulkanPixelFormat::D24_UNORM_S8_UINT;
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK: return EVulkanPixelFormat::BC1_RGB_UNORM;
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return EVulkanPixelFormat::BC1_RGB_SRGB;
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return EVulkanPixelFormat::BC1_RGBA_UNORM;
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return EVulkanPixelFormat::BC1_RGBA_SRGB;
    case VK_FORMAT_BC2_UNORM_BLOCK: return EVulkanPixelFormat::BC2_UNORM;
    case VK_FORMAT_BC2_SRGB_BLOCK: return EVulkanPixelFormat::BC2_SRGB;
    case VK_FORMAT_BC3_UNORM_BLOCK: return EVulkanPixelFormat::BC3_UNORM;
    case VK_FORMAT_BC3_SRGB_BLOCK: return EVulkanPixelFormat::BC3_SRGB;
    case VK_FORMAT_BC4_UNORM_BLOCK: return EVulkanPixelFormat::BC4_UNORM;
    case VK_FORMAT_BC4_SNORM_BLOCK: return EVulkanPixelFormat::BC4_SNORM;
    case VK_FORMAT_BC5_UNORM_BLOCK: return EVulkanPixelFormat::BC5_UNORM;
    case VK_FORMAT_BC5_SNORM_BLOCK: return EVulkanPixelFormat::BC5_SNORM;
    case VK_FORMAT_BC6H_UFLOAT_BLOCK: return EVulkanPixelFormat::BC6H_UFLOAT;
    case VK_FORMAT_BC6H_SFLOAT_BLOCK: return EVulkanPixelFormat::BC6H_SFLOAT;
    case VK_FORMAT_BC7_UNORM_BLOCK: return EVulkanPixelFormat::BC7_UNORM;
    case VK_FORMAT_BC7_SRGB_BLOCK: return EVulkanPixelFormat::BC7_SRGB;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
inline CONSTEXPR VkPresentModeKHR FVulkanInterop::PresentMode_to_VkPresentModeKHR(EVulkanPresentMode presentMode) NOEXCEPT {
    switch (presentMode) {
    case EVulkanPresentMode::Immediate: return VK_PRESENT_MODE_IMMEDIATE_KHR;
    case EVulkanPresentMode::Fifo: return VK_PRESENT_MODE_FIFO_KHR;
    case EVulkanPresentMode::RelaxedFifo: return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    case EVulkanPresentMode::Mailbox: return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
inline CONSTEXPR EVulkanPresentMode FVulkanInterop::VkPresentModeKHR_to_PresentMode(VkPresentModeKHR vkPresentMode) NOEXCEPT {
    switch (vkPresentMode) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR: return EVulkanPresentMode::Immediate;
    case VK_PRESENT_MODE_FIFO_KHR: return EVulkanPresentMode::Fifo;
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR: return EVulkanPresentMode::RelaxedFifo;
    case VK_PRESENT_MODE_MAILBOX_KHR: return EVulkanPresentMode::Mailbox;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
