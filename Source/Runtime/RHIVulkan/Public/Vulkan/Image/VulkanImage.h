#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/ImageDesc.h"
#include "RHI/ImageView.h"

#include "Container/HashMap.h"
#include "Container/HashHelpers.h"
#include "Misc/Function.h"
#include "Thread/ReadWriteLock.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanImage final : public FRefCountable {
public:
    using FImageViewDescMemoized = THashMemoizer<FImageViewDesc>;
    using FReleaseCallback = TFunction<void(struct FInternalData&)>;

    struct FInternalData {
        VkImage vkImage{ VK_NULL_HANDLE };
        FImageDesc Desc;

        FMemoryID MemoryId;
        VkImageAspectFlagBits AspectMask{ Default };
        VkImageLayout DefaultLayout{ Default };
        EVulkanQueueFamilyMask QueueFamilyMask{ Default };
        VkAccessFlagBits ReadAccessMask{ Default };

        FReleaseCallback OnRelease;

        bool IsReadOnly() const;
        bool IsExclusiveSharing() const { return (Default == QueueFamilyMask); }

        const uint3& Dimensions() const { return Desc.Dimensions; }
        u32 Width() const { return Desc.Dimensions.x; }
        u32 Height() const { return Desc.Dimensions.y; }
        u32 Depth() const { return Desc.Dimensions.z; }

        u32 ArrayLayers() const { return *Desc.ArrayLayers; }
        u32 MipmapLevels() const { return *Desc.MaxLevel; }
        u32 Samples() const { return *Desc.Samples; }

        EPixelFormat PixelFormat() const { return Desc.Format; }
        EImageType ImageType() const { return Desc.Type; }

        bool MakeView(VkImageView* pImageView, const FVulkanDevice& device, const FImageViewDescMemoized& desc) const;
    };

    FVulkanImage() = default;
    ~FVulkanImage();

    FVulkanImage(const FVulkanImage&) = delete;
    FVulkanImage& operator =(const FVulkanImage&) = delete;

    FVulkanImage(FVulkanImage&&) NOEXCEPT;
    FVulkanImage& operator =(FVulkanImage&&) NOEXCEPT;

    auto Read() const { return _data.LockShared(); }

    VkImage Handle() const { return Read()->vkImage; }

#ifdef USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _debugName; }
#endif

    VkImageView MakeView(const FVulkanDevice& device, const FImageViewDescMemoized& desc) const;
    VkImageView MakeView(const FVulkanDevice& device, bool isDefault, FImageViewDesc& desc) const;

    static bool IsSupported(const FVulkanDevice& device, const FImageDesc& desc, EMemoryType memoryType);
    bool IsSupported(const FVulkanDevice& device, FImageViewDesc& desc);

    bool Create(FVulkanResourceManager& resources,
        const FImageDesc& desc, FRawMemoryID memoryId, FVulkanMemoryObject& memoryObject,
        EVulkanQueueFamilyMask queueFamilyMask, EResourceState defaultState
        ARGS_IF_RHIDEBUG(const FStringView& debugName) );

    bool Create(FVulkanDevice& device,
        const FImageDesc& desc, FReleaseCallback&& onRelease
        ARGS_IF_RHIDEBUG(const FStringView& debugName) );

    void TearDown(FVulkanResourceManager& resources);

private:
    using FImageViewMap = HASHMAP_MEMOIZE(RHIImage, FImageViewDesc, VkImageView);

    TRHIThreadSafe<FInternalData> _data;

    mutable FReadWriteLock _viewRWLock;
    mutable FImageViewMap _viewMap;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
