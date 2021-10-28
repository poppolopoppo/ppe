#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/ImageDesc.h"
#include "RHI/ImageView.h"

#include "Container/HashMap.h"
#include "Container/HashHelpers.h"
#include "Meta/Optional.h"
#include "Misc/Function.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanImage final {
public:
    using FImageViewDescMemoized = THashMemoizer<FImageViewDesc>;
    using FImageViewMap = HASHMAP_MEMOIZE(RHIImage, FImageViewDesc, VkImageView);
    using FImageRange = FImageDataRange;

    struct FInternalData {
        VkImage vkImage{ VK_NULL_HANDLE };
        FImageDesc Desc;

        FMemoryID MemoryId;
        VkImageAspectFlagBits AspectMask{ Default };
        VkImageLayout DefaultLayout{ Default };
        EVulkanQueueFamilyMask QueueFamilyMask{ Default };
        VkAccessFlagBits ReadAccessMask{ Default };

        FOnReleaseExternalImage OnRelease;

        bool IsReadOnly() const;
        bool IsExclusiveSharing() const { return (QueueFamilyMask == Default); }

        const uint3& Dimensions() const { return Desc.Dimensions; }
        u32 Width() const { return Desc.Dimensions.x; }
        u32 Height() const { return Desc.Dimensions.y; }
        u32 Depth() const { return Desc.Dimensions.z; }

        u32 ArrayLayers() const { return *Desc.ArrayLayers; }
        u32 MipmapLevels() const { return *Desc.MaxLevel; }
        u32 Samples() const { return *Desc.Samples; }

        EPixelFormat PixelFormat() const { return Desc.Format; }
    };

    FVulkanImage() = default;

#if USE_PPE_ASSERT
    ~FVulkanImage();
#endif

    FVulkanImage(const FVulkanImage&) = delete;
    FVulkanImage& operator =(const FVulkanImage&) = delete;

    FVulkanImage(FVulkanImage&& rvalue) NOEXCEPT;
    FVulkanImage& operator =(FVulkanImage&&) = delete;

    auto Read() const { return _data.LockShared(); }

    const FImageDesc& Desc() const { return Read()->Desc; }
    VkImage Handle() const { return Read()->vkImage; }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
    auto Write_ForDebug()  { return _data.LockExclusive(); }
#endif

    VkImageView MakeView(const FVulkanDevice& device, const FImageViewDescMemoized& desc) const;
    VkImageView MakeView(const FVulkanDevice& device, Meta::TOptional<FImageViewDesc>& desc) const;

    static bool IsSupported(const FVulkanDevice& device, const FImageDesc& desc, EMemoryType memoryType);
    bool IsSupported(const FVulkanDevice& device, const FImageViewDesc& desc) const;

    NODISCARD bool Construct(FVulkanResourceManager& resources,
        const FImageDesc& desc, FRawMemoryID memoryId, FVulkanMemoryObject& memoryObject,
        EVulkanQueueFamilyMask queueFamilyMask, EResourceState defaultState
        ARGS_IF_RHIDEBUG(FConstChar debugName) );

    NODISCARD bool Construct(const FVulkanDevice& device,
        const FVulkanExternalImageDesc& desc,
        FOnReleaseExternalImage&& onRelease
        ARGS_IF_RHIDEBUG(FConstChar debugName) );

    NODISCARD bool Construct(const FVulkanDevice& device,
        const FImageDesc& desc,
        FExternalImage externalImage, FOnReleaseExternalImage&& onRelease,
        TMemoryView<const u32> queueFamilyIndices, EResourceState defaultState
        ARGS_IF_RHIDEBUG(FConstChar debugName) );

    void TearDown(FVulkanResourceManager& resources);

private:
    NODISCARD static bool CreateView_(
        VkImageView* pImageView,
        const FInternalData& data,
        const FVulkanDevice& device,
        const FImageViewDescMemoized& desc );

    TRHIThreadSafe<FInternalData> _data;

    mutable TThreadSafe<FImageViewMap, EThreadBarrier::RWLock> _viewMap;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
