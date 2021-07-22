
#include "stdafx.h"

#include "Vulkan/Command/VulkanSubmitted.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Command/VulkanCommandBatch.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanSubmitted::FVulkanSubmitted(u32 indexInPool) NOEXCEPT
:   _indexInPool(indexInPool)
{
}
//----------------------------------------------------------------------------
FVulkanSubmitted::~FVulkanSubmitted() {
#if USE_PPE_ASSERT
    const auto exclusive = _submit.LockExclusive();

    Assert_NoAssume(VK_NULL_HANDLE != exclusive->Fence);
    Assert_NoAssume(exclusive->Batches.empty());
    Assert_NoAssume(exclusive->Semaphores.empty());
#endif
}
//----------------------------------------------------------------------------
void FVulkanSubmitted::Construct(
    const FVulkanDevice& device,
    EQueueType queue,
    TMemoryView<const PVulkanCommandBatch> batches,
    TMemoryView<const VkSemaphore> semaphores ) {
    Assert_NoAssume(static_cast<u32>(queue) < static_cast<u32>(EQueueType::_Count));

    const auto exclusive = _submit.LockExclusive();

    if (not exclusive->Fence) {
        VkFenceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = 0;
        VK_CALL( device.vkCreateFence(device.vkDevice(), &info, device.vkAllocator(), &exclusive->Fence) );
    }
    else {
        VK_CALL( device.vkResetFences(device.vkDevice(), 1, &exclusive->Fence) );
    }

    exclusive->QueueType = queue;
    exclusive->Batches.Assign(batches);
    exclusive->Semaphores.Assign(semaphores);
}
//----------------------------------------------------------------------------
void FVulkanSubmitted::Release(
    const FVulkanDevice& device
    ARGS_IF_RHIDEBUG(FFrameStatistics* pStats, FVulkanDebugger& debugger, const FShaderDebugCallback& callback) ) {

    const auto exclusive = _submit.LockExclusive();

    for (VkSemaphore semaphore : exclusive->Semaphores)
        device.vkDestroySemaphore(device.vkDevice(), semaphore, device.vkAllocator());

    exclusive->Semaphores.clear();

    for (const PVulkanCommandBatch& pBatch : exclusive->Batches)
        pBatch->OnComplete(ARG0_IF_RHIDEBUG(pStats, debugger, std::move(callback)));

    exclusive->Batches.clear();
}
//----------------------------------------------------------------------------
void FVulkanSubmitted::TearDown(const FVulkanDevice& device) {
    const auto exclusive = _submit.LockExclusive();

    device.vkDestroyFence(device.vkDevice(), exclusive->Fence, device.vkAllocator());
    exclusive->Fence = VK_NULL_HANDLE;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
