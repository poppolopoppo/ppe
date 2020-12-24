#include "stdafx.h"

#include "HAL/Vulkan/VulkanRHIMemoryAllocator.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanError.h"
#include "HAL/Vulkan/VulkanRHIIncludes.h"
#include "HAL/Vulkan/VulkanRHIInstance.h"
#include "HAL/Vulkan/VulkanRHIDevice.h"

#include "HAL/PlatformMaths.h"
#include "HAL/PlatformProcess.h"

#if USE_PPE_MEMORYDOMAINS
#    include "IO/Format.h"
#    include "IO/FormatHelpers.h"
#    include "IO/StringBuilder.h"
#endif

#define USE_PPE_VKOVERSUBSCRIBING_WORKAROUND 1

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(u32, GVulkanMaxGranularity, 256*1024*1024);
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
FVulkanMemoryAllocator::FMemoryType::FMemoryType(
    u32 heapIndex, EVulkanMemoryTypeFlags flags,
    u32 memoryIndex, FMemoryTracking* parent )
:   HeapIndex(heapIndex)
,   Flags(flags)
,   MemoryName(StringFormat("TYPE#{0}{1}{2}", memoryIndex,
        Fmt::Conditional("_DEVICE", flags ^ EVulkanMemoryTypeFlags::DeviceLocal),
        Fmt::Conditional("_HOST",  flags ^ EVulkanMemoryTypeFlags::HostVisible),
        Fmt::Conditional("_CACHED",  flags ^ EVulkanMemoryTypeFlags::HostCached) ))
