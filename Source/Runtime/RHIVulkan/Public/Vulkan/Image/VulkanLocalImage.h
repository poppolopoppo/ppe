#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Image/VulkanImage.h"

#include "Maths/Range.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanLocalImage final : Meta::FNonCopyableNorMovable {
public:
    struct FImageState {
        EResourceState State{ Default };
        VkImageLayout Layout{ Default };
        VkImageAspectFlagBits Aspect{ Default };
        FImageDataRange Range;
        PVulkanFrameTask Task;

        FImageState() = default;
        FImageState(EResourceState state, VkImageLayout layout, const FImageDataRange& range, VkImageAspectFlagBits aspect, const PVulkanFrameTask& task)
        :   FImageState(state, layout, range, aspect, PVulkanFrameTask{ task })
        {}
        FImageState(EResourceState state, VkImageLayout layout, const FImageDataRange& range, VkImageAspectFlagBits aspect, PVulkanFrameTask&& rtask) NOEXCEPT
        :   State(state), Layout(layout), Aspect(aspect), Range(range), Task(std::move(rtask)) {
            Assert_NoAssume(Task);
        }
    };

    struct FImageAccess {
        FImageSubresourceRange Range;
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

    using FAccessRecords = VECTORINSITU(RHIImage, FImageAccess, 3);
    using FImageRange = FVulkanImage::FImageRange;
    using FImageViewMap = FVulkanImage::FImageViewMap;

    FVulkanLocalImage() = default;
    ~FVulkanLocalImage();

    bool Valid() const { return (!!_imageData); }

#if USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _imageData->DebugName(); }
#endif

    VkImage Handle() const { return _imageData->Handle(); }
    const FVulkanImage* GlobalData() const { return _imageData; }
    auto Read() const { return _imageData->Read(); }

    NODISCARD bool Construct(const FVulkanImage* imageData);
    void TearDown();

    void SetInitialState(bool immutable, bool invalidate);
    void AddPendingState(FImageState&& rstate) const;
    void CommitBarrier(FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default)) const;
    void ResetState(EVulkanExecutionOrder index, FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default));

    using FImageViewDescMemoized = FVulkanImage::FImageViewDescMemoized;

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
