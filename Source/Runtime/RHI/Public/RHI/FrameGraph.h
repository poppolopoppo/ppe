#pragma once

#include "RHI_fwd.h"

#include "HAL/TargetRHI.h"

#include "RHI/CommandBufferRef.h"
#include "RHI/MemoryDesc.h"
#include "RHI/ResourceEnums.h"
#include "RHI/ResourceId.h"
#include "RHI/ResourceState.h"

#include "IO/String_fwd.h"
#include "Maths/Units.h"
#include "Memory/RefPtr.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FFrameGraphStatistics {
    struct FRendering {
        u32 NumDescriptorBinds              = 0;
        u32 NumPushConstants                = 0;
        u32 NumPipelineBarriers             = 0;
        u32 NumTransferOps                  = 0;

        u32 NumIndexBufferBindings          = 0;
        u32 NumVertexBufferBindings         = 0;
        u32 NumDrawCalls                    = 0;
        u64 NumVertexCount                  = 0;
        u64 NumPrimitiveCount               = 0;
        u32 NumGraphicsPipelineBindings     = 0;
        u32 NumDynamicStateChanges          = 0;

        u32 NumDispatchCalls                = 0;
        u32 NumComputePipelineBindings      = 0;

        u32 NumRayTracingPipelineBindings   = 0;
        u32 NumTraceRaysCalls               = 0;
        u32 NumBuildASCalls                 = 0;

        FNanoseconds GpuTime{ 0 };
        FNanoseconds CpuTime{ 0 };

        FNanoseconds SubmittingTime{ 0 };
        FNanoseconds WaitingTime{ 0 };
    };

    struct FResources {
        u32 NumNewGraphicsPipeline          = 0;
        u32 NumNewComputePipeline           = 0;
        u32 NumNewRayTracingPipeline        = 0;
    };

    FRendering Renderer;
    FResources Resources;

    void Merge(const FFrameGraphStatistics& other);
};
//----------------------------------------------------------------------------
class IFrameGraphTask : public FRefCountable {
protected:
    virtual ~IFrameGraphTask() = default;
};
//----------------------------------------------------------------------------
class PPE_RHI_API IFrameGraph : public FRefCountable {
public: // interface
    virtual ~IFrameGraph() = default;

    // Returns api-specific device handle with which framegraph has been crated.
    virtual ETargetRHI TargetRHI() const = 0;
    virtual void* ExternalDevice() const = 0;

    // De-initialize instance systems.
    // Shared pointer may prevent object destruction in specified place,
    // so use this method to destroy all resources and release systems.
    virtual void TearDown() = 0;

    // Add pipeline compiler.
    // By default pipelines may be created from SPIRV binary and doesn't extract reflection.
    // External compilers can build SPIRV binary from source and extract reflection.
    virtual bool AddPipelineCompiler(const PPipelineCompiler& pcompiler) = 0;

    // Returns bitmask for all available queues.
    virtual EQueueUsage AvailableQueues() const = 0;

    // Resource Manager

    // Create resources: pipeline, image, buffer, etc.
    // See synchronization requirements on top of this file.
    virtual FMPipelineID CreatePipeline(FMeshPipelineDesc& desc, FStringView dbgName = Default) = 0;
    virtual FRTPipelineID CreatePipeline(FRayTracingPipelineDesc& desc, FStringView dbgName = Default) = 0;
    virtual FGPipelineID CreatePipeline(FGraphicsPipelineDesc& desc, FStringView dbgName = Default) = 0;
    virtual FCPipelineID CreatePipeline(FComputePipelineDesc& desc, FStringView dbgName = Default) = 0;
    virtual FImageID CreateImage(const FImageDesc& desc, const FMemoryDesc& mem = Default, FStringView dbgName = Default) = 0;
    virtual FImageID CreateImage(const FImageDesc& desc, const FMemoryDesc& mem, EResourceState defaultState, FStringView dbgName = Default) = 0;
    virtual FBufferID CreateBuffer(const FBufferDesc& desc, const FMemoryDesc& mem = Default, FStringView dbgName = Default) = 0;
    virtual FSamplerID CreateSampler(const FSamplerDesc& desc, FStringView dbgName = Default) = 0;
    virtual FSwapchainID CreateSwapchain(const FSwapchainDesc&, FRawSwapchainID oldSwapchain = Default, FStringView dbgName = Default) = 0;
    virtual FRTGeometryID CreateRayTracingGeometry(const FRayTracingGeometryDesc& desc, const FMemoryDesc& mem = Default, FStringView dbgName = Default) = 0;
    virtual FRTSceneID CreateRayTracingScene(const FRayTracingSceneDesc& desc, const FMemoryDesc& mem = Default, FStringView dbgName = Default) = 0;
    virtual FRTShaderTableID CreateRayTracingShaderTable(FStringView dbgName = Default) = 0;

    virtual bool InitPipelineResources(FRawGPipelineID pipeline, const FDescriptorSetID& id, const PPipelineResources& presources) const = 0;
    virtual bool InitPipelineResources(FRawCPipelineID pipeline, const FDescriptorSetID& id, const PPipelineResources& presources) const = 0;
    virtual bool InitPipelineResources(FRawMPipelineID pipeline, const FDescriptorSetID& id, const PPipelineResources& presources) const = 0;
    virtual bool InitPipelineResources(FRawRTPipelineID pipeline, const FDescriptorSetID& id, const PPipelineResources& presources) const = 0;

