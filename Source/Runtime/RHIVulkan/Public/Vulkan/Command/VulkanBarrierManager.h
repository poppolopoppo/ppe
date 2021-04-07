#pragma once

#include "Vulkan/VulkanCommon.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanBarrierManager final : public FRefCountable {
public:
    using FImageMemoryBarriers = VECTORMINSIZE(RHICommand, VkImageMemoryBarrier, 32);
    using FBufferMemoryBarriers = VECTORMINSIZE(RHICommand, VkBufferMemoryBarrier, 64);

    FVulkanBarrierManager() : _memoryBarrier{} {
        _memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    }

    void Commit(const FVulkanDevice& device, VkCommandBuffer cmd)
    void ForceCommit(const FVulkanDevice& device, VkCommandBuffer cmd, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);

    void ClearBarriers();

    void AddBufferBarrier(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages, const VkBufferMemoryBarrier& barrier);
    void AddImageBarrier(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages, VkDependencyFlags dependencies, const VkImageMemoryBarrier& barrier);
    void AddMemoryBarrier(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages, const VkMemoryBarrier& barrier);

private:
    FImageMemoryBarriers _imageBarriers;
    FBufferMemoryBarriers _bufferBarriers;
    VkMemoryBarrier _memoryBarrier;

    VkPipelineStageFlags _srcStages{ Default };
    VkPipelineStageFlags _dstStages{ Default };
    VkDependencyFlags _dependencies{ Default };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
