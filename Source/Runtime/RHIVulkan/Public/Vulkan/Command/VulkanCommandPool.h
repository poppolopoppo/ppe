#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Thread/MPMCBoundedQueue.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanCommandPool final : public FRefCountable {
public:
    using FFreePool = TMPMCFixedSizeQueue<VkCommandBuffer, 32>;

    FVulkanCommandPool() = default;
    ~FVulkanCommandPool();

    bool Valid() const { return (!!_pool); }

    bool Create(const FVulkanDevice& device, const PVulkanDeviceQueue& queue ARGS_IF_RHIDEBUG(FStringView dbgName = Default));
    void TearDown(const FVulkanDevice& device);

    VkCommandBuffer AllocPrimary(const FVulkanDevice& device);
    VkCommandBuffer AllocSecondary(const FVulkanDevice& device);

    void Deallocate(const FVulkanDevice& device, VkCommandBuffer cmd);

    void ResetAll(const FVulkanDevice& device, VkCommandPoolResetFlags flags);
    void TrimAll(const FVulkanDevice& device, VkCommandPoolTrimFlags flags);

    void Reset(const FVulkanDevice& device, VkCommandBuffer cmd, VkCommandBufferResetFlags flags);

    void RecyclePrimary(VkCommandBuffer cmd) const;
    void RecycleSecondary(VkCommandBuffer cmd) const;

private:
    std::atomic<VkCommandPool> _pool{ VK_NULL_HANDLE };

#if USE_PPE_RHIDEBUG
    std::atomic<u32> _liveCount{ 0 };
#endif

    FFreePool _primaries;
    FFreePool _secondaries;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
