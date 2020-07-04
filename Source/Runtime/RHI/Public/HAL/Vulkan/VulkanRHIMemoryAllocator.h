#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHIMemoryAllocator.h"

#include "Allocator/MipMapAllocator.h"
#include "Container/Array.h"

#if USE_PPE_MEMORYDOMAINS
#    include "IO/String.h"
#    include "Memory/MemoryTracking.h"
#endif

#include <atomic>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanMemoryBlock {
    FVulkanDeviceMemory DeviceMemory;
    u32 Size;
    u32 MemoryType;
    EVulkanMemoryTypeFlags Flags;
};
//----------------------------------------------------------------------------
class PPE_RHI_API FVulkanMemoryAllocator : public FGenericMemoryAllocator {
public: // must be defined by every RHI:

    FVulkanMemoryBlock Allocate(
        u32 size,
        u32 memoryTypeBits,
        EVulkanMemoryTypeFlags requiredFlags,
        EVulkanMemoryTypeFlags preferredFlags = EVulkanMemoryTypeFlags::None );

    void Deallocate(const FVulkanMemoryBlock& block) NOEXCEPT;

    FRawMemory MapMemory(const FVulkanMemoryBlock& block, u32 offset = 0, u32 size = 0);
    void UnmapMemory(const FVulkanMemoryBlock& block) NOEXCEPT;

public: // vulkan specific:
    explicit FVulkanMemoryAllocator(const FVulkanDevice& device) NOEXCEPT;
    ~FVulkanMemoryAllocator();

private:
    struct FMemoryType {
        u32 HeapIndex;
        EVulkanMemoryTypeFlags Flags;
#if USE_PPE_MEMORYDOMAINS
        FString MemoryName;
        mutable FMemoryTracking TrackingData;
        FMemoryType(u32 heapIndex, EVulkanMemoryTypeFlags flags,
            u32 memoryIndex, FMemoryTracking* parent );
#else
        FMemoryType(u32 heapIndex, EVulkanMemoryTypeFlags flags) NOEXCEPT
            : HeapIndex(heapIndex), Flags(flags)
        {}
#endif
    };

    struct FMemoryHeap {
        mutable std::atomic<u32> NumAllocations;

        u32 Granularity;
        VkDeviceSize Capacity;
#if USE_PPE_MEMORYDOMAINS
        FString HeapName;
        mutable FMemoryTracking TrackingData;
        FMemoryHeap(u32 granularity, VkDeviceSize capacity,
            u32 heapIndex, FMemoryTracking* parent );
#else
        FMemoryHeap(u32 granularity, VkDeviceSize capacity) NOEXCEPT
            : NumAllocations(0), Granularity(granularity), Capacity(capacity)
        {}
#endif
    };

    VkDevice _vkDevice;
    FVulkanAllocationCallbacks _vkAllocator;

    ARRAYINSITU(RHIDevice, FMemoryType, 4) _memoryTypes;
    ARRAYINSITU(RHIDevice, FMemoryHeap, 4) _memoryHeaps;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
