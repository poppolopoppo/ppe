#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Memory/VulkanMemoryObject.h"

#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanMemoryManager final : public FRefCountable {
public:
    using FStorage = FVulkanMemoryObject::FStorage;

    class FDedicatedMemoryAllocator;
    class FHostMemoryAllocator;
    class FDeviceMemoryAllocator;
    class FVirtualMemoryAllocator;
    class FVulkanMemoryAllocator;

    class IMemoryAllocator {
    public:
        virtual ~IMemoryAllocator() = default;

        virtual bool IsSupported(EMemoryType memType) const = 0;

        virtual bool AllocateImage(FStorage* pdata, VkImage image, const FMemoryDesc& desc) = 0;
        virtual bool AllocateBuffer(FStorage* pdata, VkBuffer buffer, const FMemoryDesc& desc) = 0;
        virtual bool AllocateAccelStruct(FStorage* pdata, VkAccelerationStructureKHR accelStruct, const FMemoryDesc& desc) = 0;

        virtual void Deallocate(FStorage& data) = 0;

        virtual bool MemoryInfo(FVulkanMemoryInfo* pinfo, const FStorage& data) const  = 0;
    };

    using FMemoryAllocatorPtr = TUniquePtr<IMemoryAllocator>;
    using FMemoryAllocatorArray = TFixedSizeStack<FMemoryAllocatorPtr, 16>;

    explicit FVulkanMemoryManager(const FVulkanDevice& device);
    ~FVulkanMemoryManager();

    bool Create();
    void TearDown();

    bool AllocateImage(FStorage* pdata, VkImage image, const FMemoryDesc& desc);
    bool AllocateBuffer(FStorage* pdata, VkBuffer buffer, const FMemoryDesc& desc);
    bool AllocateAccelStruct(FStorage* pdata, VkAccelerationStructureKHR accelStruct, const FMemoryDesc& desc);

    void Deallocate(FStorage& data);

    bool MemoryInfo(FVulkanMemoryInfo* pinfo, const FStorage& data) const;

private:
    FReadWriteLock _rwLock;
    const FVulkanDevice& _device;
    FMemoryAllocatorArray _allocators;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