,   TrackingData(MemoryName.data(), parent)
{
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
FVulkanMemoryAllocator::FMemoryHeap::FMemoryHeap(
    u32 granularity, VkDeviceSize capacity,
    u32 heapIndex, FMemoryTracking* parent )
:   NumAllocations(0)
,   Granularity(granularity)
,   Capacity(capacity)
,   HeapName(StringFormat("HEAP#{0}", heapIndex))
,   TrackingData(HeapName.data(), parent)
{
}
#endif
//----------------------------------------------------------------------------
FVulkanMemoryAllocator::FVulkanMemoryAllocator(const FVulkanDevice& device) NOEXCEPT
:   _device(device)
{}
//----------------------------------------------------------------------------
FVulkanMemoryAllocator::~FVulkanMemoryAllocator()
{}
//----------------------------------------------------------------------------
void FVulkanMemoryAllocator::CreateDeviceHeaps() {
    VkPhysicalDeviceMemoryProperties deviceMem;
    _device.Instance().vkGetPhysicalDeviceMemoryProperties(_device.vkPhysicalDevice(), &deviceMem);

    _memoryTypes.resize_Uninitialized(deviceMem.memoryTypeCount);
    _memoryHeaps.resize_Uninitialized(deviceMem.memoryHeapCount);

    forrange(i, 0, deviceMem.memoryHeapCount) {
        FMemoryHeap* const heap = INPLACE_NEW(&_memoryHeaps[i], FMemoryHeap) {
            Min(checked_cast<u32>(deviceMem.memoryHeaps[i].size / 8), GVulkanMaxGranularity),
                deviceMem.memoryHeaps[i].size
#if USE_PPE_MEMORYDOMAINS
                , i, & MEMORYDOMAIN_TRACKING_DATA(DeviceHeap)
#endif
        };

        Assert_NoAssume(heap->Capacity > 1);
        Assert_NoAssume(heap->Granularity >= alignof(size_t));

#if USE_PPE_MEMORYDOMAINS
        RegisterTrackingData(&heap->TrackingData);
#else
        UNUSED(heap);
#endif
    }

    forrange(i, 0, deviceMem.memoryTypeCount) {
        FMemoryType* const mem = INPLACE_NEW(&_memoryTypes[i], FMemoryType) {
            deviceMem.memoryTypes[i].heapIndex,
                static_cast<EVulkanMemoryTypeFlags>(deviceMem.memoryTypes[i].propertyFlags)
#if USE_PPE_MEMORYDOMAINS
                , i, & _memoryHeaps[deviceMem.memoryTypes[i].heapIndex].TrackingData
#endif
        };

#if USE_PPE_MEMORYDOMAINS
        RegisterTrackingData(&mem->TrackingData);
#else
        UNUSED(mem);
#endif
    }
}
//----------------------------------------------------------------------------
void FVulkanMemoryAllocator::DestroyDeviceHeaps() {
#if USE_PPE_MEMORYDOMAINS
    for (FMemoryType& memType : _memoryTypes) {
        Assert_NoAssume(memType.TrackingData.empty());
        UnregisterTrackingData(&memType.TrackingData);
    }
    for (FMemoryHeap& memHeap : _memoryHeaps) {
        Assert_NoAssume(memHeap.TrackingData.empty());
        UnregisterTrackingData(&memHeap.TrackingData);
    }
#endif

    _memoryTypes.clear();
    _memoryHeaps.clear();
}
//----------------------------------------------------------------------------
FVulkanMemoryBlock FVulkanMemoryAllocator::Allocate(
    u32 size,
    u32 memoryTypeBits,
    EVulkanMemoryTypeFlags requiredFlags,
    EVulkanMemoryTypeFlags preferredFlags ) {
    Assert(!(requiredFlags ^ preferredFlags));

    for (i32 backoff = 0;;) {
        // find best memory type available
        u32 memoryTypeIndex = u32(-1);
        forrange(i, 0, checked_cast<u32>(_memoryTypes.size())) {
            if (not (memoryTypeBits & (u32(1) << i)))
                continue;

            const FMemoryType& mem = _memoryTypes[i];
            if (mem.Flags ^ requiredFlags) {
                const FMemoryHeap& heap = _memoryHeaps[mem.HeapIndex];

                const u32 numBlocks = ((size + heap.Granularity - 1) / heap.Granularity);

    #if USE_PPE_VKOVERSUBSCRIBING_WORKAROUND
                const VkDeviceSize maxSize = (VkDeviceSize)( (mem.Flags ^ EVulkanMemoryTypeFlags::HostVisible
                     ? 0.66f : 0.83f ) * heap.Capacity );
    #else
                const u32 maxSize = heap.Capacity
    #endif

                if ((heap.NumAllocations.load(std::memory_order_relaxed) + numBlocks) * heap.Granularity > maxSize)
                    continue;

                memoryTypeIndex = i;
                if ((mem.Flags ^ preferredFlags) | (EVulkanMemoryTypeFlags::None == preferredFlags))
                    break;
            }
        }

        // bail if all heaps are full
        if  (Unlikely(u32(-1) == memoryTypeIndex))
            break;

        const FMemoryType& mem = _memoryTypes[memoryTypeIndex];
        const FMemoryHeap& heap = _memoryHeaps[mem.HeapIndex];

        // try to allocate a new block from selected heap
        const u32 alignedSize = checked_cast<u32>(Meta::RoundToNext(size, heap.Granularity));
        const u32 numBlocks = (size / heap.Granularity);

        u32 numAllocs = heap.NumAllocations.load(std::memory_order_acquire);
        if (VkDeviceSize(numAllocs + numBlocks) * heap.Granularity <= heap.Capacity &&
            heap.NumAllocations.compare_exchange_weak(numAllocs, numAllocs + numBlocks,
            std::memory_order_release, std::memory_order_relaxed) ) {

            VkMemoryAllocateInfo allocateInfo{};
            allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocateInfo.memoryTypeIndex = memoryTypeIndex;
            allocateInfo.allocationSize = alignedSize;

            FVulkanMemoryBlock b;
            b.MemoryType = memoryTypeIndex;
            b.Flags = _memoryTypes[memoryTypeIndex].Flags;
            b.Size = checked_cast<u32>(allocateInfo.allocationSize);

            PPE_VKDEVICE_CHECKED(_device.vkAllocateMemory, _device.vkDevice(), &allocateInfo, _device.vkAllocator(), &b.DeviceMemory); // #TODO: handle OOM

            Assert_NoAssume(VK_NULL_HANDLE != b.DeviceMemory);

#if USE_PPE_MEMORYDOMAINS
            mem.TrackingData.AllocateSystem(alignedSize);
#endif

            return b;
        }

        // try again, break if every heap if full (or if allocation failed)
        FPlatformProcess::SleepForSpinning(backoff);
    }

    return FVulkanMemoryBlock{};
}
//----------------------------------------------------------------------------
void FVulkanMemoryAllocator::Deallocate(const FVulkanMemoryBlock& block) NOEXCEPT {
    Assert(VK_NULL_HANDLE != block.DeviceMemory);
    Assert(block.Size);

    const FMemoryType& mem = _memoryTypes[block.MemoryType];
    const FMemoryHeap& heap = _memoryHeaps[mem.HeapIndex];

    _device.vkFreeMemory(_device.vkDevice(), block.DeviceMemory, _device.vkAllocator());

    Assert_NoAssume(Meta::IsAligned(heap.Granularity, block.Size));
    const u32 numBlocks = (block.Size / heap.Granularity);

    Assert_NoAssume(heap.NumAllocations >= numBlocks);
    heap.NumAllocations -= numBlocks;

#if USE_PPE_MEMORYDOMAINS
    mem.TrackingData.DeallocateSystem(block.Size);
#endif
}
//----------------------------------------------------------------------------
FRawMemory FVulkanMemoryAllocator::MapMemory(const FVulkanMemoryBlock& block, u32 offset/* = 0 */, u32 size/* = 0 */) {
    Assert(VK_NULL_HANDLE != block.DeviceMemory);
    Assert_NoAssume(EVulkanMemoryTypeFlags::HostVisible ^ block.Flags);

    const u32 mappedSize = (size ? size : block.Size);

    void* hostMemory = nullptr;
    PPE_VKDEVICE_CHECKED(_device.vkMapMemory,
        _device.vkDevice(),
        block.DeviceMemory,
        offset, mappedSize,
        0/* future use */,
        &hostMemory );

    Assert(hostMemory);
    return FRawMemory{ (u8*)hostMemory, mappedSize };
}
//----------------------------------------------------------------------------
void FVulkanMemoryAllocator::UnmapMemory(const FVulkanMemoryBlock& block) NOEXCEPT {
    Assert(VK_NULL_HANDLE != block.DeviceMemory);
    Assert_NoAssume(EVulkanMemoryTypeFlags::HostVisible ^ block.Flags);

    _device.vkUnmapMemory(_device.vkDevice(), block.DeviceMemory);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
