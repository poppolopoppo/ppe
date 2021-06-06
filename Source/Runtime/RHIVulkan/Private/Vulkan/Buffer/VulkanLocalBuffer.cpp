
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
FVulkanLocalBuffer::~FVulkanLocalBuffer() {
    Assert_NoAssume(nullptr == _bufferData); // must call TearDown() !
    Assert_NoAssume(_accessPending.empty());
    Assert_NoAssume(_accessForRead.empty());
    Assert_NoAssume(_accessForWrite.empty());
}
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

    // check uncommitted barriers
    Assert_NoAssume(_accessPending.empty());
    Assert_NoAssume(_accessForRead.empty());
    Assert_NoAssume(_accessForWrite.empty());

    _accessPending.clear_ReleaseMemory();
    _accessForRead.clear_ReleaseMemory();
    _accessForWrite.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
void FVulkanLocalBuffer::SetInitialState(bool immutable) {
    Assert_NoAssume(_accessPending.empty());
    Assert_NoAssume(_accessForRead.empty());
    Assert_NoAssume(_accessForWrite.empty());

    _isImmutable = immutable;
}
//----------------------------------------------------------------------------
void FVulkanLocalBuffer::AddPendingState(const FBufferState& bufferState) {
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
    auto it = FindFirstAccess_(_accessPending, range);

    if (_accessPending.end() != it && it->Range.First > range.First) {
        pending.Range = FBufferRange{ range.First, it->Range.First };
        it = _accessPending.insert(it, pending);
        ++it;

        range.First = it->Range.First;
    }

    for (; it != _accessPending.end() && it->Range.Overlaps(range); ++it) {
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
        _accessPending.insert(it, pending);
    }
}
//----------------------------------------------------------------------------
void FVulkanLocalBuffer::CommitBarrier(FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP)) {
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

    for (const FBufferAccess& pending : _accessPending) {
        const auto itW = FindFirstAccess_(_accessForWrite, pending.Range);
        const auto itR = FindFirstAccess_(_accessForRead, pending.Range);

        if (pending.IsWritable) {
            // write -> write, read -> write barriers
            bool barrierWW = true;

            if (itW != _accessForWrite.end() && itR != _accessForRead.end()) {
                barrierWW = (itW->Index >= itR->Index);
            }

            if (barrierWW && itW != _accessForWrite.end()) {
                for(auto jt = itW; jt != _accessForWrite.end() && jt->Range.First < pending.Range.Last; ++jt)
                    addBarrier(jt->Range.Intersect(pending.Range), *jt, pending);
            }
            else if (itR != _accessForRead.end()) {
                for(auto jt = itR; jt != _accessForRead.end() && jt->Range.First < pending.Range.Last; ++jt)
                    addBarrier(jt->Range.Intersect(pending.Range), *jt, pending);
            }

            ReplaceAccessRecords_(_accessForWrite, itW, pending);
            EraseAccessRecords_(_accessForRead, itR, pending.Range);
        }
        else {
            Assert(pending.IsReadable);
            // write -> read barrier only
            for (auto jt = itW; jt != _accessForWrite.end() && jt->Range.First < pending.Range.Last; ++jt)
                addBarrier(jt->Range.Intersect(pending.Range), *jt, pending);

            ReplaceAccessRecords_(_accessForRead, itR, pending);
        }
    }

    _accessPending.clear();
}
//----------------------------------------------------------------------------
void FVulkanLocalBuffer::ResetState(EVulkanExecutionOrder index, FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP)) {
    Assert(_accessPending.empty());

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

        _accessPending.push_back(std::move(pending));
    }

    CommitBarrier(barriers ARGS_IF_RHIDEBUG(debuggerIFP));

    // flush
    _accessForRead.clear();
    _accessForWrite.clear();
}
//----------------------------------------------------------------------------
auto FVulkanLocalBuffer::FindFirstAccess_(FAccessRecords& arr, const FBufferRange& range) -> FAccessRecords::iterator {
    u32 first = 0;
    u32 last = checked_cast<u32>(arr.size());

    while (first < last) {
        const u32 pivot = (first + last) >> 1;
        if (arr[pivot].Range.end() < range.begin())
            first = pivot + 1;
        else
            last = pivot;
    }

    if (first < arr.size() && arr[first].Range.end() >= range.begin())
        return (arr.begin() + first);

    return arr.end();
}
//----------------------------------------------------------------------------
void FVulkanLocalBuffer::ReplaceAccessRecords_(
    FAccessRecords& arr,
    FAccessRecords::iterator it,
    const FBufferAccess& barrier ) {
    Assert(arr.AliasesToContainer(it));

    bool replaced = false;
    while (it != arr.end()) {
        if (it->Range.First < barrier.Range.First &&
            it->Range.Last <= barrier.Range.Last ) {
            //	|1111111|22222|
            //     |bbbbb|          +
            //  |11|....            =
            it->Range.Last = barrier.Range.First;
            ++it;
            continue;
        }

        if (it->Range.First < barrier.Range.First &&
            it->Range.Last > barrier.Range.Last ) {
            //  |111111111111111|
            //      |bbbbb|	        +
            //  |111|bbbbb|11111|   =
            const FBufferAccess cpy = *it;

            it->Range.Last = it->Range.First;
            it = arr.insert(it + 1, barrier);
            replaced = true;

            it = arr.insert(it + 1, cpy);
            it->Range.First = barrier.Range.Last;
            continue;
        }

        if (it->Range.First >= barrier.Range.First &&
            it->Range.Last < barrier.Range.Last ) {
            if (it->Range.Last > barrier.Range.Last) {
                //  ...|22222222222|
                //   |bbbbbbb|          +
                //  ...bbbbbb|22222|    =
                it->Range.First = barrier.Range.Last;

                if (not replaced) {
                    arr.insert(it, barrier);
                    replaced = true;
                }
                break;
            }

            if (replaced) {
                //  ...|22222|33333|
                //   |bbbbbbbbbbb|      +
                //  ...|bbbbbbbbb|...   =
                it = arr.erase(it);
            }
            else {
                *it = barrier;
                ++it;
                replaced = true;
            }
            continue;
        }

        break;
    }

    if (not replaced)
        arr.insert(it, barrier);
}
//----------------------------------------------------------------------------
auto FVulkanLocalBuffer::EraseAccessRecords_(
    FAccessRecords& arr,
    FAccessRecords::iterator it,
    const FBufferRange& range ) -> FAccessRecords::iterator {
    if (arr.empty())
        return it;

    Assert(arr.AliasesToContainer(it));

    while (it != arr.end()) {
        if (it->Range.First < range.First &&
            it->Range.Last > range.Last ) {
            const FBufferAccess cpy = *it;
            it->Range.Last = range.First;

            it = arr.insert(it + 1, cpy);
            it->Range.First = range.Last;
            break;
        }

        if (it->Range.First < range.First) {
            Assert(it->Range.Last <= range.First);
            it->Range.Last = range.First;
            ++it;
            continue;
        }

        if (it->Range.Last > range.Last) {
            Assert(it->Range.First <= range.Last);
            it->Range.First = range.Last;
            break;
        }

        it = arr.erase(it);
    }

    return it;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
