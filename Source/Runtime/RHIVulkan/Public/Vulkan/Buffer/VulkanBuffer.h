#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/BufferDesc.h"
#include "RHI/BufferView.h"

#include "Container/HashMap.h"
#include "Misc/Function.h"
#include "Thread/ReadWriteLock.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanBuffer final : public FRefCountable {
public:
    using FReleaseCallback = TFunction<void(struct FInternalData&)>;

    struct FInternalData {
        VkBuffer vkBuffer{ VK_NULL_HANDLE };
        FBufferDesc Desc;

        FMemoryID MemoryId;
        EVulkanQueueFamilyMask QueueFamilyMask{ Default };
        VkAccessFlagBits ReadAccessMask{ Default };

        FReleaseCallback OnRelease;

        bool IsReadOnly() const;
        bool IsExclusiveSharing() const { return (Default == QueueFamilyMask); }

        u32 SizeInBytes() const { return Desc.SizeInBytes; }

        bool MakeView(VkBufferView* pBufferView, const FVulkanDevice& device, const FBufferViewDesc& desc) const;
    };

    FVulkanBuffer() = default;
    ~FVulkanBuffer();

    FVulkanBuffer(const FVulkanBuffer&) = delete;
    FVulkanBuffer& operator =(const FVulkanBuffer&) = delete;

    FVulkanBuffer(FVulkanBuffer&&) NOEXCEPT;
    FVulkanBuffer& operator =(FVulkanBuffer&&) NOEXCEPT;

    auto Read() const { return _data.LockShared(); }

    VkBuffer Handle() const { return Read()->vkBuffer; }
    u32 SizeInBytes() const { return Read()->Desc.SizeInBytes; }

#ifdef USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _debugName; }
#endif

    VkBufferView MakeView(const FVulkanDevice& device, const FBufferViewDesc& desc) const;

    static bool IsSupported(const FVulkanDevice& device, const FBufferDesc& desc, EMemoryType memoryType);
    bool IsSupported(const FVulkanDevice& device, FBufferViewDesc& desc);

    bool Create(FVulkanResourceManager& resources,
        const FBufferDesc& desc, FRawMemoryID memoryId, FVulkanMemoryObject& memoryObject, EVulkanQueueFamilyMask queueFamilyMask
        ARGS_IF_RHIDEBUG(const FStringView& debugName) );

    bool Create(FVulkanDevice& device,
        const FBufferDesc& desc, FReleaseCallback&& onRelease
        ARGS_IF_RHIDEBUG(const FStringView& debugName) );

    void TearDown(FVulkanResourceManager& resources);

private:
    using FBufferViewMap = HASHMAP(RHIBuffer, FBufferViewDesc, VkBufferView);

    TRHIThreadSafe<FInternalData> _data;

    mutable FReadWriteLock _viewRWLock;
    mutable FBufferViewMap _viewMap;

#if USE_PPE_RHITASKNAME
    FVulkanDebugName _debugName;
#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
