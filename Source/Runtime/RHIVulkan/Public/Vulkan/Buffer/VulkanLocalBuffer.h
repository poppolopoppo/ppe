#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Buffer/VulkanBuffer.h"
#include "Vulkan/Common/VulkanAccessRecords.h"

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
    };

    struct FBufferAccess {
        FBufferRange Range;
        VkPipelineStageFlagBits Stages{ Default };
        VkAccessFlagBits Access{ Default };
        EVulkanExecutionOrder Index{ EVulkanExecutionOrder::Initial };
        bool IsReadable : 1;
        bool IsWritable : 1;

        FBufferAccess() NOEXCEPT : IsReadable(false), IsWritable(false) {}

        bool Valid() const { return (IsReadable || IsWritable); }
    };

    using FAccessRecords = TVulkanAccessRecords<FBufferAccess, VkDeviceSize>;

    FVulkanLocalBuffer() = default;
#if USE_PPE_RHIDEBUG
    ~FVulkanLocalBuffer();
#endif

    bool Valid() const { return (!!_bufferData); }

#if USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _bufferData->DebugName(); }
    TMemoryView<const FBufferAccess> ReadAccess_ForDebug() const { return *_accessForRead; }
    TMemoryView<const FBufferAccess> WriteAccess_ForDebug() const { return *_accessForWrite; }
#endif

    const FBufferDesc& Desc() const { return _bufferData->Desc(); }
    VkBuffer Handle() const { return _bufferData->Handle(); }
    const FVulkanBuffer* GlobalData() const { return _bufferData; }

    auto Read() const { return _bufferData->Read(); }

    NODISCARD bool Construct(const FVulkanBuffer* bufferData);
    void TearDown();

    void SetInitialState(bool immutable);
    void AddPendingState(const FBufferState& bufferState) const;
    void CommitBarrier(FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default)) const;
    void ResetState(EVulkanExecutionOrder index, FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default));

    void AddPendingState(EResourceState state, VkDeviceSize begin, VkDeviceSize end, const PVulkanFrameTask& task) const {
        AddPendingState(state, begin, end, PVulkanFrameTask{ task });
    }
    void AddPendingState(EResourceState state, VkDeviceSize begin, VkDeviceSize end, PVulkanFrameTask&& rtask) const {
        AddPendingState(FBufferState{ state, { begin, end }, std::move(rtask) });
    }

    VkBufferView MakeView(const FVulkanDevice& device, const FBufferViewDesc& desc) const {
        return _bufferData->MakeView(device, desc);
    }

private:
    const FVulkanBuffer* _bufferData{ nullptr };

    mutable FAccessRecords _accessPending;
    mutable FAccessRecords _accessForRead;
    mutable FAccessRecords _accessForWrite;

    bool _isImmutable{ false };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
