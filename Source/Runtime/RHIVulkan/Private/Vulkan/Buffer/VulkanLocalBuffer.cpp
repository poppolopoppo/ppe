
#include "stdafx.h"

#include "Vulkan/Buffer/VulkanLocalBuffer.h"

#include "Vulkan/Command/VulkanBarrierManager.h"
#include "Vulkan/Command/VulkanFrameTask.h"
#include "Vulkan/Debugger/VulkanLocalDebugger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanLocalBuffer::~FVulkanLocalBuffer() {
    Assert_NoAssume(nullptr == _bufferData); // must call TearDown() !
    Assert_NoAssume(_accessPending->empty());
    Assert_NoAssume(_accessForRead->empty());
    Assert_NoAssume(_accessForWrite->empty());
}
#endif
//----------------------------------------------------------------------------
bool FVulkanLocalBuffer::Construct(const FVulkanBuffer* bufferData) {
    Assert(bufferData);
    Assert_NoAssume(nullptr == _bufferData);

    _bufferData = bufferData;
    _isImmutable = _bufferData->Read()->IsReadOnly();

    return true;
}
//----------------------------------------------------------------------------
void FVulkanLocalBuffer::TearDown() {
    Assert(_bufferData);

    _bufferData = nullptr;

    // check uncommitted barriers
    Assert_NoAssume(_accessPending->empty());
    Assert_NoAssume(_accessForRead->empty());
    Assert_NoAssume(_accessForWrite->empty());

    _accessPending->clear_ReleaseMemory();
    _accessForRead->clear_ReleaseMemory();
    _accessForWrite->clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
void FVulkanLocalBuffer::SetInitialState(bool immutable) {
    Assert_NoAssume(_accessPending->empty());
    Assert_NoAssume(_accessForRead->empty());
    Assert_NoAssume(_accessForWrite->empty());

    _isImmutable = immutable;
}
//----------------------------------------------------------------------------
void FVulkanLocalBuffer::AddPendingState(const FBufferState& bufferState) const {
    Assert(bufferState.Task);

    if (_isImmutable)
        return;

    Assert(bufferState.Range.First < _bufferData->Read()->SizeInBytes());

    FBufferAccess pending;
    pending.Range = bufferState.Range;
    pending.Index = bufferState.Task->ExecutionOrder();
    pending.Stages = EResourceState_ToPipelineStages(bufferState.State);
    pending.Access = EResourceState_ToAccess(bufferState.State);
    pending.IsReadable = EResourceState_IsReadable(bufferState.State);
    pending.IsWritable = EResourceState_IsWritable(bufferState.State);

    // merge with pending
    FBufferRange range = pending.Range;
    auto it = _accessPending.FindFirstAccess(range);

    if (_accessPending->end() != it && it->Range.First > range.First) {
        pending.Range = FBufferRange{ range.First, it->Range.First };
        it = _accessPending->insert(it, pending);
        ++it;

        range.First = it->Range.First;
    }

    for (; it != _accessPending->end() && it->Range.Overlaps(range); ++it) {
        Assert_NoAssume(it->Index == pending.Index);

        it->Range.First = Min(it->Range.First, range.First);
        range.First = it->Range.Last;

        it->Stages |= pending.Stages;
        it->Access |= pending.Access;
        it->IsReadable |= pending.IsReadable;
        it->IsWritable |= pending.IsWritable;
    }

    if (not range.Empty()) {
        pending.Range = range;
        _accessPending->insert(it, pending);
    }
}
//----------------------------------------------------------------------------
void FVulkanLocalBuffer::CommitBarrier(FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP)) const {
    if (_isImmutable)
        return;

    const auto addBarrier = [this, &barriers ARGS_IF_RHIDEBUG(debuggerIFP)](
        const FBufferRange& sharedRange, const FBufferAccess& src, const FBufferAccess& dst ) {
        if (not sharedRange.Empty()) {
            VkBufferMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.pNext = nullptr;
            barrier.buffer = Handle();
            barrier.offset = sharedRange.First;
            barrier.size = sharedRange.Extent();
            barrier.srcAccessMask = src.Access;
            barrier.dstAccessMask = dst.Access;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            barriers.AddBufferBarrier(src.Stages, dst.Stages, barrier);

#if USE_PPE_RHIDEBUG
            if (debuggerIFP)
                debuggerIFP->AddBarrier(_bufferData, src.Index, dst.Index, src.Stages, dst.Stages, 0, barrier);
#endif
        }
    };

    for (const FBufferAccess& pending : *_accessPending) {
        const auto itW = _accessForWrite.FindFirstAccess(pending.Range);
        const auto itR = _accessForRead.FindFirstAccess(pending.Range);

        if (pending.IsWritable) {
            // write -> write, read -> write barriers
            bool barrierWW = true;

            if (itW != _accessForWrite->end() && itR != _accessForRead->end()) {
                barrierWW = (itW->Index >= itR->Index);
            }

            if (barrierWW && itW != _accessForWrite->end()) {
                for(auto jt = itW; jt != _accessForWrite->end() && jt->Range.First < pending.Range.Last; ++jt)
                    addBarrier(jt->Range.Intersect(pending.Range), *jt, pending);
            }
            else if (itR != _accessForRead->end()) {
                for(auto jt = itR; jt != _accessForRead->end() && jt->Range.First < pending.Range.Last; ++jt)
                    addBarrier(jt->Range.Intersect(pending.Range), *jt, pending);
            }

            _accessForWrite.ReplaceAccessRecords(itW, pending);
            _accessForRead.EraseAccessRecords(itR, pending.Range);
        }
        else {
            Assert(pending.IsReadable);
            // write -> read barrier only
            for (auto jt = itW; jt != _accessForWrite->end() && jt->Range.First < pending.Range.Last; ++jt)
                addBarrier(jt->Range.Intersect(pending.Range), *jt, pending);

            _accessForRead.ReplaceAccessRecords(itR, pending);
        }
    }

    _accessPending->clear();

#if USE_PPE_RHIDEBUG
    // check for empty barriers !
    for (const FBufferAccess& access : _accessForRead->MakeView())
        Assert(not access.Range.Empty());
    for (const FBufferAccess& access : _accessForWrite->MakeView())
        Assert(not access.Range.Empty());
#endif
}
//----------------------------------------------------------------------------
void FVulkanLocalBuffer::ResetState(EVulkanExecutionOrder index, FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP)) {
    Assert(_accessPending->empty());

    if (_isImmutable)
        return;

    const auto sharedBuf = _bufferData->Read();

    // add full range barrier
    {
        FBufferAccess pending;
        pending.Range = FBufferRange{ 0 , static_cast<VkDeviceSize>(_bufferData->Read()->SizeInBytes()) };
        pending.Stages = static_cast<VkPipelineStageFlagBits>(0);
        pending.Access = sharedBuf->ReadAccessMask;
        pending.IsReadable = true;
        pending.IsWritable = false;
        pending.Index = index;

        _accessPending->push_back(std::move(pending));
    }

    CommitBarrier(barriers ARGS_IF_RHIDEBUG(debuggerIFP));

    // flush
    _accessForRead->clear();
    _accessForWrite->clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
