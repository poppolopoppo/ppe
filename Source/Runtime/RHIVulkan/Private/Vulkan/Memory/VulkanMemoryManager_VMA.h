﻿#pragma once

#include "Vulkan/Memory/VulkanMemoryManager.h"

#ifdef USE_PPE_RHIVMA

#include "Vulkan/Instance/VulkanDevice.h"

#include "vma-external.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVulkanMemoryManager::FVulkanMemoryAllocator final : public IMemoryAllocator {
public:
    FVulkanMemoryAllocator(const FVulkanDevice& device, EVulkanMemoryType memoryType);
    ~FVulkanMemoryAllocator() override;

    void DutyCycle(u32 frameIndex) override;
    void DefragmentMemory(FVulkanResourceManager& resources) override;

    bool IsSupported(EMemoryType ) const NOEXCEPT override { return true; }

    NODISCARD bool AllocateImage(FBlock* pData, VkImage image, const FMemoryDesc& desc) override;
    NODISCARD bool AllocateBuffer(FBlock* pData, VkBuffer buffer, const FMemoryDesc& desc) override;
    NODISCARD bool AllocateAccelStruct(FBlock* pData, VkAccelerationStructureKHR accelStruct, const FMemoryDesc& desc) override;

    void Deallocate(FBlock& data) override;

    NODISCARD bool MemoryInfo(FVulkanMemoryInfo* pInfo, const FBlock& data) const NOEXCEPT override;

