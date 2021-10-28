#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Common/VulkanAccessRecords.h"
#include "Vulkan/Image/VulkanImage.h"

#include "Maths/Range.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanLocalImage final : Meta::FNonCopyableNorMovable {
public:
    using FSubRange = FImageDataRange::FSubRange;

    struct FImageState {
        EResourceState State{ Default };
        VkImageLayout Layout{ Default };
        VkImageAspectFlagBits Aspect{ Default };
        FImageDataRange Range;
        PVulkanFrameTask Task;

        FImageState() = default;
    };

    struct FImageAccess {
        FSubRange Range;
        VkImageLayout Layout{ Default };
        VkPipelineStageFlagBits Stages{ Default };
        VkAccessFlagBits Access{ Default };
        EVulkanExecutionOrder Index{ EVulkanExecutionOrder::Initial };
        bool IsReadable : 1;
        bool IsWritable : 1;
        bool InvalidateBefore : 1;
        bool InvalidateAfter : 1;

        FImageAccess() : IsReadable(false), IsWritable(false), InvalidateBefore(false), InvalidateAfter(false) {}
    };

    using FAccessRecords = TVulkanAccessRecords<FImageAccess, FSubRange::value_type>;
    using FImageRange = FVulkanImage::FImageRange;
    using FImageViewMap = FVulkanImage::FImageViewMap;

    FVulkanLocalImage() = default;
#if USE_PPE_RHIDEBUG
    ~FVulkanLocalImage();
#endif

    bool Valid() const { return (!!_imageData); }

#if USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _imageData->DebugName(); }
    TMemoryView<const FImageAccess> ReadWriteAccess_ForDebug() const { return *_accessForReadWrite; }
#endif

    const FImageDesc& Desc() const { return _imageData->Desc(); }
    VkImage Handle() const { return _imageData->Handle(); }
    const FVulkanImage* GlobalData() const { return _imageData; }

    auto Read() const { return _imageData->Read(); }

    NODISCARD bool Construct(const FVulkanImage* pImageData);
    void TearDown();

    void SetInitialState(bool immutable, bool invalidate);
    void AddPendingState(const FImageState& st) const;
    void CommitBarrier(FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default)) const;
    void ResetState(EVulkanExecutionOrder index, FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default));

    using FImageViewDescMemoized = FVulkanImage::FImageViewDescMemoized;

    void AddPendingState(EResourceState state, VkImageLayout layout, const FImageDataRange& range, VkImageAspectFlagBits aspect, const PVulkanFrameTask& task) const {
        AddPendingState(state, layout, range, aspect, PVulkanFrameTask{ task });
    }
    void AddPendingState(EResourceState state, VkImageLayout layout, const FImageDataRange& range, VkImageAspectFlagBits aspect, PVulkanFrameTask&& rtask) const {
        AddPendingState(FImageState{ state, layout, aspect, range, std::move(rtask) });
    }

    VkImageView MakeView(const FVulkanDevice& device, const FImageViewDescMemoized& desc) const {
        return _imageData->MakeView(device, desc);
    }
    VkImageView MakeView(const FVulkanDevice& device, Meta::TOptional<FImageViewDesc>& desc) const {
        return _imageData->MakeView(device, desc);
    }

private:
    const FVulkanImage* _imageData;
    VkImageLayout _finalLayout{ VK_IMAGE_LAYOUT_GENERAL };

    mutable FAccessRecords _accessPending;
    mutable FAccessRecords _accessForReadWrite;
    bool _isImmutable{ false };
};
//----------------------------------------------------------------------------
using FImageState = FVulkanLocalImage::FImageState;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
