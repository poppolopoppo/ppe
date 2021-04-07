#pragma once

#include "Vulkan/Vulkan_fwd.h"

#include "Vulkan/Common/VulkanDebug.h"
#include "Vulkan/Common/VulkanEnums.h"
#include "Vulkan/Common/VulkanError.h"

#include "RHI/ResourceId.h"
#include "RHI/ResourceTypes.h"

#include "Container/Stack.h"
#include "Maths/ScalarMatrix.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(u64, FVulkanBLASHandle);
//----------------------------------------------------------------------------
struct FVulkanGeometryInstance {
    float4x3 Transform;

    u32 CustomIndex     : 24;
    u32 Mask            :  8;

    u32 InstanceOffset  : 24;
    u32 Flags           :  8;

    FVulkanBLASHandle BlasHandle;
};
//----------------------------------------------------------------------------
using FVulkanDescriptorSets = TFixedSizeStack<VkDescriptorSet, MaxDescriptorSets>;
//----------------------------------------------------------------------------
struct FVulkanPipelineResourceSet {
    struct FResource {
        FDescriptorSetID DescriptorSetId;
        const FVulkanPipelineResources* PipelineResources;
        u32 OffsetIndex{ UMax };
        u32 OffsetCount{ 0 };
    };

    TFixedSizeStack<FResource, MaxDescriptorSets> Resources;
    mutable TFixedSizeStack<u32, MaxBufferDynamicOffsets> DynamicOffsets;
};
//----------------------------------------------------------------------------
using PVulkanShaderModule = PShaderData<VkShaderModule>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
