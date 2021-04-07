#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/MemoryDesc.h"

#include "Meta/AlignedStorage.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanMemoryInfo {
    void* MappedPtr{ nullptr };
    VkDeviceMemory Memory{ VK_NULL_HANDLE };
    VkMemoryPropertyFlags Flags{ Default };
    u32 Offset{ 0 };
    u32 Size{ 0 };
};
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanMemoryObject final : Meta::FNonCopyable {
public:
    using FStorage = ALIGNED_STORAGE(sizeof(u64) * 4, alignof(u64));

    FVulkanMemoryObject() = default;
    FVulkanMemoryObject(FVulkanMemoryObject&&) = default;
    ~FVulkanMemoryObject();

    EMemoryType MemoryType() const {
        const FReadWriteLock::FScopeLockRead scopeLock(_rwLock);
        return _desc.Type;
    }

#if USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _debugName; }
#endif

    void Create(const FMemoryDesc& desc ARGS_IF_RHIDEBUG(FStringView debugName));
    void TearDown(FVulkanResourceManager resources);

    bool AllocateImage(FVulkanMemoryManager& memory, VkImage image);
    bool AllocateBuffer(FVulkanMemoryManager& memory, VkBuffer buffer);
    bool AllocateAccelStruct(FVulkanMemoryManager& memory, VkAccelerationStructureKHR accelStruct);

    bool MemoryInfo(FVulkanMemoryInfo* pinfo, FVulkanMemoryManager& memory) const;

private:
    FStorage _storage;
    FMemoryDesc _desc;
    FReadWriteLock _rwLock;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
