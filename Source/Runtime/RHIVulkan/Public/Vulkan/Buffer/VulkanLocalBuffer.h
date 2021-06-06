#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Buffer/VulkanBuffer.h"

#include "Maths/Range.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanLocalBuffer final : Meta::FNonCopyableNorMovable {
public:
    using FBufferRange = TRange<VkDeviceSize>;

    struct FBufferState {
        EResourceState State{ Default };
        FBufferRange Range;
        PVulkanFrameTask Task;

        FBufferState() = default;
        FBufferState(EResourceState state, VkDeviceSize begin, VkDeviceSize end, PVulkanFrameTask&& rtask)
        :   State(state), Range(begin, end), Task(std::move(rtask)) {
            Assert_NoAssume(Task);
        }
    };

    struct FBufferAccess {
        FBufferRange Range;
        VkPipelineStageFlagBits Stages{ Default };
        VkAccessFlagBits Access{ Default };
        EVulkanExecutionOrder Index{ EVulkanExecutionOrder::Initial };
        bool IsReadable : 1;
        bool IsWritable : 1;

        FBufferAccess() : IsReadable(false), IsWritable(false) {}

        bool Valid() const { return (IsReadable | IsWritable); }
    };

    using FAccessRecords = VECTORINSITU(RHIBuffer, FBufferAccess, 3);

    FVulkanLocalBuffer() = default;
    ~FVulkanLocalBuffer();

    bool Valid() const { return (!!_bufferData); }

#ifdef USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _bufferData->DebugName(); }
#endif

    VkBuffer Handle() const { return _bufferData->Handle(); }
    const FVulkanBuffer* GlobalData() const { return _bufferData; }
    auto InternalData() const { return _bufferData->Read(); }

    NODISCARD bool Construct(const FVulkanBuffer* bufferData);
    void TearDown();

    void SetInitialState(bool immutable);
    void AddPendingState(const FBufferState& bufferState);
    void CommitBarrier(FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default));
    void ResetState(EVulkanExecutionOrder index, FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default));

    VkBufferView MakeView(const FVulkanDevice& device, const FBufferViewDesc& desc) const {
        return _bufferData->MakeView(device, desc);
    }

private:
    static FAccessRecords::iterator FindFirstAccess_(FAccessRecords& arr, const FBufferRange& range);
    static void ReplaceAccessRecords_(FAccessRecords& arr, FAccessRecords::iterator it, const FBufferAccess& barrier);
    static FAccessRecords::iterator EraseAccessRecords_(FAccessRecords& arr, FAccessRecords::iterator it, const FBufferRange& range);

    const FVulkanBuffer* _bufferData;

    FAccessRecords _accessPending;
    FAccessRecords _accessForRead;
    FAccessRecords _accessForWrite;
    bool _isImmutable{ false };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
