#pragma once

#include "Vulkan/Vulkan_fwd.h"

#ifdef RHI_VULKAN

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
enum class EVulkanMemoryTypeFlags : u32 {
    None          = 0,
    DeviceLocal   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // sit in GPU VRam, no CPU access
    HostVisible   = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, // CPU needs to write GPU data
    HostCoherent  = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // no need to flush CPU<->GPU to make visible
    HostCached    = VK_MEMORY_PROPERTY_HOST_CACHED_BIT, // CPU needs to read-back GPU data
};
ENUM_FLAGS(EVulkanMemoryTypeFlags);
//----------------------------------------------------------------------------
struct FVulkanMemoryBlock {
    FVulkanDeviceMemory DeviceMemory;
    u32 Size;
    u32 MemoryType;
    EVulkanMemoryTypeFlags Flags;
};
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanMemoryAllocator {
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

    void CreateDeviceHeaps();
    void DestroyDeviceHeaps();

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

    const FVulkanDevice& _device;

    ARRAYINSITU(RHIDevice, FMemoryType, 4) _memoryTypes;
    ARRAYINSITU(RHIDevice, FMemoryHeap, 4) _memoryHeaps;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
