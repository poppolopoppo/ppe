
#include "stdafx.h"

#include "Vulkan/Memory/VulkanMemoryObject.h"

#include "Vulkan/Instance/VulkanResourceManager.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanMemoryObject::~FVulkanMemoryObject() {
    const auto exclusiveData = _data.LockExclusive();
    Assert_NoAssume(nullptr == exclusiveData->Block.MemoryHandle);
}
#endif
//----------------------------------------------------------------------------
FVulkanMemoryObject::FVulkanMemoryObject(FVulkanMemoryObject&& rvalue) NOEXCEPT
:   _data(std::move(*rvalue._data.LockExclusive()))
{
}
//----------------------------------------------------------------------------
bool FVulkanMemoryObject::Construct(const FMemoryDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    const auto exclusiveData = _data.LockExclusive();
    Assert_NoAssume(nullptr == exclusiveData->Block.MemoryHandle);

    exclusiveData->Desc = desc;

    ONLY_IF_RHIDEBUG(_debugName = debugName);
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanMemoryObject::AllocateBuffer(FVulkanMemoryManager& memory, VkBuffer buffer) {
    const auto exclusiveData = _data.LockExclusive();
    Assert_NoAssume(nullptr == exclusiveData->Block.MemoryHandle);

    LOG_CHECK(RHI, memory.AllocateBuffer(&exclusiveData->Block, buffer, exclusiveData->Desc) );
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanMemoryObject::AllocateImage(FVulkanMemoryManager& memory, VkImage image) {
    const auto exclusiveData = _data.LockExclusive();
    Assert_NoAssume(nullptr == exclusiveData->Block.MemoryHandle);

    LOG_CHECK(RHI, memory.AllocateImage(&exclusiveData->Block, image, exclusiveData->Desc) );
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanMemoryObject::AllocateAccelStruct(FVulkanMemoryManager& memory, VkAccelerationStructureKHR accelStruct) {
    const auto exclusiveData = _data.LockExclusive();
    Assert_NoAssume(nullptr == exclusiveData->Block.MemoryHandle);

    LOG_CHECK(RHI, memory.AllocateAccelStruct(&exclusiveData->Block, accelStruct, exclusiveData->Desc) );
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanMemoryObject::AllocateAccelStruct(FVulkanMemoryManager& memory, VkAccelerationStructureNV accelStruct) {
    const auto exclusiveData = _data.LockExclusive();
    Assert_NoAssume(nullptr == exclusiveData->Block.MemoryHandle);

    LOG_CHECK(RHI, memory.AllocateAccelStruct(&exclusiveData->Block, accelStruct, exclusiveData->Desc) );
    return true;
}
//----------------------------------------------------------------------------
void FVulkanMemoryObject::TearDown(FVulkanResourceManager& resources) {
    const auto exclusiveData = _data.LockExclusive();

    if (exclusiveData->Block.MemoryHandle)
        resources.MemoryManager().Deallocate(exclusiveData->Block);Assert_NoAssume(nullptr == exclusiveData->Block.MemoryHandle);

    Assert_NoAssume(UMax == exclusiveData->Block.AllocatorId);
    Assert_NoAssume(nullptr == exclusiveData->Block.MemoryHandle);

    exclusiveData->Desc = Default;

    ONLY_IF_RHIDEBUG(_debugName.Clear());
}
//----------------------------------------------------------------------------
bool FVulkanMemoryObject::MemoryInfo(FVulkanMemoryInfo* pInfo, FVulkanMemoryManager& memory) const {
    Assert(pInfo);

    const auto sharedData = _data.LockShared();
    return memory.MemoryInfo(pInfo, sharedData->Block);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