    virtual bool IsSupported(FRawImageID image, const FImageViewDesc& desc) const = 0;
    virtual bool IsSupported(FRawBufferID buffer, const FBufferViewDesc& desc) const = 0;
    virtual bool IsSupported(const FImageDesc& desc, EMemoryType memType = EMemoryType::Default) const = 0;
    virtual bool IsSupported(const FBufferDesc& desc, EMemoryType memType = EMemoryType::Default) const = 0;

    // Creates internal descriptor set and release dynamically allocated memory in the 'resources'.
    // After that your can not modify the 'resources', but you still can use it in the tasks.
    virtual bool CachePipelineResources(FPipelineResources& resources) = 0;
    virtual void ReleaseResource(FPipelineResources& resources) = 0;

    // Release reference to resource, Returns 'true' if resource has been deleted.
    // See synchronization requirements on top of this file.
    virtual bool ReleaseResource(FGPipelineID& id) = 0;
    virtual bool ReleaseResource(FCPipelineID& id) = 0;
    virtual bool ReleaseResource(FMPipelineID& id) = 0;
    virtual bool ReleaseResource(FRTPipelineID& id) = 0;
    virtual bool ReleaseResource(FImageID& id) = 0;
    virtual bool ReleaseResource(FBufferID& id) = 0;
    virtual bool ReleaseResource(FSamplerID& id) = 0;
    virtual bool ReleaseResource(FSwapchainID& id) = 0;
    virtual bool ReleaseResource(FRTGeometryID& id) = 0;
    virtual bool ReleaseResource(FRTSceneID& id) = 0;
    virtual bool ReleaseResource(FRTShaderTableID& id) = 0;

    // Returns resource description.
    virtual const FBufferDesc& Description(FRawBufferID id) const = 0;
    virtual const FImageDesc& Description(FRawImageID id) const = 0;

    // api-specific variant (#TODO)
    virtual void* ExternalDescription(FRawBufferID id) const = 0;
    virtual void* ExternalDescription(FRawImageID id) const = 0;

    // Returns 'true' if resource is not deleted.
    virtual bool IsResourceAlive(FRawGPipelineID id) const = 0;
    virtual bool IsResourceAlive(FRawCPipelineID id) const = 0;
    virtual bool IsResourceAlive(FRawMPipelineID id) const = 0;
    virtual bool IsResourceAlive(FRawRTPipelineID id) const = 0;
    virtual bool IsResourceAlive(FRawImageID id) const = 0;
    virtual bool IsResourceAlive(FRawBufferID id) const = 0;
    virtual bool IsResourceAlive(FRawSwapchainID id) const = 0;
    virtual bool IsResourceAlive(FRawRTGeometryID id) const = 0;
    virtual bool IsResourceAlive(FRawRTSceneID id) const = 0;
    virtual bool IsResourceAlive(FRawRTShaderTableID id) const = 0;

    // Returns strong reference to resource if it valid, otherwise returns invalid ID.
    virtual FGPipelineID AcquireResource(FRawGPipelineID id) = 0;
    virtual FCPipelineID AcquireResource(FRawCPipelineID id) = 0;
    virtual FMPipelineID AcquireResource(FRawMPipelineID id) = 0;
    virtual FRTPipelineID AcquireResource(FRawRTPipelineID id) = 0;
    virtual FImageID AcquireResource(FRawImageID id) = 0;
    virtual FBufferID AcquireResource(FRawBufferID id) = 0;
    virtual FSwapchainID AcquireResource(FRawSwapchainID id) = 0;
    virtual FRTGeometryID AcquireResource(FRawRTGeometryID id) = 0;
    virtual FRTSceneID AcquireResource(FRawRTSceneID id) = 0;
    virtual FRTShaderTableID AcquireResource(FRawRTShaderTableID id) = 0;

    // Copy data into host-visible memory.
    virtual bool UpdateHostBuffer(FRawBufferID id, size_t offset, size_t size, const void* data) = 0;

    // Returns pointer to host-visible memory.
    virtual bool MapBufferRange(FRawBufferID id, size_t offset, size_t& size, void** data) = 0;

    // Frame execution

    // Begin command buffer recording.
    virtual FCommandBufferRef Begin(const FCommandBufferDesc&, TMemoryView<FCommandBufferRef> dependsOn = {}) = 0;

    // Compile framegraph for current command buffer and append it to the pending command buffer queue (that are waiting for submitting to GPU).
    virtual bool Execute(FCommandBufferRef&) = 0;

    // Wait until all commands complete execution on the GPU or until time runs out.
    virtual bool Wait(TMemoryView<FCommandBufferRef> commands, FNanoseconds timeout = FNanoseconds{3600'000'000'000}) = 0;

    // Submit all pending command buffers and present all pending swapchain images.
    virtual bool Flush(EQueueUsage queues = EQueueUsage::All) = 0;

    // Wait until all commands will complete their work on GPU, trigger events for 'FReadImage' and 'FReadBuffer' tasks.
    virtual bool WaitIdle() = 0;

    // Debugging
    virtual bool DumpStatistics(FFrameGraphStatistics* pstats) const = 0;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
