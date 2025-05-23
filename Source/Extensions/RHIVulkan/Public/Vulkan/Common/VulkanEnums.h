#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/EnumHelpers.h"
#include "RHI/ResourceEnums.h"
#include "RHI/ResourceState.h"
#include "RHI/VertexEnums.h"

#include "Meta/Enum.h"

//----------------------------------------------------------------------------
// Enum operators for Vk:
//----------------------------------------------------------------------------
ENUM_FLAGS(VkMemoryHeapFlagBits);
ENUM_FLAGS(VkMemoryPropertyFlagBits);
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

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
// Memory types:
//----------------------------------------------------------------------------
enum class EVulkanMemoryType : u32 {
    HostRead        = u32(EMemoryType::HostRead),
    HostWrite       = u32(EMemoryType::HostWrite),
    Dedicated       = u32(EMemoryType::Dedicated),
    //AllowAliasing = u32(EMemoryType::AllowAliasing),  // #TODO: memory aliasing
    //Sparse        = u32(EMemoryType::Sparse),         // #TODO: sparse resource

    LocalInGPU      = EMemoryType_Last << 1,
    HostCoherent    = EMemoryType_Last << 2,
    HostCached      = EMemoryType_Last << 3,
    ForBuffer       = EMemoryType_Last << 4,
    ForImage        = EMemoryType_Last << 5,
    Virtual         = EMemoryType_Last << 6,
    _Last,

    All             = ((_Last-1) << 1) - 1,
    HostVisible     = HostRead | HostWrite,
};
ENUM_FLAGS(EVulkanMemoryType);
inline CONSTEXPR EVulkanMemoryType EVulkanMemoryType_All{ (u32(EVulkanMemoryType::Virtual) << 1_u32) - 1_u32 };
inline CONSTEXPR EVulkanMemoryType EVulkanMemoryType_HostVisible{ EVulkanMemoryType::HostRead | EVulkanMemoryType::HostWrite };
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
CONSTEXPR EVulkanExecutionOrder operator ""_execution_order (unsigned long long value) {
    return static_cast<EVulkanExecutionOrder>(static_cast<u32>(value));
}
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
        static_cast<u32>(mask) | (static_cast<u32>(1) << (static_cast<u32>(family) & static_cast<u32>(EVulkanQueueFamily::_Count)))
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
PPE_RHIVULKAN_API VkSampleCountFlagBits VkCast(FMultiSamples value) NOEXCEPT;
PPE_RHIVULKAN_API VkLogicOp VkCast(ELogicOp value);
PPE_RHIVULKAN_API VkBlendFactor VkCast(EBlendFactor factor);
PPE_RHIVULKAN_API VkBlendOp VkCast(EBlendOp value);
PPE_RHIVULKAN_API VkFormat VkCast(EVertexFormat value);
PPE_RHIVULKAN_API VkVertexInputRate VkCast(EVertexInputRate value);
PPE_RHIVULKAN_API VkShaderStageFlagBits VkCast(EShaderType value);
PPE_RHIVULKAN_API VkShaderStageFlagBits VkCast(EShaderStages values);
PPE_RHIVULKAN_API VkDynamicState VkCast(EPipelineDynamicState value);
PPE_RHIVULKAN_API VkAttachmentLoadOp VkCast(EAttachmentLoadOp value, const FVulkanDevice& device);
PPE_RHIVULKAN_API VkAttachmentStoreOp VkCast(EAttachmentStoreOp value, const FVulkanDevice& device);
PPE_RHIVULKAN_API VkCompareOp VkCast(ECompareOp value);
PPE_RHIVULKAN_API VkStencilOp VkCast(EStencilOp value);
PPE_RHIVULKAN_API VkPolygonMode VkCast(EPolygonMode value);
PPE_RHIVULKAN_API VkCullModeFlagBits VkCast(ECullMode value);
PPE_RHIVULKAN_API VkFilter VkCast(ETextureFilter value);
PPE_RHIVULKAN_API VkSamplerMipmapMode VkCast(EMipmapFilter value);
PPE_RHIVULKAN_API VkSamplerAddressMode VkCast(EAddressMode value);
PPE_RHIVULKAN_API VkBorderColor VkCast(EBorderColor value);
PPE_RHIVULKAN_API VkImageType VkCast(EImageDim value);
PPE_RHIVULKAN_API VkImageCreateFlagBits VkCast(EImageFlags value);
PPE_RHIVULKAN_API VkImageViewType VkCast(EImageView value);
PPE_RHIVULKAN_API VkImageUsageFlagBits VkCast(EImageUsage values);
PPE_RHIVULKAN_API VkImageAspectFlagBits VkCast(EImageAspect values);
PPE_RHIVULKAN_API VkImageAspectFlagBits VkCast(EImageAspect values, EPixelFormat format);
PPE_RHIVULKAN_API VkBufferUsageFlagBits VkCast(EBufferUsage values);
PPE_RHIVULKAN_API VkIndexType VkCast(EIndexFormat value);
PPE_RHIVULKAN_API VkPrimitiveTopology VkCast(EPrimitiveTopology value);
PPE_RHIVULKAN_API VkGeometryFlagBitsNV VkCast(ERayTracingGeometryFlags values);
PPE_RHIVULKAN_API VkGeometryInstanceFlagBitsNV VkCast(ERayTracingInstanceFlags values);
PPE_RHIVULKAN_API VkBuildAccelerationStructureFlagBitsNV VkCast(ERayTracingBuildFlags values);
PPE_RHIVULKAN_API VkColorSpaceKHR VkCast(EColorSpace value);
PPE_RHIVULKAN_API VkCompositeAlphaFlagBitsKHR VkCast(ECompositeAlpha value);
PPE_RHIVULKAN_API VkPresentModeKHR VkCast(EPresentMode value);
PPE_RHIVULKAN_API VkSurfaceTransformFlagBitsKHR VkCast(ESurfaceTransform value);
//----------------------------------------------------------------------------
// Vk to RHI:
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API EBufferUsage RHICast(VkBufferUsageFlagBits flags);
PPE_RHIVULKAN_API EImageDim RHICast(VkImageType value);
PPE_RHIVULKAN_API EImageFlags RHICast(VkImageCreateFlagBits value);
PPE_RHIVULKAN_API EImageUsage RHICast(VkImageUsageFlagBits flags);
PPE_RHIVULKAN_API FMultiSamples RHICast(VkSampleCountFlagBits flags);
PPE_RHIVULKAN_API EImageAspect RHICast(VkImageAspectFlagBits flags);
PPE_RHIVULKAN_API EColorSpace RHICast(VkColorSpaceKHR value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Resource state:
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API VkPipelineStageFlagBits EResourceState_ToPipelineStages(EResourceState value);
PPE_RHIVULKAN_API VkAccessFlagBits EResourceState_ToAccess(EResourceState value);
PPE_RHIVULKAN_API VkImageLayout EResourceState_ToImageLayout(EResourceState value, VkImageAspectFlags aspect);
PPE_RHIVULKAN_API VkShadingRatePaletteEntryNV VkCast(EShadingRatePalette value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Pixel format:
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API VkFormat VkCast(EPixelFormat value);
PPE_RHIVULKAN_API EPixelFormat RHICast(VkFormat value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
