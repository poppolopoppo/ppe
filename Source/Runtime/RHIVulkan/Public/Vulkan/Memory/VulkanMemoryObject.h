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
struct FVulkanMemoryBlock {
    u32 AllocatorId{ UMax };
    void* MemoryHandle{ nullptr };
};
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanMemoryObject final : Meta::FNonCopyable {
public:
    struct FInternalData {
        FVulkanMemoryBlock Block;
        FMemoryDesc Desc;
    };

    FVulkanMemoryObject() = default;
    FVulkanMemoryObject(FVulkanMemoryObject&&) NOEXCEPT;
#if USE_PPE_RHIDEBUG
    ~FVulkanMemoryObject();
#endif

    EVulkanMemoryType MemoryType() const { return static_cast<EVulkanMemoryType>(_data.LockShared()->Desc.Type); }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    bool Construct(const FMemoryDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(FVulkanResourceManager& resources);

    NODISCARD bool AllocateImage(FVulkanMemoryManager& memory, VkImage image);
    NODISCARD bool AllocateBuffer(FVulkanMemoryManager& memory, VkBuffer buffer);
    NODISCARD bool AllocateAccelStruct(FVulkanMemoryManager& memory, VkAccelerationStructureKHR accelStruct);

    NODISCARD bool MemoryInfo(FVulkanMemoryInfo* pInfo, FVulkanMemoryManager& memory) const;

private:
    TRHIThreadSafe<FInternalData> _data;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
