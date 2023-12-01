// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Image/VulkanLocalImage.h"

#include "Vulkan/Command/VulkanBarrierManager.h"
#include "Vulkan/Command/VulkanFrameTask.h"
#include "Vulkan/Debugger/VulkanLocalDebugger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanLocalImage::~FVulkanLocalImage() {
    Assert_NoAssume(nullptr == _imageData);
}
#endif
//----------------------------------------------------------------------------
bool FVulkanLocalImage::Construct(const FVulkanImage* pImageData) {
    Assert(pImageData);
    Assert_NoAssume(nullptr == _imageData);

    const auto sharedImg = pImageData->Read();

    _imageData = pImageData;
    _finalLayout = sharedImg->DefaultLayout;
    _isImmutable = false; //sharedImg->IsReadOnly();

    // set initial state
    FImageAccess pending{};
    pending.IsReadable = false;
    pending.IsWritable = false;
    pending.Stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    pending.Access = Zero;
    pending.Layout = sharedImg->DefaultLayout;
    pending.Index = EVulkanExecutionOrder::Initial;
    pending.Range = { 0, sharedImg->ArrayLayers() * sharedImg->MipmapLevels() };

    _accessForReadWrite->push_back(std::move(pending));

    return true;
}
//----------------------------------------------------------------------------
void FVulkanLocalImage::TearDown() {
    Assert_NoAssume(nullptr != _imageData);

    _imageData = nullptr;

    // check of uncommitted barriers

    Assert_NoAssume(_accessPending->empty());
    Assert_NoAssume(_accessForReadWrite->empty());

    _accessPending->clear();
    _accessForReadWrite->clear();
}
//----------------------------------------------------------------------------
void FVulkanLocalImage::SetInitialState(bool immutable, bool invalidate) {
    // image must be in initial state

    Assert(_accessForReadWrite->size() == 1);
    Assert(_accessForReadWrite->front().Stages == VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    _isImmutable = immutable;

    if (invalidate) {
        AssertMessage("must be mutable to allow image layout transition", not _isImmutable);

        _accessForReadWrite->front().Layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }
}
//----------------------------------------------------------------------------
void FVulkanLocalImage::AddPendingState(const FImageState& st) const {
    Assert(st.Task);
    Assert(_imageData);

    const auto sharedImg = _imageData->Read();
    Assert((st.Range.Mipmaps.First < sharedImg->MipmapLevels()) and
           (st.Range.Mipmaps.Last <= sharedImg->MipmapLevels()) );
    Assert((st.Range.Layers.First < sharedImg->ArrayLayers()) and
           (st.Range.Layers.Last <= sharedImg->ArrayLayers()) );

    if (_isImmutable)
        return;

    FImageAccess pending;
    pending.IsReadable = EResourceState_IsReadable(st.State);
    pending.IsWritable = EResourceState_IsWritable(st.State);
    pending.InvalidateBefore = (st.State & EResourceState::InvalidateBefore);
    pending.InvalidateAfter = (st.State & EResourceState::InvalidateAfter);
    pending.Stages = EResourceState_ToPipelineStages(st.State);
    pending.Access = EResourceState_ToAccess(st.State);
    pending.Layout = st.Layout;
    pending.Index = st.Task->ExecutionOrder;

    // extract sub ranges

    const u32 arrLayers = sharedImg->ArrayLayers();
    const u32 mipLevels = sharedImg->MipmapLevels();

    VECTORINSITU(RHIImage, FSubRange, 8) subRanges;
    FSubRange layerRange{ st.Range.Layers.First, Min(st.Range.Layers.Last, arrLayers) };
    FSubRange mipmapRange{ st.Range.Mipmaps.First, Min(st.Range.Mipmaps.Last, mipLevels) };

    if (st.Range.WholeLayers() and st.Range.WholeMipmaps()) {
        subRanges.emplace_back(0u, arrLayers * mipLevels);
    }
    else if (st.Range.WholeLayers()) {
        subRanges.emplace_back(
            mipmapRange.First * arrLayers + layerRange.First,
            (mipmapRange.Last - 1) * arrLayers + layerRange.Last );
    }
    else forrange(mip, mipmapRange.First, mipmapRange.Last) {
        subRanges.emplace_back(
            mip * arrLayers + layerRange.First,
            mip * arrLayers + layerRange.Last );
    }

    // merge with pending

    for (FSubRange& range : subRanges) {
        auto it = _accessPending.FindFirstAccess(range);

        if (it != _accessPending->end() and it->Range.First > range.First) {
            pending.Range = { range.First, it->Range.First };

            it = _accessPending->insert(it, pending);
            ++it;

            range.First = it->Range.First;
        }

        for (; it != _accessPending->end() and it->Range.Overlaps(range); ++it) {
            AssertMessage("something goes wrong - resource has uncommited state from another task", it->Index == pending.Index);
            AssertMessage("can't use different layouts inside single task", it->Layout == pending.Layout);

            it->Range.First = Min(it->Range.First, range.First);
            range.First = it->Range.Last;

            it->Stages |= pending.Stages;
            it->Access |= pending.Access;
            it->IsReadable |= pending.IsReadable;
            it->IsWritable |= pending.IsWritable;
            it->InvalidateBefore &= pending.InvalidateBefore;
            it->InvalidateAfter &= pending.InvalidateAfter;
        }

        if (not range.Empty()) {
            pending.Range = range;
            _accessPending->insert(it, pending);
        }
    }
}
//----------------------------------------------------------------------------
void FVulkanLocalImage::ResetState(EVulkanExecutionOrder index, FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP)) {
    AssertMessage("you must commit all pending states before reseting", _accessPending->empty());

    // add full range barrier
    {
        const auto sharedImg = _imageData->Read();

        FImageAccess pending;
        pending.IsReadable = true;
        pending.IsWritable = false;
        pending.InvalidateBefore = false;
        pending.InvalidateAfter = false;
        pending.Stages = Zero;
        pending.Access = sharedImg->ReadAccessMask;
        pending.Layout = _finalLayout;
        pending.Index = index;
        pending.Range = { 0u, sharedImg->ArrayLayers() * sharedImg->MipmapLevels() };

        _accessPending->push_back(std::move(pending));
    }

    CommitBarrier(barriers ARGS_IF_RHIDEBUG(debuggerIFP));

    // flush

    _accessForReadWrite->clear();
}
//----------------------------------------------------------------------------
void FVulkanLocalImage::CommitBarrier(FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP)) const {
    const auto sharedImg = _imageData->Read();

    for (const auto& pending : *_accessPending) {
        const auto first = _accessForReadWrite.FindFirstAccess(pending.Range);

        for (auto it = first; it != _accessForReadWrite->end() and it->Range.First < pending.Range.Last; ++it) {
            const FSubRange range = it->Range.Intersect(pending.Range);
            const u32 arrLayers = sharedImg->ArrayLayers();

            const bool isModified = (
                (it->Layout != pending.Layout) or           // layout -> layout
                (it->IsReadable and pending.IsWritable) or  // read -> write
                it->IsWritable );                           // write -> read/write

            if (not range.Empty() and isModified) {
                const bool mustInvalidate = (it->InvalidateAfter or pending.InvalidateBefore);

                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.pNext = nullptr;
                barrier.image = sharedImg->vkImage;
                barrier.oldLayout = (mustInvalidate ? VK_IMAGE_LAYOUT_UNDEFINED : it->Layout);
                barrier.newLayout = pending.Layout;
                barrier.srcAccessMask = it->Access;
                barrier.dstAccessMask = pending.Access;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                barrier.subresourceRange.aspectMask = sharedImg->AspectMask;
                barrier.subresourceRange.baseMipLevel = (range.First / arrLayers);
                barrier.subresourceRange.levelCount = ((range.Last - range.First - 1) / arrLayers + 1); // #TODO: use power of 2 values?
                barrier.subresourceRange.baseArrayLayer = (range.First % arrLayers);
                barrier.subresourceRange.layerCount = ((range.Last - range.First - 1) % arrLayers + 1);

                Assert(barrier.subresourceRange.levelCount > 0);
                Assert(barrier.subresourceRange.layerCount > 0);

                barriers.AddImageBarrier(it->Stages, pending.Stages, 0, barrier);

#if USE_PPE_RHIDEBUG
                if (debuggerIFP)
                    debuggerIFP->AddBarrier(_imageData, it->Index, pending.Index, it->Stages, pending.Stages, 0, barrier);
#endif
            }
        }

        _accessForReadWrite.ReplaceAccessRecords(first, pending);
    }

    _accessPending->clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
