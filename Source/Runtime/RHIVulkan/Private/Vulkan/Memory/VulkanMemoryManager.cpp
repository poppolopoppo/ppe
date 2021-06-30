
#include "stdafx.h"

#include "Vulkan/Memory/VulkanMemoryManager.h"

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
bool FVulkanMemoryManager::Construct() {
    const auto exclusiveAllocators = _allocators.LockExclusive();
    Assert(exclusiveAllocators->empty());



    return true;
}
//----------------------------------------------------------------------------
void FVulkanMemoryManager::TearDown() {
    const auto exclusiveAllocators = _allocators.LockExclusive();

    exclusiveAllocators->clear();
}
//----------------------------------------------------------------------------
bool FVulkanMemoryManager::AllocateImage(FStorage* pData, VkImage image, const FMemoryDesc& desc) {
    Assert(pData);

    const auto sharedAllocators = _allocators.LockShared();
    Assert(not sharedAllocators->empty());

    forrange(i, 0, sharedAllocators->size()) {
        const FMemoryAllocatorPtr& alloc = sharedAllocators->at(i);

        if (alloc->IsSupported(desc.Type)) {
            LOG_CHECK(RHI, alloc->AllocateImage(pData, image, desc) );

            *reinterpret_cast<u32*>(&pData) = checked_cast<u32>(i);
            return true;
        }
    }

    LOG(RHI, Error, L"unsupported memory type: {0}", desc.Type);
    return false;
}
//----------------------------------------------------------------------------
bool FVulkanMemoryManager::AllocateBuffer(FStorage* pData, VkBuffer buffer, const FMemoryDesc& desc) {
    Assert(pData);

    const auto sharedAllocators = _allocators.LockShared();
    Assert(not sharedAllocators->empty());

    forrange(i, 0, sharedAllocators->size()) {
        const FMemoryAllocatorPtr& alloc = sharedAllocators->at(i);

        if (alloc->IsSupported(desc.Type)) {
            LOG_CHECK(RHI, alloc->AllocateBuffer(pData, buffer, desc) );

            *reinterpret_cast<u32*>(&pData) = checked_cast<u32>(i);
            return true;
        }
    }

    LOG(RHI, Error, L"unsupported memory type: {0}", desc.Type);
    return false;
}
//----------------------------------------------------------------------------
bool FVulkanMemoryManager::AllocateAccelStruct(FStorage* pData, VkAccelerationStructureKHR accelStruct, const FMemoryDesc& desc) {
    Assert(pData);

    const auto sharedAllocators = _allocators.LockShared();
    Assert(not sharedAllocators->empty());

    forrange(i, 0, sharedAllocators->size()) {
        const FMemoryAllocatorPtr& alloc = sharedAllocators->at(i);

        if (alloc->IsSupported(desc.Type)) {
            LOG_CHECK(RHI, alloc->AllocateAccelStruct(pData, accelStruct, desc) );

            *reinterpret_cast<u32*>(&pData) = checked_cast<u32>(i);
            return true;
        }
    }

    LOG(RHI, Error, L"unsupported memory type: {0}", desc.Type);
    return false;
}
//----------------------------------------------------------------------------
void FVulkanMemoryManager::Deallocate(FStorage& data) {
    const auto sharedAllocators = _allocators.LockShared();
    Assert(not sharedAllocators->empty());

    const u32 allocId = *reinterpret_cast<const u32*>(&data);
    sharedAllocators->at(allocId)->Deallocate(data);

    ONLY_IF_RHIDEBUG(FPlatformMemory::Memdeadbeef(&data, sizeof(data)));
}
//----------------------------------------------------------------------------
bool FVulkanMemoryManager::MemoryInfo(FVulkanMemoryInfo* pInfo, const FStorage& data) const {
    const auto sharedAllocators = _allocators.LockShared();
    Assert(not sharedAllocators->empty());

    const u32 allocId = *reinterpret_cast<const u32*>(&data);
    return sharedAllocators->at(allocId)->MemoryInfo(pInfo, data);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
