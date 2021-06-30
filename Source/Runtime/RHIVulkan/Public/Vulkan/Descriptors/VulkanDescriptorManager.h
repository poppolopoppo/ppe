#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Descriptors/VulkanDescriptorSetLayout.h"

#include "Thread/CriticalSection.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanDescriptorManager final : public FRefCountable {
public:
    STATIC_CONST_INTEGRAL(u32, MaxDescriptorSets, 1u << 10);
    STATIC_CONST_INTEGRAL(u32, MaxDescriptorPoolSize, 1u << 11);

    using FDescriptorPools = TFixedSizeStack<VkDescriptorPool, 8>;

    explicit FVulkanDescriptorManager(const FVulkanDevice& device) NOEXCEPT;
    ~FVulkanDescriptorManager();

    NODISCARD bool Construct();
    void TearDown();

    NODISCARD bool AllocateDescriptorSet(FVulkanDescriptorSet* pDescriptors, VkDescriptorSetLayout layout);
    void DeallocateDescriptorSet(const FVulkanDescriptorSet& descriptors);
    void DeallocateDescriptorSets(TMemoryView<const FVulkanDescriptorSet> many);

private:
    NODISCARD VkDescriptorPool CreateDescriptorPool_(FDescriptorPools& pools);

    const FVulkanDevice& _device;

    TThreadSafe<FDescriptorPools, EThreadBarrier::CriticalSection> _descriptorPools;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
