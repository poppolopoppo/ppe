#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Container/RingBuffer.h"
#include "Thread/CriticalSection.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanCommandPool final {
public:
    using FFreePool = TFixedSizeRingBuffer<VkCommandBuffer, 32>;

    FVulkanCommandPool() = default;
    ~FVulkanCommandPool();

    bool Valid() const { return (!!_pool); }

    NODISCARD bool Construct(const FVulkanDevice& device, const PVulkanDeviceQueue& queue ARGS_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(const FVulkanDevice& device);

    VkCommandBuffer AllocPrimary(const FVulkanDevice& device);
    VkCommandBuffer AllocSecondary(const FVulkanDevice& device);

    void Deallocate(const FVulkanDevice& device, VkCommandBuffer cmd);

    void ResetAll(const FVulkanDevice& device, VkCommandPoolResetFlags flags);
    void TrimAll(const FVulkanDevice& device, VkCommandPoolTrimFlags flags);

    void ResetBuffer(const FVulkanDevice& device, VkCommandBuffer cmd, VkCommandBufferResetFlags flags);

    void RecyclePrimary(VkCommandBuffer cmd);
    void RecycleSecondary(VkCommandBuffer cmd);

private:
    VkCommandBuffer AllocBuffer_(FFreePool* pPool, const FVulkanDevice& device, VkCommandBufferLevel level);

    VkCommandPool _pool{ VK_NULL_HANDLE };

    FCriticalSection _poolCS;
    FFreePool _freePrimaries;
    FFreePool _freeSecondaries;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
    std::atomic<u32> _maxBuffersAllocated{ 0 };
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
