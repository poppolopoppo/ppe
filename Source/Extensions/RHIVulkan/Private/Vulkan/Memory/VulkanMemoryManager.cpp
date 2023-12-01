// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Memory/VulkanMemoryManager.h"

#define USE_PPE_RHIVMA (1) // Use Vulkan Memory Allocator open-source library
#include "Vulkan/Memory/VulkanMemoryManager_VMA.h"

#include "RHI/EnumToString.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanMemoryManager::FVulkanMemoryManager(const FVulkanDevice& device)
:   _device(device) {

}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanMemoryManager::~FVulkanMemoryManager() {
    Assert_NoAssume(_allocators.LockExclusive()->empty());
}
#endif
//----------------------------------------------------------------------------
void FVulkanMemoryManager::DutyCycle(u32 frameIndex) {
    for (auto& pAllocator : *_allocators.LockExclusive())
        pAllocator->DutyCycle(frameIndex);
}
//----------------------------------------------------------------------------
void FVulkanMemoryManager::ReleaseMemory(FVulkanResourceManager& resources) {
    for (auto& pAllocator : *_allocators.LockExclusive())
        pAllocator->DefragmentMemory(resources);
}
//----------------------------------------------------------------------------
bool FVulkanMemoryManager::Construct() {
    const auto exclusiveAllocators = _allocators.LockExclusive();
    Assert(exclusiveAllocators->empty());

#if USE_PPE_RHIVMA
    // VMA is the only allocator used at the moment,
    // I'd like to implement one some day but VMA is already excellent
    exclusiveAllocators->Push(MakeUnique<FVulkanMemoryAllocator>(_device, EVulkanMemoryType::All));
#endif

    AssertReleaseMessage("no vulkan memory allocator available", not exclusiveAllocators->empty());
    return true;
}
//----------------------------------------------------------------------------
void FVulkanMemoryManager::TearDown() {
    const auto exclusiveAllocators = _allocators.LockExclusive();

    exclusiveAllocators->clear();
}
//----------------------------------------------------------------------------
bool FVulkanMemoryManager::AllocateImage(FBlock* pData, VkImage image, const FMemoryDesc& desc) {
    Assert(pData);

    const auto sharedAllocators = _allocators.LockShared();
    Assert(not sharedAllocators->empty());

    forrange(i, 0, sharedAllocators->size()) {
        const FMemoryAllocatorPtr& alloc = sharedAllocators->at(i);

        if (alloc->IsSupported(desc.Type)) {
            PPE_LOG_CHECK(RHI, alloc->AllocateImage(pData, image, desc) );
            pData->AllocatorId = checked_cast<u32>(i);
            return true;
        }
    }

    PPE_LOG(RHI, Error, "unsupported memory type: {0}", desc.Type);
    return false;
}
//----------------------------------------------------------------------------
bool FVulkanMemoryManager::AllocateBuffer(FBlock* pData, VkBuffer buffer, const FMemoryDesc& desc) {
    Assert(pData);

    const auto sharedAllocators = _allocators.LockShared();
    Assert(not sharedAllocators->empty());

    forrange(i, 0, sharedAllocators->size()) {
        const FMemoryAllocatorPtr& alloc = sharedAllocators->at(i);

        if (alloc->IsSupported(desc.Type)) {
            PPE_LOG_CHECK(RHI, alloc->AllocateBuffer(pData, buffer, desc));
            pData->AllocatorId = checked_cast<u32>(i);
            return true;
        }
    }

    PPE_LOG(RHI, Error, "unsupported memory type: {0}", desc.Type);
    return false;
}
//----------------------------------------------------------------------------
bool FVulkanMemoryManager::AllocateAccelStruct(FBlock* pData, VkAccelerationStructureNV accelStruct, const FMemoryDesc& desc) {
    Assert(pData);

    const auto sharedAllocators = _allocators.LockShared();
    Assert(not sharedAllocators->empty());

    forrange(i, 0, sharedAllocators->size()) {
        const FMemoryAllocatorPtr& alloc = sharedAllocators->at(i);

        if (alloc->IsSupported(desc.Type)) {
            PPE_LOG_CHECK(RHI, alloc->AllocateAccelStruct(pData, accelStruct, desc) );
            pData->AllocatorId = checked_cast<u32>(i);
            return true;
        }
    }

    PPE_LOG(RHI, Error, "unsupported memory type: {0}", desc.Type);
    return false;
}
//----------------------------------------------------------------------------
void FVulkanMemoryManager::Deallocate(FBlock& data) {
    const auto sharedAllocators = _allocators.LockShared();
    Assert(not sharedAllocators->empty());

    sharedAllocators->at(data.AllocatorId)->Deallocate(data);
    Assert_NoAssume(nullptr == data.MemoryHandle);

    data.AllocatorId = UMax;
}
//----------------------------------------------------------------------------
bool FVulkanMemoryManager::MemoryInfo(FVulkanMemoryInfo* pInfo, const FBlock& data) const {
    const auto sharedAllocators = _allocators.LockShared();
    Assert(not sharedAllocators->empty());

    return sharedAllocators->at(data.AllocatorId)->MemoryInfo(pInfo, data);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
