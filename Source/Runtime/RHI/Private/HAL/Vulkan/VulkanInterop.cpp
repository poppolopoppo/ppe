#include "stdafx.h"

#include "HAL/Vulkan/VulkanInterop.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanRHIIncludes.h"
#include "HAL/Vulkan/VulkanRHIDevice.h"
#include "HAL/Vulkan/VulkanRHIFixedFunctionState.h"
#include "HAL/Vulkan/VulkanRHIInputAssembly.h"
#include "HAL/Vulkan/VulkanRHIMemoryAllocator.h"
#include "HAL/Vulkan/VulkanRHIPipelineLayout.h"
#include "HAL/Vulkan/VulkanRHIShaderStage.h"
#include "HAL/Vulkan/VulkanRHISurfaceFormat.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VkColorSpaceKHR FVulkanInterop::Vk(EVulkanColorSpace colorSpace) NOEXCEPT {
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
EVulkanColorSpace FVulkanInterop::RHI(VkColorSpaceKHR vkColorSpace) NOEXCEPT {
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
VkFormat FVulkanInterop::Vk(EVulkanPixelFormat pixelFormat) NOEXCEPT {
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
EVulkanPixelFormat FVulkanInterop::RHI(VkFormat vkFormat) NOEXCEPT {
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
VkPresentModeKHR FVulkanInterop::Vk(EVulkanPresentMode presentMode) NOEXCEPT {
    switch (presentMode) {
    case EVulkanPresentMode::Immediate: return VK_PRESENT_MODE_IMMEDIATE_KHR;
    case EVulkanPresentMode::Fifo: return VK_PRESENT_MODE_FIFO_KHR;
    case EVulkanPresentMode::RelaxedFifo: return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    case EVulkanPresentMode::Mailbox: return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanPresentMode FVulkanInterop::RHI(VkPresentModeKHR vkPresentMode) NOEXCEPT {
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
VkBlendFactor FVulkanInterop::Vk(EVulkanBlendFactor value) NOEXCEPT {
    switch (value) {
    case EVulkanBlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
    case EVulkanBlendFactor::One: return VK_BLEND_FACTOR_ONE;
    case EVulkanBlendFactor::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
    case EVulkanBlendFactor::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case EVulkanBlendFactor::DstColor: return VK_BLEND_FACTOR_DST_COLOR;
    case EVulkanBlendFactor::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case EVulkanBlendFactor::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
    case EVulkanBlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case EVulkanBlendFactor::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
    case EVulkanBlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case EVulkanBlendFactor::ConstantColor: return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case EVulkanBlendFactor::OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case EVulkanBlendFactor::ConstantAlpha: return VK_BLEND_FACTOR_CONSTANT_ALPHA;
    case EVulkanBlendFactor::OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    case EVulkanBlendFactor::SrcAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanBlendFactor FVulkanInterop::RHI(VkBlendFactor value) NOEXCEPT {
    switch (value) {
    case VK_BLEND_FACTOR_ZERO: return EVulkanBlendFactor::Zero;
    case VK_BLEND_FACTOR_ONE: return EVulkanBlendFactor::One;
    case VK_BLEND_FACTOR_SRC_COLOR: return EVulkanBlendFactor::SrcColor;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR: return EVulkanBlendFactor::OneMinusSrcColor;
    case VK_BLEND_FACTOR_DST_COLOR: return EVulkanBlendFactor::DstColor;
    case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR: return EVulkanBlendFactor::OneMinusDstColor;
    case VK_BLEND_FACTOR_SRC_ALPHA: return EVulkanBlendFactor::SrcAlpha;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA: return EVulkanBlendFactor::OneMinusSrcAlpha;
    case VK_BLEND_FACTOR_DST_ALPHA: return EVulkanBlendFactor::DstAlpha;
    case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA: return EVulkanBlendFactor::OneMinusDstAlpha;
    case VK_BLEND_FACTOR_CONSTANT_COLOR: return EVulkanBlendFactor::ConstantColor;
    case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR: return EVulkanBlendFactor::OneMinusConstantColor;
    case VK_BLEND_FACTOR_CONSTANT_ALPHA: return EVulkanBlendFactor::ConstantAlpha;
    case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA: return EVulkanBlendFactor::OneMinusConstantAlpha;
    case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE: return EVulkanBlendFactor::SrcAlphaSaturate;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
VkBlendOp FVulkanInterop::Vk(EVulkanBlendOp value) NOEXCEPT {
    switch (value) {
    case EVulkanBlendOp::Add: return VK_BLEND_OP_ADD;
    case EVulkanBlendOp::Subtract: return VK_BLEND_OP_SUBTRACT;
    case EVulkanBlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case EVulkanBlendOp::Min: return VK_BLEND_OP_MIN;
    case EVulkanBlendOp::Max: return VK_BLEND_OP_MAX;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanBlendOp FVulkanInterop::RHI(VkBlendOp value) NOEXCEPT {
    switch (value) {
    case VK_BLEND_OP_ADD: return EVulkanBlendOp::Add;
    case VK_BLEND_OP_SUBTRACT: return EVulkanBlendOp::Subtract;
    case VK_BLEND_OP_REVERSE_SUBTRACT: return EVulkanBlendOp::ReverseSubtract;
    case VK_BLEND_OP_MIN: return EVulkanBlendOp::Min;
    case VK_BLEND_OP_MAX: return EVulkanBlendOp::Max;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
VkColorComponentFlagBits FVulkanInterop::Vk(EVulkanColorComponentMask value) NOEXCEPT {
    switch (value) {
    case EVulkanColorComponentMask::None: return VkColorComponentFlagBits(0);
    case EVulkanColorComponentMask::R: return VK_COLOR_COMPONENT_R_BIT;
    case EVulkanColorComponentMask::G: return VK_COLOR_COMPONENT_G_BIT;
    case EVulkanColorComponentMask::B: return VK_COLOR_COMPONENT_B_BIT;
    case EVulkanColorComponentMask::A: return VK_COLOR_COMPONENT_A_BIT;
    case EVulkanColorComponentMask::RA: return VkColorComponentFlagBits(VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_A_BIT);
    case EVulkanColorComponentMask::RG: return VkColorComponentFlagBits(VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT);
    case EVulkanColorComponentMask::RGB: return VkColorComponentFlagBits(VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT);
    case EVulkanColorComponentMask::RGBA: return VkColorComponentFlagBits(VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT);
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanColorComponentMask FVulkanInterop::RHI(VkColorComponentFlagBits value) NOEXCEPT {
    switch (VkColorComponentFlags(value)) {
    case VkColorComponentFlags(0): return EVulkanColorComponentMask::None;
    case VK_COLOR_COMPONENT_R_BIT: return EVulkanColorComponentMask::R;
    case VK_COLOR_COMPONENT_G_BIT: return EVulkanColorComponentMask::G;
    case VK_COLOR_COMPONENT_B_BIT: return EVulkanColorComponentMask::B;
    case VK_COLOR_COMPONENT_A_BIT: return EVulkanColorComponentMask::A;
    case VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_A_BIT: return EVulkanColorComponentMask::RA;
    case VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT: return EVulkanColorComponentMask::RG;
    case VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT: return EVulkanColorComponentMask::RGB;
    case VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT: return EVulkanColorComponentMask::RGBA;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
VkLogicOp FVulkanInterop::Vk(EVulkanLogicOp value) NOEXCEPT {
    switch (value) {
    case EVulkanLogicOp::Clear: return VK_LOGIC_OP_CLEAR;
    case EVulkanLogicOp::And: return VK_LOGIC_OP_AND;
    case EVulkanLogicOp::AndReverse: return VK_LOGIC_OP_AND_REVERSE;
    case EVulkanLogicOp::Copy: return VK_LOGIC_OP_COPY;
    case EVulkanLogicOp::AndInverted: return VK_LOGIC_OP_AND_INVERTED;
    case EVulkanLogicOp::NoOp: return VK_LOGIC_OP_NO_OP;
    case EVulkanLogicOp::Xor: return VK_LOGIC_OP_XOR;
    case EVulkanLogicOp::Or: return VK_LOGIC_OP_OR;
    case EVulkanLogicOp::Nor: return VK_LOGIC_OP_NOR;
    case EVulkanLogicOp::Equivalent: return VK_LOGIC_OP_EQUIVALENT;
    case EVulkanLogicOp::Invert: return VK_LOGIC_OP_INVERT;
    case EVulkanLogicOp::OrReverse: return VK_LOGIC_OP_OR_REVERSE;
    case EVulkanLogicOp::CopyInverted: return VK_LOGIC_OP_COPY_INVERTED;
    case EVulkanLogicOp::OrInverted: return VK_LOGIC_OP_OR_INVERTED;
    case EVulkanLogicOp::Nand: return VK_LOGIC_OP_NAND;
    case EVulkanLogicOp::Set: return VK_LOGIC_OP_SET;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanLogicOp FVulkanInterop::RHI(VkLogicOp value) NOEXCEPT {
    switch (value) {
    case VK_LOGIC_OP_CLEAR: return EVulkanLogicOp::Clear;
    case VK_LOGIC_OP_AND: return EVulkanLogicOp::And;
    case VK_LOGIC_OP_AND_REVERSE: return EVulkanLogicOp::AndReverse;
    case VK_LOGIC_OP_COPY: return EVulkanLogicOp::Copy;
    case VK_LOGIC_OP_AND_INVERTED: return EVulkanLogicOp::AndInverted;
    case VK_LOGIC_OP_NO_OP: return EVulkanLogicOp::NoOp;
    case VK_LOGIC_OP_XOR: return EVulkanLogicOp::Xor;
    case VK_LOGIC_OP_OR: return EVulkanLogicOp::Or;
    case VK_LOGIC_OP_NOR: return EVulkanLogicOp::Nor;
    case VK_LOGIC_OP_EQUIVALENT: return EVulkanLogicOp::Equivalent;
    case VK_LOGIC_OP_INVERT: return EVulkanLogicOp::Invert;
    case VK_LOGIC_OP_OR_REVERSE: return EVulkanLogicOp::OrReverse;
    case VK_LOGIC_OP_COPY_INVERTED: return EVulkanLogicOp::CopyInverted;
    case VK_LOGIC_OP_OR_INVERTED: return EVulkanLogicOp::OrInverted;
    case VK_LOGIC_OP_NAND: return EVulkanLogicOp::Nand;
    case VK_LOGIC_OP_SET: return EVulkanLogicOp::Set;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
VkCompareOp FVulkanInterop::Vk(EVulkanCompareOp value) NOEXCEPT {
    switch (value) {
    case EVulkanCompareOp::Never: return VK_COMPARE_OP_NEVER;
    case EVulkanCompareOp::Less: return VK_COMPARE_OP_LESS;
    case EVulkanCompareOp::Equal: return VK_COMPARE_OP_EQUAL;
    case EVulkanCompareOp::LessOrEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
    case EVulkanCompareOp::Greater: return VK_COMPARE_OP_GREATER;
    case EVulkanCompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
    case EVulkanCompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case EVulkanCompareOp::Always: return VK_COMPARE_OP_ALWAYS;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanCompareOp FVulkanInterop::RHI(VkCompareOp value) NOEXCEPT {
    switch (value) {
    case VK_COMPARE_OP_NEVER: return EVulkanCompareOp::Never;
    case VK_COMPARE_OP_LESS: return EVulkanCompareOp::Less;
    case VK_COMPARE_OP_EQUAL: return EVulkanCompareOp::Equal;
    case VK_COMPARE_OP_LESS_OR_EQUAL: return EVulkanCompareOp::LessOrEqual;
    case VK_COMPARE_OP_GREATER: return EVulkanCompareOp::Greater;
    case VK_COMPARE_OP_NOT_EQUAL: return EVulkanCompareOp::NotEqual;
    case VK_COMPARE_OP_GREATER_OR_EQUAL: return EVulkanCompareOp::GreaterOrEqual;
    case VK_COMPARE_OP_ALWAYS: return EVulkanCompareOp::Always;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
VkStencilOp FVulkanInterop::Vk(EVulkanStencilOp value) NOEXCEPT {
    switch (value) {
    case EVulkanStencilOp::Keep: return VK_STENCIL_OP_KEEP;
    case EVulkanStencilOp::Zero: return VK_STENCIL_OP_ZERO;
    case EVulkanStencilOp::Replace: return VK_STENCIL_OP_REPLACE;
    case EVulkanStencilOp::IncrementAndClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case EVulkanStencilOp::DecrementAndClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case EVulkanStencilOp::Invert: return VK_STENCIL_OP_INVERT;
    case EVulkanStencilOp::IncrementAndWrap: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case EVulkanStencilOp::DecrementAndWrap: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanStencilOp FVulkanInterop::RHI(VkStencilOp value) NOEXCEPT {
    switch (value) {
    case VK_STENCIL_OP_KEEP: return EVulkanStencilOp::Keep;
    case VK_STENCIL_OP_ZERO: return EVulkanStencilOp::Zero;
    case VK_STENCIL_OP_REPLACE: return EVulkanStencilOp::Replace;
    case VK_STENCIL_OP_INCREMENT_AND_CLAMP: return EVulkanStencilOp::IncrementAndClamp;
    case VK_STENCIL_OP_DECREMENT_AND_CLAMP: return EVulkanStencilOp::DecrementAndClamp;
    case VK_STENCIL_OP_INVERT: return EVulkanStencilOp::Invert;
    case VK_STENCIL_OP_INCREMENT_AND_WRAP: return EVulkanStencilOp::IncrementAndWrap;
    case VK_STENCIL_OP_DECREMENT_AND_WRAP: return EVulkanStencilOp::DecrementAndWrap;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
VkCullModeFlagBits FVulkanInterop::Vk(EVulkanCullMode value) NOEXCEPT {
    switch (value) {
    case EVulkanCullMode::None: return VK_CULL_MODE_NONE;
    case EVulkanCullMode::Back: return VK_CULL_MODE_BACK_BIT;
    case EVulkanCullMode::Front: return VK_CULL_MODE_FRONT_BIT;
    case EVulkanCullMode::FrontAndBack: return VkCullModeFlagBits(VK_CULL_MODE_BACK_BIT|VK_CULL_MODE_FRONT_BIT);
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanCullMode FVulkanInterop::RHI(VkCullModeFlagBits value) NOEXCEPT {
    switch (value) {
    case VK_CULL_MODE_NONE: return EVulkanCullMode::None;
    case VK_CULL_MODE_BACK_BIT: return EVulkanCullMode::Back;
    case VK_CULL_MODE_FRONT_BIT: return EVulkanCullMode::Front;
    case VK_CULL_MODE_BACK_BIT|VK_CULL_MODE_FRONT_BIT: return EVulkanCullMode::FrontAndBack;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
VkFrontFace FVulkanInterop::Vk(EVulkanFrontFace value) NOEXCEPT {
    switch (value) {
    case EVulkanFrontFace::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    case EVulkanFrontFace::Clockwise: return VK_FRONT_FACE_CLOCKWISE;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanFrontFace FVulkanInterop::RHI(VkFrontFace value) NOEXCEPT {
    switch (value) {
    case VK_FRONT_FACE_COUNTER_CLOCKWISE: return EVulkanFrontFace::CounterClockwise;
    case VK_FRONT_FACE_CLOCKWISE: return EVulkanFrontFace::Clockwise;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
VkPolygonMode FVulkanInterop::Vk(EVulkanPolygonMode value) NOEXCEPT {
    switch (value) {
    case EVulkanPolygonMode::Fill: return VK_POLYGON_MODE_FILL;
    case EVulkanPolygonMode::Line: return VK_POLYGON_MODE_LINE;
    case EVulkanPolygonMode::Point: return VK_POLYGON_MODE_POINT;
    case EVulkanPolygonMode::FillRectangle: return VK_POLYGON_MODE_FILL_RECTANGLE_NV;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanPolygonMode FVulkanInterop::RHI(VkPolygonMode value) NOEXCEPT {
    switch (value) {
    case VK_POLYGON_MODE_FILL: return EVulkanPolygonMode::Fill;
    case VK_POLYGON_MODE_LINE: return EVulkanPolygonMode::Line;
    case VK_POLYGON_MODE_POINT: return EVulkanPolygonMode::Point;
    case VK_POLYGON_MODE_FILL_RECTANGLE_NV: return EVulkanPolygonMode::FillRectangle;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
VkConservativeRasterizationModeEXT FVulkanInterop::Vk(EVulkanConservativeRasterizationMode value) NOEXCEPT {
    switch (value) {
    case EVulkanConservativeRasterizationMode::Disabled: return VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT;
    case EVulkanConservativeRasterizationMode::OverEstimate: return VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
    case EVulkanConservativeRasterizationMode::UnderEstimate: return VK_CONSERVATIVE_RASTERIZATION_MODE_UNDERESTIMATE_EXT;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanConservativeRasterizationMode FVulkanInterop::RHI(VkConservativeRasterizationModeEXT value) NOEXCEPT {
    switch (value) {
    case VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT: return EVulkanConservativeRasterizationMode::Disabled;
    case VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT: return EVulkanConservativeRasterizationMode::OverEstimate;
    case VK_CONSERVATIVE_RASTERIZATION_MODE_UNDERESTIMATE_EXT: return EVulkanConservativeRasterizationMode::UnderEstimate;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VkDynamicState FVulkanInterop::Vk(EVulkanDynamicState value) NOEXCEPT {
    switch (value) {
    case EVulkanDynamicState::None: AssertNotReached();
    case EVulkanDynamicState::Viewport: return VK_DYNAMIC_STATE_VIEWPORT;
    case EVulkanDynamicState::Scissor: return VK_DYNAMIC_STATE_SCISSOR;
    case EVulkanDynamicState::LineWidth: return VK_DYNAMIC_STATE_LINE_WIDTH;
    case EVulkanDynamicState::DepthBias: return VK_DYNAMIC_STATE_DEPTH_BIAS;
    case EVulkanDynamicState::BlendConstants: return VK_DYNAMIC_STATE_BLEND_CONSTANTS;
    case EVulkanDynamicState::DepthBounds: return VK_DYNAMIC_STATE_DEPTH_BOUNDS;
    case EVulkanDynamicState::StencilCompareMask: return VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
    case EVulkanDynamicState::StencilWriteMask: return VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
    case EVulkanDynamicState::StencilReference: return VK_DYNAMIC_STATE_STENCIL_REFERENCE;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanDynamicState FVulkanInterop::RHI(VkDynamicState value) NOEXCEPT {
    switch (value) {
    case VK_DYNAMIC_STATE_VIEWPORT: return EVulkanDynamicState::Viewport;
    case VK_DYNAMIC_STATE_SCISSOR: return EVulkanDynamicState::Scissor;
    case VK_DYNAMIC_STATE_LINE_WIDTH: return EVulkanDynamicState::LineWidth;
    case VK_DYNAMIC_STATE_DEPTH_BIAS: return EVulkanDynamicState::DepthBias;
    case VK_DYNAMIC_STATE_BLEND_CONSTANTS: return EVulkanDynamicState::BlendConstants;
    case VK_DYNAMIC_STATE_DEPTH_BOUNDS: return EVulkanDynamicState::DepthBounds;
    case VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK: return EVulkanDynamicState::StencilCompareMask;
    case VK_DYNAMIC_STATE_STENCIL_WRITE_MASK: return EVulkanDynamicState::StencilWriteMask;
    case VK_DYNAMIC_STATE_STENCIL_REFERENCE: return EVulkanDynamicState::StencilReference;

    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VkPrimitiveTopology FVulkanInterop::Vk(EVulkanPrimitiveTopology value) NOEXCEPT {
    switch (value) {
    case EVulkanPrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case EVulkanPrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case EVulkanPrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case EVulkanPrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case EVulkanPrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case EVulkanPrimitiveTopology::TriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    case EVulkanPrimitiveTopology::LineListWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    case EVulkanPrimitiveTopology::LineStripWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    case EVulkanPrimitiveTopology::TriangleListWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
    case EVulkanPrimitiveTopology::TriangleStripWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    case EVulkanPrimitiveTopology::PatchList: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanPrimitiveTopology FVulkanInterop::RHI(VkPrimitiveTopology value) NOEXCEPT {
    switch (value) {
    case VK_PRIMITIVE_TOPOLOGY_POINT_LIST: return EVulkanPrimitiveTopology::PointList;
    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST: return EVulkanPrimitiveTopology::LineList;
    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: return EVulkanPrimitiveTopology::LineStrip;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: return EVulkanPrimitiveTopology::TriangleList;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: return EVulkanPrimitiveTopology::TriangleStrip;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN: return EVulkanPrimitiveTopology::TriangleFan;
    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY: return EVulkanPrimitiveTopology::LineListWithAdjacency;
    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY: return EVulkanPrimitiveTopology::LineStripWithAdjacency;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY: return EVulkanPrimitiveTopology::TriangleListWithAdjacency;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY: return EVulkanPrimitiveTopology::TriangleStripWithAdjacency;
    case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST: return EVulkanPrimitiveTopology::PatchList;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
VkVertexInputRate FVulkanInterop::Vk(EVulkanVertexInputRate value) NOEXCEPT {
    switch (value) {
    case EVulkanVertexInputRate::Vertex: return VK_VERTEX_INPUT_RATE_VERTEX;
    case EVulkanVertexInputRate::Instance: return VK_VERTEX_INPUT_RATE_INSTANCE;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanVertexInputRate FVulkanInterop::RHI(VkVertexInputRate value) NOEXCEPT {
    switch (value) {
    case VK_VERTEX_INPUT_RATE_VERTEX: return EVulkanVertexInputRate::Vertex;
    case VK_VERTEX_INPUT_RATE_INSTANCE: return EVulkanVertexInputRate::Instance;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VkMemoryPropertyFlagBits FVulkanInterop::Vk(EVulkanMemoryTypeFlags value) NOEXCEPT {
    VkMemoryPropertyFlags vkFlags{0};
    if (value ^ EVulkanMemoryTypeFlags::DeviceLocal) vkFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (value ^ EVulkanMemoryTypeFlags::HostVisible) vkFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    if (value ^ EVulkanMemoryTypeFlags::HostCoherent) vkFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (value ^ EVulkanMemoryTypeFlags::HostCached) vkFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    return VkMemoryPropertyFlagBits(vkFlags);
}
//----------------------------------------------------------------------------
EVulkanMemoryTypeFlags FVulkanInterop::RHI(VkMemoryPropertyFlagBits value) NOEXCEPT {
    EVulkanMemoryTypeFlags flags{0};
    if (value & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) flags = flags | EVulkanMemoryTypeFlags::DeviceLocal;
    if (value & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) flags = flags | EVulkanMemoryTypeFlags::HostVisible;
    if (value & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) flags = flags | EVulkanMemoryTypeFlags::HostCoherent;
    if (value & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) flags = flags | EVulkanMemoryTypeFlags::HostCached;
    return flags;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VkShaderStageFlagBits FVulkanInterop::Vk(EVulkanShaderStageFlags value) NOEXCEPT {
    VkShaderStageFlags vkFlags{0};
    if (value ^ EVulkanShaderStageFlags::Vertex) vkFlags |= VK_SHADER_STAGE_VERTEX_BIT;
    if (value ^ EVulkanShaderStageFlags::TesselationControl) vkFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    if (value ^ EVulkanShaderStageFlags::TesselationEvaluation) vkFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    if (value ^ EVulkanShaderStageFlags::Geometry) vkFlags |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if (value ^ EVulkanShaderStageFlags::Fragment) vkFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if (value ^ EVulkanShaderStageFlags::Compute) vkFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
    if (value ^ EVulkanShaderStageFlags::RayCast) vkFlags |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    if (value ^ EVulkanShaderStageFlags::RayAnyHit) vkFlags |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    if (value ^ EVulkanShaderStageFlags::RayClosestHit) vkFlags |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    if (value ^ EVulkanShaderStageFlags::RayMiss) vkFlags |= VK_SHADER_STAGE_MISS_BIT_KHR;
    if (value ^ EVulkanShaderStageFlags::RayIntersection) vkFlags |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    if (value ^ EVulkanShaderStageFlags::RayCallable) vkFlags |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;
    if (value ^ EVulkanShaderStageFlags::Mesh) vkFlags |= VK_SHADER_STAGE_MESH_BIT_NV;
    if (value ^ EVulkanShaderStageFlags::Task) vkFlags |= VK_SHADER_STAGE_TASK_BIT_NV;
    return VkShaderStageFlagBits(vkFlags);
}
//----------------------------------------------------------------------------
EVulkanShaderStageFlags FVulkanInterop::RHI(VkShaderStageFlagBits value) NOEXCEPT {
    EVulkanShaderStageFlags flags{0};
    if (value & VK_SHADER_STAGE_VERTEX_BIT) flags = flags | EVulkanShaderStageFlags::Vertex;
    if (value & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) flags = flags | EVulkanShaderStageFlags::TesselationControl;
    if (value & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) flags = flags | EVulkanShaderStageFlags::TesselationEvaluation;
    if (value & VK_SHADER_STAGE_GEOMETRY_BIT) flags = flags | EVulkanShaderStageFlags::Geometry;
    if (value & VK_SHADER_STAGE_FRAGMENT_BIT) flags = flags | EVulkanShaderStageFlags::Fragment;
    if (value & VK_SHADER_STAGE_COMPUTE_BIT) flags = flags | EVulkanShaderStageFlags::Compute;
    if (value & VK_SHADER_STAGE_RAYGEN_BIT_KHR) flags = flags | EVulkanShaderStageFlags::RayCast;
    if (value & VK_SHADER_STAGE_ANY_HIT_BIT_KHR) flags = flags | EVulkanShaderStageFlags::RayAnyHit;
    if (value & VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) flags = flags | EVulkanShaderStageFlags::RayClosestHit;
    if (value & VK_SHADER_STAGE_MISS_BIT_KHR) flags = flags | EVulkanShaderStageFlags::RayMiss;
    if (value & VK_SHADER_STAGE_INTERSECTION_BIT_KHR) flags = flags | EVulkanShaderStageFlags::RayIntersection;
    if (value & VK_SHADER_STAGE_CALLABLE_BIT_KHR) flags = flags | EVulkanShaderStageFlags::RayCallable;
    if (value & VK_SHADER_STAGE_MESH_BIT_NV) flags = flags | EVulkanShaderStageFlags::Mesh;
    if (value & VK_SHADER_STAGE_TASK_BIT_NV) flags = flags | EVulkanShaderStageFlags::Task;
    return flags;
}
//----------------------------------------------------------------------------
VkPipelineShaderStageCreateFlagBits FVulkanInterop::Vk(EVulkanShaderStageCreateFlags value) NOEXCEPT {
    VkPipelineShaderStageCreateFlags vkFlags{0};
    if (value ^ EVulkanShaderStageCreateFlags::AllowVaryingSubgroupSize) vkFlags |= VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT;
    if (value ^ EVulkanShaderStageCreateFlags::RequireFullSubgroups) vkFlags |= VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
    return VkPipelineShaderStageCreateFlagBits(vkFlags);
}
//----------------------------------------------------------------------------
EVulkanShaderStageCreateFlags FVulkanInterop::RHI(VkPipelineShaderStageCreateFlagBits value) NOEXCEPT {
    EVulkanShaderStageCreateFlags flags{0};
    if (value & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) flags = flags | EVulkanShaderStageCreateFlags::AllowVaryingSubgroupSize;
    if (value & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) flags = flags | EVulkanShaderStageCreateFlags::RequireFullSubgroups;
    return flags;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VkDescriptorBindingFlagBits FVulkanInterop::Vk(EVulkanDescriptorFlags value) NOEXCEPT {
    VkDescriptorBindingFlags vkFlags{0};
    if (value ^ EVulkanDescriptorFlags::UpdateAfterBind) vkFlags |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    if (value ^ EVulkanDescriptorFlags::UpdateUnusedWhilePending) vkFlags |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
    if (value ^ EVulkanDescriptorFlags::PartiallyBound) vkFlags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
    if (value ^ EVulkanDescriptorFlags::VariableDescriptorCount) vkFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
    return VkDescriptorBindingFlagBits(vkFlags);
}
//----------------------------------------------------------------------------
EVulkanDescriptorFlags FVulkanInterop::RHI(VkDescriptorBindingFlagBits value) NOEXCEPT {
    EVulkanDescriptorFlags flags{0};
    if (value & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT) flags = flags | EVulkanDescriptorFlags::UpdateAfterBind;
    if (value & VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT) flags = flags | EVulkanDescriptorFlags::UpdateUnusedWhilePending;
    if (value & VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT) flags = flags | EVulkanDescriptorFlags::PartiallyBound;
    if (value & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) flags = flags | EVulkanDescriptorFlags::VariableDescriptorCount;
    return flags;
}
//----------------------------------------------------------------------------
VkDescriptorType FVulkanInterop::Vk(EVulkanDescriptorType value) NOEXCEPT {
    switch (value) {
    case EVulkanDescriptorType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
    case EVulkanDescriptorType::CombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case EVulkanDescriptorType::SampledImage: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case EVulkanDescriptorType::StorageImage: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case EVulkanDescriptorType::UniformTexelBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case EVulkanDescriptorType::StorageTexelBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case EVulkanDescriptorType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case EVulkanDescriptorType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case EVulkanDescriptorType::UniformBufferDynamic: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case EVulkanDescriptorType::StorageBufferDynamic: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case EVulkanDescriptorType::InputAttachment: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    case EVulkanDescriptorType::InlineUniformBlock: return VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    case EVulkanDescriptorType::AccelerationStructure: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EVulkanDescriptorType FVulkanInterop::RHI(VkDescriptorType value) NOEXCEPT {
    switch (value) {
    case VK_DESCRIPTOR_TYPE_SAMPLER: return EVulkanDescriptorType::Sampler;
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return EVulkanDescriptorType::CombinedImageSampler;
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return EVulkanDescriptorType::SampledImage;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: return EVulkanDescriptorType::StorageImage;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return EVulkanDescriptorType::UniformTexelBuffer;
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return EVulkanDescriptorType::StorageTexelBuffer;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return EVulkanDescriptorType::UniformBuffer;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: return EVulkanDescriptorType::StorageBuffer;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return EVulkanDescriptorType::UniformBufferDynamic;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return EVulkanDescriptorType::StorageBufferDynamic;
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return EVulkanDescriptorType::InputAttachment;
    case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT: return EVulkanDescriptorType::InlineUniformBlock;
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: return EVulkanDescriptorType::AccelerationStructure;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
VkDescriptorSetLayoutCreateFlagBits FVulkanInterop::Vk(EVulkanDescriptorSetFlags value) NOEXCEPT {
    VkDescriptorSetLayoutCreateFlags vkFlags{0};
    if (value ^ EVulkanDescriptorSetFlags::PushDescriptor) vkFlags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
    if (value ^ EVulkanDescriptorSetFlags::UpdateAfterBindPool) vkFlags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    return VkDescriptorSetLayoutCreateFlagBits(vkFlags);
}
//----------------------------------------------------------------------------
EVulkanDescriptorSetFlags FVulkanInterop::RHI(VkDescriptorSetLayoutCreateFlagBits value) NOEXCEPT {
    EVulkanDescriptorSetFlags flags{0};
    if (value & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR) flags = flags | EVulkanDescriptorSetFlags::PushDescriptor;
    if (value & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) flags = flags | EVulkanDescriptorSetFlags::UpdateAfterBindPool;
    return flags;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
