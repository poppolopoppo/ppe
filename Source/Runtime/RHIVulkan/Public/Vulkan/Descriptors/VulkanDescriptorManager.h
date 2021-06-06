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

    bool Construct();
    void TearDown();

    bool AllocateDescriptorSet(FVulkanDescriptorSet* pdescriptors, VkDescriptorSetLayout layout);
    void DeallocateDescriptorSet(const FVulkanDescriptorSet& descriptors);
    void DeallocateDescriptorSets(TMemoryView<const FVulkanDescriptorSet> many);

private:
    void CreateDescriptorPool_();

    const FVulkanDevice& _device;

    FCriticalSection _barrier;
    FDescriptorPools _descriptorPools;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
