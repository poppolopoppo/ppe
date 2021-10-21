
#include "stdafx.h"

#include "Vulkan/Descriptors/VulkanDescriptorManager.h"

#include "Vulkan/Instance/VulkanDevice.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanDescriptorManager::FVulkanDescriptorManager(const FVulkanDevice& device) NOEXCEPT
:   _device(device) {
}
//----------------------------------------------------------------------------
FVulkanDescriptorManager::~FVulkanDescriptorManager() {
    Assert_NoAssume(_descriptorPools.LockExclusive()->empty());
}
//----------------------------------------------------------------------------
bool FVulkanDescriptorManager::Construct() {
    const auto exclusivePools = _descriptorPools.LockExclusive();
    Assert_NoAssume(exclusivePools->empty());

    LOG_CHECK(RHI, !!CreateDescriptorPool_(exclusivePools.Value()) );

    return true;
}
//----------------------------------------------------------------------------
void FVulkanDescriptorManager::TearDown() {
    const auto exclusivePools = _descriptorPools.LockExclusive();

    for (VkDescriptorPool vkPool : exclusivePools.Value()) {
        if (VK_NULL_HANDLE != vkPool)
            _device.vkDestroyDescriptorPool(_device.vkDevice(), vkPool, _device.vkAllocator());
    }

    exclusivePools->clear();
}
//----------------------------------------------------------------------------
bool FVulkanDescriptorManager::AllocateDescriptorSet(FVulkanDescriptorSet* pDescriptors, VkDescriptorSetLayout layout) {
    Assert(pDescriptors);
    Assert(VK_NULL_HANDLE != layout);

    VkDescriptorSetAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &layout;

    const auto exclusivePools = _descriptorPools.LockExclusive();

    forrange(i, 0, exclusivePools->size()) {
        const VkDescriptorPool vkPool = exclusivePools->at(i);
        Assert(VK_NULL_HANDLE != vkPool);

        info.descriptorPool = vkPool;

        if (_device.vkAllocateDescriptorSets(_device.vkDevice(), &info, &pDescriptors->First) == VK_SUCCESS ) {
            Assert_NoAssume(VK_NULL_HANDLE != pDescriptors->First);
            pDescriptors->IndexInPool = checked_cast<u8>(i);
            return true;
        }
    }

    info.descriptorPool = CreateDescriptorPool_(*exclusivePools);
    LOG_CHECK(RHI, info.descriptorPool );

    VK_CHECK( _device.vkAllocateDescriptorSets(_device.vkDevice(), &info, &pDescriptors->First) );

    Assert_NoAssume(VK_NULL_HANDLE != pDescriptors->First);
    pDescriptors->IndexInPool = checked_cast<u8>(exclusivePools->size() - 1);
    return true;
}
//----------------------------------------------------------------------------
void FVulkanDescriptorManager::DeallocateDescriptorSet(const FVulkanDescriptorSet& descriptors) {
    const auto exclusivePools = _descriptorPools.LockExclusive();
    AssertRelease(descriptors.IndexInPool < exclusivePools->size());

    VK_CALL( _device.vkFreeDescriptorSets(
        _device.vkDevice(),
        exclusivePools->at(descriptors.IndexInPool),
        1, &descriptors.First ) );
}
//----------------------------------------------------------------------------
void FVulkanDescriptorManager::DeallocateDescriptorSets(TMemoryView<const FVulkanDescriptorSet> many) {
    const auto exclusivePools = _descriptorPools.LockExclusive();

    TFixedSizeStack<VkDescriptorSet, 16> batch;
    u8 lastIndex = UMax;

    for (const FVulkanDescriptorSet& ds : many) {
        if (lastIndex != ds.IndexInPool && not batch.empty()) {
            VK_CALL( _device.vkFreeDescriptorSets(
                _device.vkDevice(),
                exclusivePools->at(lastIndex),
                checked_cast<u32>(batch.size()), batch.data() ) );

            batch.clear();
        }

        lastIndex = ds.IndexInPool;
        batch.Push(ds.First);
    }

    if (not batch.empty()) {
        VK_CALL( _device.vkFreeDescriptorSets(
            _device.vkDevice(),
            exclusivePools->at(lastIndex),
            checked_cast<u32>(batch.size()), batch.data() ) );
    }
}
//----------------------------------------------------------------------------
VkDescriptorPool FVulkanDescriptorManager::CreateDescriptorPool_(FDescriptorPools& pools) {
    LOG_CHECK(RHI, pools.size() < pools.capacity());

    TFixedSizeStack<VkDescriptorPoolSize, 16> poolSizes;

    poolSizes.Push(VK_DESCRIPTOR_TYPE_SAMPLER, MaxDescriptorPoolSize);
    poolSizes.Push(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MaxDescriptorPoolSize * 4);
    poolSizes.Push(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MaxDescriptorPoolSize);
    poolSizes.Push(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, MaxDescriptorPoolSize);

    poolSizes.Push(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MaxDescriptorPoolSize * 4);
    poolSizes.Push(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MaxDescriptorPoolSize * 2);
    poolSizes.Push(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MaxDescriptorPoolSize);
    poolSizes.Push(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MaxDescriptorPoolSize);

    if (_device.Enabled().RayTracingNV)
        poolSizes.Push(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, MaxDescriptorPoolSize);
    else if (_device.Enabled().RayTracingKHR)
        poolSizes.Push(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, MaxDescriptorPoolSize);

    VkDescriptorPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.poolSizeCount = checked_cast<u32>(poolSizes.size());
    info.pPoolSizes = poolSizes.data();
    info.maxSets = MaxDescriptorSets;
    info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VkDescriptorPool vkDescriptorPool;
    VK_CHECK( _device.vkCreateDescriptorPool(
        _device.vkDevice(),
        &info,
        _device.vkAllocator(),
        &vkDescriptorPool ) );

    Assert_NoAssume(VK_NULL_HANDLE != vkDescriptorPool);
    pools.Push(vkDescriptorPool);
    return vkDescriptorPool;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