private:
    struct FData {
        VmaAllocation Allocation;
    };

    NODISCARD static VmaAllocationCreateFlags ConvertToMemoryFlags_(EMemoryType memoryType);
    NODISCARD static VmaMemoryUsage ConvertToMemoryUsage_(EMemoryType memoryType);
    NODISCARD static VkMemoryPropertyFlags ConvertToMemoryProperties_(EMemoryType memoryType);

    const FVulkanDevice& _device;
    TThreadSafe<VmaAllocator, EThreadBarrier::RWLock> _allocator;
};
//----------------------------------------------------------------------------
inline FVulkanMemoryManager::FVulkanMemoryAllocator::FVulkanMemoryAllocator(const FVulkanDevice& device, EVulkanMemoryType memoryType)
:   _device(device)
,   _allocator() {
    const auto exclusiveAllocator = _allocator.LockExclusive();

    VmaVulkanFunctions funcs = {};
    funcs.vkGetPhysicalDeviceProperties = device.api()->instance_api_->vkGetPhysicalDeviceProperties;
    funcs.vkGetPhysicalDeviceMemoryProperties = device.api()->instance_api_->vkGetPhysicalDeviceMemoryProperties;
    funcs.vkAllocateMemory = device.api()->vkAllocateMemory;
    funcs.vkFreeMemory = device.api()->vkFreeMemory;
    funcs.vkMapMemory = device.api()->vkMapMemory;
    funcs.vkUnmapMemory = device.api()->vkUnmapMemory;
    funcs.vkFlushMappedMemoryRanges = device.api()->vkFlushMappedMemoryRanges;
    funcs.vkInvalidateMappedMemoryRanges = device.api()->vkInvalidateMappedMemoryRanges;
    funcs.vkBindBufferMemory = device.api()->vkBindBufferMemory;
    funcs.vkBindImageMemory = device.api()->vkBindImageMemory;
    funcs.vkGetBufferMemoryRequirements = device.api()->vkGetBufferMemoryRequirements;
    funcs.vkGetImageMemoryRequirements = device.api()->vkGetImageMemoryRequirements;
    funcs.vkCreateBuffer = device.api()->vkCreateBuffer;
    funcs.vkDestroyBuffer = device.api()->vkDestroyBuffer;
    funcs.vkCreateImage = device.api()->vkCreateImage;
    funcs.vkDestroyImage = device.api()->vkDestroyImage;
    funcs.vkCmdCopyBuffer = device.api()->vkCmdCopyBuffer;

    VmaAllocatorCreateFlags flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;

#if VMA_DEDICATED_ALLOCATION
    if (device.Enabled().DedicatedAllocation) {
        if (device.vkVersion() == EShaderLangFormat::Vulkan_100) {
            funcs.vkGetBufferMemoryRequirements2KHR = device.api()->vkGetBufferMemoryRequirements2KHR;
            funcs.vkGetImageMemoryRequirements2KHR = device.api()->vkGetImageMemoryRequirements2KHR;
        }
        else {
            funcs.vkGetBufferMemoryRequirements2KHR = device.api()->vkGetBufferMemoryRequirements2;
            funcs.vkGetImageMemoryRequirements2KHR = device.api()->vkGetImageMemoryRequirements2;
        }
        flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
    }
#endif
#if VMA_BIND_MEMORY2
    if (device.Enabled().BindMemory2) {
        if (device.vkVersion() == EShaderLangFormat::Vulkan_100) {
            funcs.vkBindBufferMemory2KHR = device.api()->vkBindBufferMemory2KHR;
            funcs.vkBindImageMemory2KHR = device.api()->vkBindImageMemory2KHR;
        }
        else {
            funcs.vkBindBufferMemory2KHR = device.api()->vkBindBufferMemory2;
            funcs.vkBindImageMemory2KHR = device.api()->vkBindImageMemory2;
        }
        flags |= VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT ;
    }
#endif
#if VMA_MEMORY_BUDGET
    if (device.Enabled().MemoryBudget) {
        if (device.vkVersion() == EShaderLangFormat::Vulkan_100) {
            funcs.vkGetPhysicalDeviceMemoryProperties2KHR = device.api()->instance_api_->vkGetPhysicalDeviceMemoryProperties2KHR;
        }
        else {
            funcs.vkGetPhysicalDeviceMemoryProperties2KHR = device.api()->instance_api_->vkGetPhysicalDeviceMemoryProperties2;
        }
        flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    }
#endif

    VmaAllocatorCreateInfo info{};
    info.flags = flags;
    info.physicalDevice = device.vkPhysicalDevice();
    info.device = device.vkDevice();
    info.pAllocationCallbacks = device.vkAllocator();
    info.preferredLargeHeapBlockSize = (static_cast<VkDeviceSize>(GPUPageSizeInMb) << 20);
    info.pDeviceMemoryCallbacks = nullptr;
    //info.frameInUseCount // ignore
    info.pHeapSizeLimit = nullptr; // #TODO
    info.pVulkanFunctions = &funcs;

    VK_CALL( vmaCreateAllocator(&info, &exclusiveAllocator.Value()) );
}
//----------------------------------------------------------------------------
inline FVulkanMemoryManager::FVulkanMemoryAllocator::~FVulkanMemoryAllocator() {
    vmaDestroyAllocator(_allocator.LockExclusive().Value());
}
//----------------------------------------------------------------------------
inline void FVulkanMemoryManager::FVulkanMemoryAllocator::DutyCycle(u32 frameIndex) {
    vmaSetCurrentFrameIndex(_allocator.LockExclusive().Value(), frameIndex);
}
//----------------------------------------------------------------------------
inline void FVulkanMemoryManager::FVulkanMemoryAllocator::DefragmentMemory(FVulkanResourceManager& resources) {

    resources.
}
//----------------------------------------------------------------------------
inline bool FVulkanMemoryManager::FVulkanMemoryAllocator::AllocateImage(FBlock* pData, VkImage image, const FMemoryDesc& desc) {
    const auto exclusiveAllocator = _allocator.LockExclusive();

    VmaAllocationCreateInfo info{};
    info.flags = ConvertToMemoryFlags_(desc.Type);
    info.usage = ConvertToMemoryUsage_(desc.Type);
    info.requiredFlags = ConvertToMemoryProperties_(desc.Type);
    info.preferredFlags = 0;
    info.memoryTypeBits = 0;
    info.pool = VK_NULL_HANDLE;
    info.pUserData = nullptr;

    VmaAllocation allocation = nullptr;

    if (desc.ExternalRequirements.has_value()) {
        // because using private api
        VMA_DEBUG_GLOBAL_MUTEX_LOCK

        VkMemoryRequirements vkMemoryRequirements{};
        bool requireDedicatedAllocation = false;
        bool prefersDedicatedAllocation = false;
        exclusiveAllocator->GetImageMemoryRequirements(image, vkMemoryRequirements, requireDedicatedAllocation, prefersDedicatedAllocation);

        vkMemoryRequirements.alignment = desc.ExternalRequirements->Alignment;
        vkMemoryRequirements.memoryTypeBits &= (desc.ExternalRequirements->MemoryTypeBits ? desc.ExternalRequirements->MemoryTypeBits : ~0u);

        Assert_NoAssume(!!vkMemoryRequirements.memoryTypeBits);

        VK_CHECK(exclusiveAllocator->AllocateMemory(
            vkMemoryRequirements,
            requireDedicatedAllocation,
            prefersDedicatedAllocation,
            VK_NULL_HANDLE,
            UINT32_MAX,
            image, info,
            VMA_SUBALLOCATION_TYPE_IMAGE_UNKNOWN,
            1, &allocation ));
    }
    else {
        VK_CHECK(vmaAllocateMemoryForImage(
            exclusiveAllocator.Value(),
            image, &info, &allocation, nullptr ));
    }

    VK_CHECK(vmaBindImageMemory(exclusiveAllocator.Value(), allocation, image ));

    pData->MemoryHandle = allocation;
    return true;
}
//----------------------------------------------------------------------------
inline bool FVulkanMemoryManager::FVulkanMemoryAllocator::AllocateBuffer(FBlock* pData, VkBuffer buffer, const FMemoryDesc& desc) {
    const auto exclusiveAllocator = _allocator.LockExclusive();

    VmaAllocationCreateInfo info{};
    info.flags = ConvertToMemoryFlags_(desc.Type);
    info.usage = ConvertToMemoryUsage_(desc.Type);
    info.requiredFlags = ConvertToMemoryProperties_(desc.Type);
    info.preferredFlags = 0;
    info.memoryTypeBits = 0;
    info.pool = VK_NULL_HANDLE;
    info.pUserData = nullptr;

    VmaAllocation allocation = nullptr;

    if (desc.ExternalRequirements.has_value()) {
        // because using private api
        VMA_DEBUG_GLOBAL_MUTEX_LOCK

        VkMemoryRequirements vkMemoryRequirements{};
        bool requireDedicatedAllocation = false;
        bool prefersDedicatedAllocation = false;
        exclusiveAllocator->GetBufferMemoryRequirements(buffer, vkMemoryRequirements, requireDedicatedAllocation, prefersDedicatedAllocation);

        vkMemoryRequirements.alignment = desc.ExternalRequirements->Alignment;
        vkMemoryRequirements.memoryTypeBits &= (desc.ExternalRequirements->MemoryTypeBits ? desc.ExternalRequirements->MemoryTypeBits : ~0u);

        Assert_NoAssume(!!vkMemoryRequirements.memoryTypeBits);

        VK_CHECK(exclusiveAllocator->AllocateMemory(
            vkMemoryRequirements,
            requireDedicatedAllocation,
            prefersDedicatedAllocation,
            buffer,
            UINT32_MAX, // #TODO ?
            VK_NULL_HANDLE, info,
            VMA_SUBALLOCATION_TYPE_BUFFER,
            1, &allocation ));
    }
    else {
        VK_CHECK(vmaAllocateMemoryForBuffer(
            exclusiveAllocator.Value(),
            buffer, &info, &allocation, nullptr ));
    }

    VK_CHECK(vmaBindBufferMemory(exclusiveAllocator.Value(), allocation, buffer ));

    pData->MemoryHandle = allocation;
    return true;
}
//----------------------------------------------------------------------------
inline bool FVulkanMemoryManager::FVulkanMemoryAllocator::AllocateAccelStruct(FBlock* pData, VkAccelerationStructureKHR accelStruct, const FMemoryDesc& desc) {
    const auto exclusiveAllocator = _allocator.LockExclusive();

    UNUSED(pData);
    UNUSED(accelStruct);
    UNUSED(desc);

    // #TODO: VK_KHR_ray_tracing_pipeline

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
inline void FVulkanMemoryManager::FVulkanMemoryAllocator::Deallocate(FBlock& data) {
    const auto exclusiveAllocator = _allocator.LockExclusive();

    vmaFreeMemory(exclusiveAllocator.Value(), static_cast<VmaAllocation>(data.MemoryHandle));

    data.MemoryHandle = nullptr;
}
//----------------------------------------------------------------------------
inline bool FVulkanMemoryManager::FVulkanMemoryAllocator::MemoryInfo(FVulkanMemoryInfo* pInfo, const FBlock& data) const noexcept {
    Assert(pInfo);

    const auto sharedData = _allocator.LockShared();

    const VmaAllocation mem = static_cast<VmaAllocation>(data.MemoryHandle);
    if (mem == nullptr)
        return false;

    VmaAllocationInfo info{};
    vmaGetAllocationInfo(sharedData.Value(), mem, &info);

    const auto& props = _device.Capabilities().MemoryProperties;
    Assert(info.memoryType < props.memoryTypeCount);

    if (pInfo) {
        pInfo->Memory = info.deviceMemory;
        pInfo->Flags = props.memoryTypes[info.memoryType].propertyFlags;
        pInfo->Offset = checked_cast<u32>(info.offset);
        pInfo->Size = checked_cast<u32>(info.size);
        pInfo->MappedPtr = static_cast<ubyte*>(info.pMappedData);
    }

    return true;
}
//----------------------------------------------------------------------------
inline VmaAllocationCreateFlags FVulkanMemoryManager::FVulkanMemoryAllocator::ConvertToMemoryFlags_(EMemoryType memoryType) {
    VmaAllocationCreateFlags result = Zero;

    if (memoryType & EMemoryType::Dedicated)
        result |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    if ((memoryType & EMemoryType::HostRead) or (memoryType & EMemoryType::HostWrite))
        result |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

    // #TODO: VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT

    return result;
}
//----------------------------------------------------------------------------
inline VmaMemoryUsage FVulkanMemoryManager::FVulkanMemoryAllocator::ConvertToMemoryUsage_(EMemoryType memoryType) {
    if (memoryType & EMemoryType::HostRead)
        return VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_TO_CPU;

    if (memoryType & EMemoryType::HostWrite)
        return VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;

    return VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
}
//----------------------------------------------------------------------------
inline VkMemoryPropertyFlags FVulkanMemoryManager::FVulkanMemoryAllocator::ConvertToMemoryProperties_(EMemoryType memoryType) {
    const EVulkanMemoryType values = static_cast<EVulkanMemoryType>(memoryType);

    VkMemoryPropertyFlags flags = 0;

    for (u32 t = 1; t < static_cast<u32>(EVulkanMemoryType::_Last); t <<= 1) {
        if (not Meta::EnumHas(values, t))
            continue;

        switch (static_cast<EVulkanMemoryType>(t)) {
        case EVulkanMemoryType::HostRead: flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; break;
        case EVulkanMemoryType::HostWrite: flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; break;
        case EVulkanMemoryType::LocalInGPU: flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; break;
        case EVulkanMemoryType::HostCoherent: flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; break;
        case EVulkanMemoryType::HostCached: flags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT; break;
        case EVulkanMemoryType::Dedicated:
        case EVulkanMemoryType::ForBuffer:
        case EVulkanMemoryType::ForImage: break;
        case EVulkanMemoryType::Virtual:
        case EVulkanMemoryType::HostVisible: AssertNotImplemented();
        case EVulkanMemoryType::All:
        case EVulkanMemoryType::_Last: AssertNotReached();
        }
    }

    return flags;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIVMA