
#include "stdafx.h"

#include "Vulkan/Command/VulkanBarrierManager.h"

#include "Vulkan/Instance/VulkanDevice.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanBarrierManager::FVulkanBarrierManager() NOEXCEPT
:   _memoryBarrier{} {
    _memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
}
//----------------------------------------------------------------------------
void FVulkanBarrierManager::Commit(const FVulkanDevice& device, VkCommandBuffer cmd) {
    const u32 memCount = !!(_memoryBarrier.srcAccessMask | _memoryBarrier.dstAccessMask);

    if (memCount || not _bufferBarriers.empty() || not _imageBarriers.empty()) {
        Assert(_srcStages != Zero);
        Assert(_dstStages != Zero);

        device.vkCmdPipelineBarrier(cmd, _srcStages, _dstStages, _dependencies,
            memCount, &_memoryBarrier,
            static_cast<u32>(_bufferBarriers.size()), _bufferBarriers.data(),
            static_cast<u32>(_imageBarriers.size()), _imageBarriers.data() );

        ClearBarriers();
    }
}
//----------------------------------------------------------------------------
void FVulkanBarrierManager::ForceCommit(
    const FVulkanDevice& device,
    VkCommandBuffer cmd,
    VkPipelineStageFlags srcStage,
    VkPipelineStageFlags dstStage ) {
    const u32 memCount = !!(_memoryBarrier.srcAccessMask | _memoryBarrier.dstAccessMask);

    _srcStages |= srcStage;
    _dstStages |= dstStage;

    if (!!_srcStages && !!_dstStages) {
        device.vkCmdPipelineBarrier(cmd, _srcStages, _dstStages, _dependencies,
            memCount, &_memoryBarrier,
            static_cast<u32>(_bufferBarriers.size()), _bufferBarriers.data(),
            static_cast<u32>(_imageBarriers.size()), _imageBarriers.data() );

        ClearBarriers();
    }
}
//----------------------------------------------------------------------------
void FVulkanBarrierManager::ClearBarriers() {
    _imageBarriers.clear();
    _bufferBarriers.clear();

    _memoryBarrier.srcAccessMask = _memoryBarrier.dstAccessMask = 0;

    _srcStages = _dstStages = 0;
    _dependencies = 0;
}
//----------------------------------------------------------------------------
void FVulkanBarrierManager::AddBufferBarrier(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages, const VkBufferMemoryBarrier& barrier) {
    _srcStages |= srcStages;
    _dstStages |= dstStages;
    _bufferBarriers.push_back(barrier);
}
//----------------------------------------------------------------------------
void FVulkanBarrierManager::AddImageBarrier(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages, VkDependencyFlags dependencies, const VkImageMemoryBarrier& barrier) {
    _srcStages |= srcStages;
    _dstStages |= dstStages;
    _dependencies |= dependencies;
    _imageBarriers.push_back(barrier);
}
//----------------------------------------------------------------------------
void FVulkanBarrierManager::AddMemoryBarrier(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages, const VkMemoryBarrier& barrier) {
    Assert(nullptr == barrier.pNext);

    _srcStages |= srcStages;
    _dstStages |= dstStages;
    _memoryBarrier.srcAccessMask |= barrier.srcAccessMask;
    _memoryBarrier.dstAccessMask |= barrier.dstAccessMask;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
