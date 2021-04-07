#pragma once

#include "Vulkan/Vulkan_fwd.h"

#include "RHI/EnumHelpers.h"
#include "RHI/ImageHelpers.h"
#include "RHI/RayTracingEnums.h"
#include "RHI/RenderStateEnums.h"
#include "RHI/ResourceEnums.h"
#include "RHI/SamplerEnums.h"
#include "RHI/ShaderEnums.h"
#include "RHI/VertexEnums.h"

#include "Meta/Enum.h"
#include "RHI/PixelFormatInfo.h"
#include "RHI/ResourceState.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Enum operators for Vk:
//----------------------------------------------------------------------------
ENUM_FLAGS(VkPipelineStageFlagBits);
ENUM_FLAGS(VkAccessFlagBits);
ENUM_FLAGS(VkDependencyFlagBits);
ENUM_FLAGS(VkImageAspectFlagBits);
ENUM_FLAGS(VkStencilFaceFlagBits);
ENUM_FLAGS(VkShaderStageFlagBits);
ENUM_FLAGS(VkImageCreateFlagBits);
ENUM_FLAGS(VkQueueFlagBits);
ENUM_FLAGS(VkImageUsageFlagBits);
ENUM_FLAGS(VkBufferUsageFlagBits);
ENUM_FLAGS(VkSampleCountFlagBits);
ENUM_FLAGS(VkGeometryFlagBitsNV);
ENUM_FLAGS(VkGeometryInstanceFlagBitsNV);
ENUM_FLAGS(VkBuildAccelerationStructureFlagBitsNV);
//----------------------------------------------------------------------------
// Memory types:
//----------------------------------------------------------------------------
enum class EVulkanMemoryType : u32 {
    HostRead        = u32(EMemoryType::HostRead),
    HostWrite       = u32(EMemoryType::HostWrite),
    Dedicated       = u32(EMemoryType::Dedicated),
    //AllowAliasing = u32(EMemoryType::AllowAliasing),
    //Sparse        = u32(EMemoryType::Sparse),
    _Offset         = u32(EMemoryType::_Last)-1,

    LocalInGPU      = _Offset << 1,
    HostCoherent    = _Offset << 2,
    HostCached      = _Offset << 3,
    ForBuffer       = _Offset << 4,
    ForImage        = _Offset << 5,
    Virtual         = _Offset << 6,
    _Last,

    All             = ((_Last-1) << 1) - 1,
    HostVisible     = HostRead | HostWrite,
};
ENUM_FLAGS(EVulkanMemoryType);
//----------------------------------------------------------------------------
// Execution ordering
//----------------------------------------------------------------------------
enum class EVulkanExecutionOrder : u32 {
    Initial        = 0,
    First           = 1,
    Final           = 0x80000000u,
    Unknown         = ~0u,
};
//----------------------------------------------------------------------------
CONSTEXPR EVulkanExecutionOrder& operator ++(EVulkanExecutionOrder& value) NOEXCEPT {
    return (value = static_cast<EVulkanExecutionOrder>(static_cast<u32>(value) + 1));
}
//----------------------------------------------------------------------------
// Queue family:
//----------------------------------------------------------------------------
enum class EVulkanQueueFamily : u32 {
    _Count          = 31,

    External        = VK_QUEUE_FAMILY_EXTERNAL,
    Foreign         = VK_QUEUE_FAMILY_FOREIGN_EXT,
    Ignored         = VK_QUEUE_FAMILY_IGNORED,

