#include "stdafx.h"

#include "RHI/FrameDebug.h"

#if USE_PPE_RHIDEBUG && USE_PPE_RHITRACE
#   include "HAL/PlatformProcess.h"
#   include <atomic>
#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
//----------------------------------------------------------------------------
FFrameStatistics::FRendering& FFrameStatistics::FRendering::operator +=(const FRendering& other) NOEXCEPT {
    NumDescriptorBinds += other.NumDescriptorBinds;
    NumPushConstants += other.NumDescriptorBinds;
    NumPipelineBarriers += other.NumDescriptorBinds;
    NumTransferOps += other.NumDescriptorBinds;
    NumIndexBufferBindings += other.NumDescriptorBinds;
    NumVertexBufferBindings += other.NumDescriptorBinds;
    NumDrawCalls += other.NumDescriptorBinds;
    NumVertexCount += other.NumDescriptorBinds;
    NumPrimitiveCount += other.NumDescriptorBinds;
    NumGraphicsPipelineBindings += other.NumDescriptorBinds;
    NumDynamicStateChanges += other.NumDescriptorBinds;
    NumDispatchCalls += other.NumDescriptorBinds;
    NumComputePipelineBindings += other.NumDescriptorBinds;
    NumRayTracingPipelineBindings += other.NumDescriptorBinds;
    NumTraceRaysCalls += other.NumDescriptorBinds;
    NumBuildASCalls += other.NumDescriptorBinds;
    GpuTime += other.NumDescriptorBinds;
    CpuTime += other.NumDescriptorBinds;
    SubmittingTime += other.NumDescriptorBinds;
    WaitingTime += other.NumDescriptorBinds;
    return (*this);
}
//----------------------------------------------------------------------------
FFrameStatistics::FResources& FFrameStatistics::FResources::operator +=(const FResources& other) NOEXCEPT {
    NumNewGraphicsPipeline += other.NumNewGraphicsPipeline;
    NumNewComputePipeline += other.NumNewGraphicsPipeline;
    NumNewRayTracingPipeline += other.NumNewGraphicsPipeline;
    return (*this);
}
//----------------------------------------------------------------------------
#endif //!#if USE_PPE_RHIDEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
//----------------------------------------------------------------------------
FDebugColorScheme::FDebugColorScheme() NOEXCEPT
  : RenderPass{ FLinearColor::OrangeRed() },
    Compute{ FLinearColor::MediumBlue() },
    DeviceLocalTransfer{ FLinearColor::Green() },
    HostToDeviceTransfer{ FLinearColor::BlueViolet() },
    DeviceToHostTransfer{ FLinearColor::BlueViolet() },
    Present{ FLinearColor::Red() },
    RayTracing{ FLinearColor::Lime() },
    BuildRayTracingStruct{ FLinearColor::Lime() },
    Draw{ FLinearColor::Bisque() },
    DrawMeshes{ FLinearColor::Bisque() },
    CustomDraw{ FLinearColor::Bisque() },
    CmdSubBatchBackground{ FLinearColor::SlateBlue() },
    CmdSubBatchLabel{ FLinearColor::LightGray() },
    CmbBatchBackground{ FLinearColor::Aquamarine() },
    CmdBatchLabel{ FLinearColor::LightGray() },
    TaskLabel{ FLinearColor::White() },
    ResourceBackground{ FLinearColor::Silver() },
    BarrierGroupBorder{ FLinearColor::Olive() },
    GroupBorder{ FLinearColor::DarkGray() },
    TaskDependency{ FLinearColor::GhostWhite() },
    Debug{ FLinearColor::Pink() },
    LayoutBarrier{ FLinearColor::Yellow() },
    WriteAfterWriteBarrier{ FLinearColor::DodgerBlue() },
    WriteAfterReadBarrier{ FLinearColor::LimeGreen() },
    ReadAfterWriteBarrier{ FLinearColor::Red() }
{

}
//----------------------------------------------------------------------------
const FDebugColorScheme& FDebugColorScheme::Get() NOEXCEPT {
    // #TODO: handle profiles / user themes ?
    static const FDebugColorScheme GInstance;
    return GInstance;
}
//----------------------------------------------------------------------------
#endif //!#if USE_PPE_RHIDEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG && USE_PPE_RHITRACE
//----------------------------------------------------------------------------
namespace {
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment
struct CACHELINE_ALIGNED FRHITraceUID_ {
    std::atomic<u64> Next{ 0 };
    std::atomic<u64> Break{ UINT64_MAX };

    static FRHITraceUID_& Get() NOEXCEPT {
        static FRHITraceUID_ GInstance;
        return GInstance;
    }
};
PRAGMA_MSVC_WARNING_POP() // Padding
} //!namespace
//----------------------------------------------------------------------------
u64 FFrameStatistics::NextTraceUID() {
    auto& rhiTrace = FRHITraceUID_::Get();
    const u64 uid = rhiTrace.Next.fetch_add(1, std::memory_order_relaxed);
    if (rhiTrace.Break.load(std::memory_order_relaxed) == uid)
        PPE_DEBUG_BREAK();
    return uid;
}
//----------------------------------------------------------------------------
void FFrameStatistics::SetBreakOnTraceUID(u64 uid) {
    auto& rhiTrace = FRHITraceUID_::Get();
    u64 expected = rhiTrace.Break.load(std::memory_order_relaxed);
    for (i32 backoff = 0;;) {
        if (rhiTrace.Break.compare_exchange_strong(expected, uid))
            break;
        FPlatformProcess::SleepForSpinning(backoff);
    }
}
//----------------------------------------------------------------------------
#endif //!#if USE_PPE_RHIDEBUG && USE_PPE_RHITRACE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
