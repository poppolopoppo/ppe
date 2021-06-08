#include "stdafx.h"

#include "RHI/FrameGraph.h"

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
#endif //!USE_PPE_RHIDEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