    Unknown         = Ignored,
};
//----------------------------------------------------------------------------
enum class EVulkanQueueFamilyMask : u32 {
    All             = ~0u,
    Unknown         = 0,
};
ENUM_FLAGS(EVulkanQueueFamilyMask);
//----------------------------------------------------------------------------
CONSTEXPR EVulkanQueueFamilyMask operator |(EVulkanQueueFamilyMask mask, EVulkanQueueFamily family) {
    return static_cast<EVulkanQueueFamilyMask>(
        u32(mask) | (u32(1) << (u32(family) & u32(EVulkanQueueFamily::_Count)))
    );
}
//----------------------------------------------------------------------------
CONSTEXPR EVulkanQueueFamilyMask& operator |=(EVulkanQueueFamilyMask& mask, EVulkanQueueFamily family) {
    return (mask = (mask | family));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RHI to Vk:
//----------------------------------------------------------------------------
CONSTEXPR VkSampleCountFlagBits VkCast(FMultiSamples value) NOEXCEPT;
CONSTEXPR VkLogicOp VkCast(ELogicOp value);
CONSTEXPR VkBlendFactor VkCast(EBlendFactor factor);
CONSTEXPR VkBlendOp VkCast(EBlendOp value);
CONSTEXPR VkFormat VkCast(EVertexFormat value);
CONSTEXPR VkVertexInputRate VkCast(EVertexInputRate value);
CONSTEXPR VkShaderStageFlagBits VkCast(EShaderType value);
CONSTEXPR VkShaderStageFlagBits VkEnumCast(EShaderStages values);
CONSTEXPR VkDynamicState VkCast(EPipelineDynamicState value);
CONSTEXPR VkAttachmentLoadOp VkCast(EAttachmentLoadOp value);
CONSTEXPR VkAttachmentStoreOp VkCast(EAttachmentStoreOp value);
CONSTEXPR VkCompareOp VkCast(ECompareOp value);
CONSTEXPR VkStencilOp VkCast(EStencilOp value);
CONSTEXPR VkPolygonMode VkCast(EPolygonMode value);
CONSTEXPR VkCullModeFlagBits VkCast(ECullMode value);
CONSTEXPR VkFilter VkCast(ETextureFilter value);
CONSTEXPR VkSamplerMipmapMode VkCast(EMipmapFilter value);
CONSTEXPR VkSamplerAddressMode VkCast(EAddressMode value);
CONSTEXPR VkBorderColor VkCast(EBorderColor value);
CONSTEXPR VkImageViewType VkCast(EImageType value);
CONSTEXPR VkImageUsageFlagBits VkCast(EImageUsage values);
CONSTEXPR VkImageAspectFlagBits VkCast(EImageAspect values);
CONSTEXPR VkImageAspectFlagBits VkCast(EImageAspect values, EPixelFormat format);
CONSTEXPR VkBufferUsageFlagBits VkCast(EBufferUsage values);
CONSTEXPR VkIndexType VkCast(EIndexFormat value);
CONSTEXPR VkGeometryFlagBitsKHR VkCast(ERayTracingGeometryFlags values);
CONSTEXPR VkGeometryInstanceFlagBitsKHR VkCast(ERayTracingInstanceFlags values);
CONSTEXPR VkBuildAccelerationStructureFlagBitsKHR VkCast(ERayTracingBuildFlags values);
//----------------------------------------------------------------------------
// Vk to RHI:
//----------------------------------------------------------------------------
CONSTEXPR EBufferUsage RHICast(VkBufferUsageFlagBits flags);
CONSTEXPR EImageType RHICast(VkImageType type, VkImageCreateFlags flags, u32 arrayLayers, VkSampleCountFlagBits samples);
CONSTEXPR EImageUsage RHICast(VkImageUsageFlagBits flags);
CONSTEXPR FMultiSamples RHICast(VkSampleCountFlagBits flags);
CONSTEXPR EImageAspect RHICast(VkImageAspectFlagBits flags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Resource state:
//----------------------------------------------------------------------------
CONSTEXPR VkPipelineStageFlagBits EResourceState_ToPipelineStages(EResourceState value);
CONSTEXPR VkAccessFlagBits EResourceState_ToAccess(EResourceState value);
CONSTEXPR VkImageLayout EResourceState_ToImageLayout(EResourceState value, VkImageAspectFlags aspect);
CONSTEXPR VkShadingRate VkCast(EShadingRatePalette value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Pixel format:
//----------------------------------------------------------------------------
CONSTEXPR VkFormat VkCast(EPixelFormat value);
CONSTEXPR EPixelFormat RHICast(VkFormat value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#include "Vulkan/Common/VulkanEnums-inl.h"
