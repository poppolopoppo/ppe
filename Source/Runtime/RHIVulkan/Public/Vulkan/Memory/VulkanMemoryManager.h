#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Memory/VulkanMemoryObject.h"

#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanMemoryManager final {
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

        NODISCARD virtual bool IsSupported(EMemoryType memType) const = 0;

        NODISCARD virtual bool AllocateImage(FStorage* pdata, VkImage image, const FMemoryDesc& desc) = 0;
        NODISCARD virtual bool AllocateBuffer(FStorage* pdata, VkBuffer buffer, const FMemoryDesc& desc) = 0;
        NODISCARD virtual bool AllocateAccelStruct(FStorage* pdata, VkAccelerationStructureKHR accelStruct, const FMemoryDesc& desc) = 0;

        virtual void Deallocate(FStorage& data) = 0;

        NODISCARD virtual bool MemoryInfo(FVulkanMemoryInfo* pinfo, const FStorage& data) const  = 0;
    };

    using FMemoryAllocatorPtr = TUniquePtr<IMemoryAllocator>;
    using FMemoryAllocatorArray = TFixedSizeStack<FMemoryAllocatorPtr, 16>;

    explicit FVulkanMemoryManager(const FVulkanDevice& device);
#if USE_PPE_RHIDEBUG
    ~FVulkanMemoryManager();
#endif

    NODISCARD bool Construct();
    void TearDown();

    NODISCARD bool AllocateImage(FStorage* pData, VkImage image, const FMemoryDesc& desc);
    NODISCARD bool AllocateBuffer(FStorage* pData, VkBuffer buffer, const FMemoryDesc& desc);
    NODISCARD bool AllocateAccelStruct(FStorage* pData, VkAccelerationStructureKHR accelStruct, const FMemoryDesc& desc);

    void Deallocate(FStorage& data);

    NODISCARD bool MemoryInfo(FVulkanMemoryInfo* pInfo, const FStorage& data) const;

private:
    const FVulkanDevice& _device;
    TRHIThreadSafe<FMemoryAllocatorArray> _allocators;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
