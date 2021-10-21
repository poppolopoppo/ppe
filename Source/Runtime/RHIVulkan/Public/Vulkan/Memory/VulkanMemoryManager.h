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
    using FBlock = FVulkanMemoryBlock;

    class FVulkanMemoryAllocator;

    class IMemoryAllocator {
    public:
        virtual ~IMemoryAllocator() = default;

        virtual void DutyCycle(u32 frameIndex) = 0;
        virtual void DefragmentMemory(FVulkanResourceManager& resources) = 0;

        NODISCARD virtual bool IsSupported(EMemoryType memType) const NOEXCEPT = 0;

        NODISCARD virtual bool AllocateImage(FBlock* pData, VkImage image, const FMemoryDesc& desc) = 0;
        NODISCARD virtual bool AllocateBuffer(FBlock* pData, VkBuffer buffer, const FMemoryDesc& desc) = 0;
        NODISCARD virtual bool AllocateAccelStruct(FBlock* pData, VkAccelerationStructureKHR accelStruct, const FMemoryDesc& desc) = 0;
        NODISCARD virtual bool AllocateAccelStruct(FBlock* pData, VkAccelerationStructureNV accelStruct, const FMemoryDesc& desc) = 0;

        virtual void Deallocate(FBlock& data) = 0;

        NODISCARD virtual bool MemoryInfo(FVulkanMemoryInfo* pInfo, const FBlock& data) const NOEXCEPT = 0;
    };

    using FMemoryAllocatorPtr = TUniquePtr<IMemoryAllocator>;
    using FMemoryAllocatorArray = TFixedSizeStack<FMemoryAllocatorPtr, 16>;

    explicit FVulkanMemoryManager(const FVulkanDevice& device);
#if USE_PPE_RHIDEBUG
    ~FVulkanMemoryManager();
#endif

    NODISCARD bool Construct();
    void TearDown();

    void DutyCycle(u32 frameIndex);
    void ReleaseMemory(FVulkanResourceManager& resources);

    NODISCARD bool AllocateImage(FBlock* pData, VkImage image, const FMemoryDesc& desc);
    NODISCARD bool AllocateBuffer(FBlock* pData, VkBuffer buffer, const FMemoryDesc& desc);
    NODISCARD bool AllocateAccelStruct(FBlock* pData, VkAccelerationStructureKHR accelStruct, const FMemoryDesc& desc);
    NODISCARD bool AllocateAccelStruct(FBlock* pData, VkAccelerationStructureNV accelStruct, const FMemoryDesc& desc);

    void Deallocate(FBlock& data);

    NODISCARD bool MemoryInfo(FVulkanMemoryInfo* pInfo, const FBlock& data) const;

private:
    const FVulkanDevice& _device;
    TRHIThreadSafe<FMemoryAllocatorArray> _allocators;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
