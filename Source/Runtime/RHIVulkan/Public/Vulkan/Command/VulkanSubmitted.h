#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/FrameDebug.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanSubmitted final : Meta::FNonCopyableNorMovable {
public:
    STATIC_CONST_INTEGRAL(u32, MaxBatches, 32);
    STATIC_CONST_INTEGRAL(u32, MaxSemaphores, 8);

    using FBatches = TFixedSizeStack<PVulkanCommandBatch, MaxBatches>;
    using FSemaphores = TFixedSizeStack<VkSemaphore, MaxSemaphores>;

    struct FInternalSubmit {
        FBatches Batches;
        FSemaphores Semaphores;
        VkFence Fence;
        EQueueType QueueType;
    };

    explicit FVulkanSubmitted(u32 indexInPool) NOEXCEPT;
    ~FVulkanSubmitted();

    u32 IndexInPool() const { return _indexInPool; }

    auto Read() const { return _submit.LockShared(); }
    auto Write() { return _submit.LockExclusive(); }

private:
    friend class FVulkanFrameGraph;
    void Create_(const FVulkanDevice& device, EQueueType queue, TMemoryView<const PVulkanCommandBatch> batches, TMemoryView<const VkSemaphore> semaphores);
    void Release_(const FVulkanDevice& device ARGS_IF_RHIDEBUG(FVulkanDebugger& debugger, const FShaderDebugCallback& callback, FFrameStatistics& stats));
    void TearDown_(const FVulkanDevice& device);

    const u32 _indexInPool;

    TRHIThreadSafe<FInternalSubmit> _submit;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
