#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/BufferDesc.h"
#include "RHI/BufferView.h"
#include "RHI/FrameGraph.h"

#include "Container/HashMap.h"
#include "Misc/Function.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanBuffer final {
public:
    struct FInternalData {
        VkBuffer vkBuffer{ VK_NULL_HANDLE };
        FBufferDesc Desc;

        FMemoryID MemoryId;
        EVulkanQueueFamilyMask QueueFamilyMask{ Default };
        VkAccessFlagBits ReadAccessMask{ Default };
        bool IsExternal : 1;

        FOnReleaseExternalBuffer OnRelease;

        bool IsReadOnly() const NOEXCEPT;
        bool IsExclusiveSharing() const { return (QueueFamilyMask == Default); }

        size_t SizeInBytes() const { return Desc.SizeInBytes; }
    };

    FVulkanBuffer() = default;
#if USE_PPE_RHIDEBUG
    ~FVulkanBuffer();
#endif

    FVulkanBuffer(const FVulkanBuffer&) = delete;
    FVulkanBuffer& operator =(const FVulkanBuffer&) = delete;

    FVulkanBuffer(FVulkanBuffer&& rvalue) NOEXCEPT;
    FVulkanBuffer& operator =(FVulkanBuffer&&) = delete;

    auto Read() const { return _data.LockShared(); }

    const FBufferDesc& Desc() const { return Read()->Desc; }
    VkBuffer Handle() const { return Read()->vkBuffer; }
    size_t SizeInBytes() const { return Read()->Desc.SizeInBytes; }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
    auto Write_ForDebug()  { return _data.LockExclusive(); }
#endif

    VkBufferView MakeView(const FVulkanDevice& device, const FBufferViewDesc& desc) const;

    static bool IsSupported(const FVulkanDevice& device, const FBufferDesc& desc, EMemoryType memoryType) NOEXCEPT;
    bool IsSupported(const FVulkanDevice& device, const FBufferViewDesc& desc) const NOEXCEPT;

    NODISCARD bool Construct(FVulkanResourceManager& resources,
        const FBufferDesc& desc, FRawMemoryID memoryId, FVulkanMemoryObject& memoryObject,
        EVulkanQueueFamilyMask queueFamilyMask
        ARGS_IF_RHIDEBUG(FConstChar debugName) );

    NODISCARD bool Construct(const FVulkanDevice& device,
        const FVulkanExternalBufferDesc& desc,
        FOnReleaseExternalBuffer&& onRelease
        ARGS_IF_RHIDEBUG(FConstChar debugName) );

    NODISCARD bool Construct(const FVulkanDevice& device,
        const FBufferDesc& desc,
        FExternalBuffer externalBuffer, FOnReleaseExternalBuffer&& onRelease,
        TMemoryView<const u32> queueFamilyIndices
        ARGS_IF_RHIDEBUG(FConstChar debugName) );

    void TearDown(FVulkanResourceManager& resources);

private:
    using FBufferViewMap = HASHMAP(RHIBuffer, FBufferViewDesc, VkBufferView);

    TRHIThreadSafe<FInternalData> _data;

    mutable TThreadSafe<FBufferViewMap, EThreadBarrier::RWLock> _viewMap;

#if USE_PPE_RHITASKNAME
    FVulkanDebugName _debugName;
#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
