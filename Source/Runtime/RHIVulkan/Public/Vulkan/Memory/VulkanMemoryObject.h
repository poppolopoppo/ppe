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
    ubyte* MappedPtr{ nullptr };
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
    FVulkanMemoryObject(FVulkanMemoryObject&&) NOEXCEPT;
    ~FVulkanMemoryObject();

    EMemoryType MemoryType() const {
        const FReadWriteLock::FScopeLockRead scopeLock(_rwLock);
        return _desc.Type;
    }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    bool Construct(const FMemoryDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(FVulkanResourceManager& resources);

    NODISCARD bool AllocateImage(FVulkanMemoryManager& memory, VkImage image);
    NODISCARD bool AllocateBuffer(FVulkanMemoryManager& memory, VkBuffer buffer);
    NODISCARD bool AllocateAccelStruct(FVulkanMemoryManager& memory, VkAccelerationStructureKHR accelStruct);

    NODISCARD bool MemoryInfo(FVulkanMemoryInfo* pinfo, FVulkanMemoryManager& memory) const;

private:
    FReadWriteLock _rwLock;
    FStorage _storage;
    FMemoryDesc _desc;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
