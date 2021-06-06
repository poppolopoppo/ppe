#pragma once

#include "RHI_fwd.h"

#include "HAL/TargetRHI.h"

#include "RHI/CommandBatch.h"
#include "RHI/FrameDebug.h"
#include "RHI/MemoryDesc.h"
#include "RHI/ResourceEnums.h"
#include "RHI/ResourceId.h"
#include "RHI/ResourceState.h"

#include "IO/String_fwd.h"
#include "Memory/RefPtr.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IFrameTask {
protected:
    IFrameTask() = default;
public:
    virtual ~IFrameTask() = default;
};
//----------------------------------------------------------------------------
class PPE_RHI_API IFrameGraph : public FRefCountable {
protected:
    IFrameGraph() = default;
public: // interface
    virtual ~IFrameGraph() = default;

    // Returns api-specific device handle with which framegraph has been crated.
    virtual ETargetRHI TargetRHI() const NOEXCEPT = 0;
    virtual void* ExternalDevice() const NOEXCEPT = 0;

    // De-initialize instance systems.
    // Shared pointer may prevent object destruction in specified place,
    // so use this method to destroy all resources and release systems.
    virtual void TearDown() = 0;

    // Release cached memory
    virtual void ReleaseMemory() NOEXCEPT = 0;

    // Add pipeline compiler.
    // By default pipelines may be created from SPIRV binary and doesn't extract reflection.
    // External compilers can build SPIRV binary from source and extract reflection.
    virtual bool AddPipelineCompiler(const PPipelineCompiler& pcompiler) = 0;

    // Returns bitmask for all available queues.
    virtual EQueueUsage AvailableQueues() const NOEXCEPT = 0;

#if USE_PPE_RHIDEBUG
    // Callback will be called at end of the frame if debugging enabled by
    // calling 'Task::EnableDebugTrace' and shader compiled with 'EShaderLangFormat::EnableDebugTrace' flag.
    virtual bool SetShaderDebugCallback(FShaderDebugCallback&& rcallback) = 0;
#endif

    // --- Resource Manager ---

    // Create resources: pipeline, image, buffer, etc.
    // See synchronization requirements on top of this file.
    NODISCARD virtual FMPipelineID CreatePipeline(FMeshPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) = 0;
    NODISCARD virtual FRTPipelineID CreatePipeline(FRayTracingPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) = 0;
    NODISCARD virtual FGPipelineID CreatePipeline(FGraphicsPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) = 0;
    NODISCARD virtual FCPipelineID CreatePipeline(FComputePipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) = 0;
    NODISCARD virtual FImageID CreateImage(const FImageDesc& desc, const FMemoryDesc& mem = Default ARGS_IF_RHIDEBUG(FConstChar debugName = Default)) = 0;
    NODISCARD virtual FImageID CreateImage(const FImageDesc& desc, const FMemoryDesc& mem, EResourceState defaultState ARGS_IF_RHIDEBUG(FConstChar debugName)) = 0;
    NODISCARD virtual FBufferID CreateBuffer(const FBufferDesc& desc, const FMemoryDesc& mem = Default ARGS_IF_RHIDEBUG(FConstChar debugName = Default)) = 0;
    NODISCARD virtual FSamplerID CreateSampler(const FSamplerDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) = 0;
    NODISCARD virtual FSwapchainID CreateSwapchain(const FSwapchainDesc& desc, FRawSwapchainID oldSwapchain = Default ARGS_IF_RHIDEBUG(FConstChar debugName = Default)) = 0;
    NODISCARD virtual FRTGeometryID CreateRayTracingGeometry(const FRayTracingGeometryDesc& desc, const FMemoryDesc& mem = Default ARGS_IF_RHIDEBUG(FConstChar debugName = Default)) = 0;
    NODISCARD virtual FRTSceneID CreateRayTracingScene(const FRayTracingSceneDesc& desc, const FMemoryDesc& mem = Default ARGS_IF_RHIDEBUG(FConstChar debugName = Default)) = 0;
    NODISCARD virtual FRTShaderTableID CreateRayTracingShaderTable(ARG0_IF_RHIDEBUG(const FStringView& name)) = 0;

    NODISCARD virtual bool InitPipelineResources(FPipelineResources* pResources, FRawGPipelineID pipeline, const FDescriptorSetID& id) const = 0;
    NODISCARD virtual bool InitPipelineResources(FPipelineResources* pResources, FRawCPipelineID pipeline, const FDescriptorSetID& id) const = 0;
    NODISCARD virtual bool InitPipelineResources(FPipelineResources* pResources, FRawMPipelineID pipeline, const FDescriptorSetID& id) const = 0;
    NODISCARD virtual bool InitPipelineResources(FPipelineResources* pResources, FRawRTPipelineID pipeline, const FDescriptorSetID& id) const = 0;

    virtual bool IsSupported(FRawImageID image, const FImageViewDesc& desc) const NOEXCEPT = 0;
    virtual bool IsSupported(FRawBufferID buffer, const FBufferViewDesc& desc) const NOEXCEPT = 0;
    virtual bool IsSupported(const FImageDesc& desc, EMemoryType memType = EMemoryType::Default) const NOEXCEPT = 0;
    virtual bool IsSupported(const FBufferDesc& desc, EMemoryType memType = EMemoryType::Default) const NOEXCEPT = 0;

    // Creates internal descriptor set and release dynamically allocated memory in the 'resources'.
    // After that your can not modify the 'resources', but you still can use it in the tasks.
    NODISCARD virtual bool CachePipelineResources(FPipelineResources& resources) = 0;
    virtual void ReleaseResource(FPipelineResources& resources) = 0;

    // Release reference to resource, Returns 'true' if resource has been deleted.
    // See synchronization requirements on top of this file.
    NODISCARD virtual bool ReleaseResource(FGPipelineID& id) = 0;
    NODISCARD virtual bool ReleaseResource(FCPipelineID& id) = 0;
    NODISCARD virtual bool ReleaseResource(FMPipelineID& id) = 0;
    NODISCARD virtual bool ReleaseResource(FRTPipelineID& id) = 0;
    NODISCARD virtual bool ReleaseResource(FImageID& id) = 0;
    NODISCARD virtual bool ReleaseResource(FBufferID& id) = 0;
    NODISCARD virtual bool ReleaseResource(FSamplerID& id) = 0;
    NODISCARD virtual bool ReleaseResource(FSwapchainID& id) = 0;
    NODISCARD virtual bool ReleaseResource(FRTGeometryID& id) = 0;
    NODISCARD virtual bool ReleaseResource(FRTSceneID& id) = 0;
    NODISCARD virtual bool ReleaseResource(FRTShaderTableID& id) = 0;

    // Returns resource description.
    virtual const FImageDesc& Description(FRawImageID id) const = 0;
    virtual const FBufferDesc& Description(FRawBufferID id) const = 0;

    NODISCARD virtual FImageID CreateImage(
        const FImageDesc& desc,
        FExternalImage externalImage,
        FOnReleaseExternalImage&& onRelease,
        TMemoryView<const u32> queueFamilyIndices
        ARGS_IF_RHIDEBUG(FConstChar debugName)) = 0;
    NODISCARD virtual FBufferID CreateBuffer(
        const FBufferDesc& desc,
        FExternalBuffer externalBuffer,
        FOnReleaseExternalBuffer&& onRelease,
        TMemoryView<const u32> queueFamilyIndices
        ARGS_IF_RHIDEBUG(FConstChar debugName)) = 0;

    // api-specific variant (#TODO)
    virtual const void* ExternalDescription(FRawBufferID id) const NOEXCEPT = 0;
    virtual const void* ExternalDescription(FRawImageID id) const NOEXCEPT = 0;

    // Returns 'true' if resource is not deleted.
    virtual bool IsResourceAlive(FRawGPipelineID id) const NOEXCEPT = 0;
    virtual bool IsResourceAlive(FRawCPipelineID id) const NOEXCEPT = 0;
    virtual bool IsResourceAlive(FRawMPipelineID id) const NOEXCEPT = 0;
    virtual bool IsResourceAlive(FRawRTPipelineID id) const NOEXCEPT = 0;
    virtual bool IsResourceAlive(FRawImageID id) const NOEXCEPT = 0;
    virtual bool IsResourceAlive(FRawBufferID id) const NOEXCEPT = 0;
    virtual bool IsResourceAlive(FRawSwapchainID id) const NOEXCEPT = 0;
    virtual bool IsResourceAlive(FRawRTGeometryID id) const NOEXCEPT = 0;
    virtual bool IsResourceAlive(FRawRTSceneID id) const NOEXCEPT = 0;
    virtual bool IsResourceAlive(FRawRTShaderTableID id) const NOEXCEPT = 0;

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
    NODISCARD virtual bool UpdateHostBuffer(FRawBufferID id, size_t offset, size_t size, const void* data) = 0;

    // Returns pointer to host-visible memory.
    NODISCARD virtual bool MapBufferRange(FRawBufferID id, size_t offset, size_t& size, void** data) = 0;

    // --- Frame execution ---

    // Begin command buffer recording.
    virtual FCommandBufferBatch Begin(const FCommandBufferDesc& desc, TMemoryView<const FCommandBufferBatch> dependsOn = {}) = 0;

    // Compile framegraph for current command buffer and append it to the pending command buffer queue (that are waiting for submitting to GPU).
    NODISCARD virtual bool Execute(FCommandBufferBatch& cmdBatch) = 0;

    // Wait until all commands complete execution on the GPU or until time runs out.
    NODISCARD virtual bool Wait(TMemoryView<const FCommandBufferBatch> commands, FNanoseconds timeout = FNanoseconds{3600'000'000'000}) = 0;

    // Submit all pending command buffers and present all pending swapchain images.
    NODISCARD virtual bool Flush(EQueueUsage queues = EQueueUsage::All) = 0;

    // Wait until all commands will complete their work on GPU, trigger events for 'FReadImage' and 'FReadBuffer' tasks.
    NODISCARD virtual bool WaitIdle() = 0;

    // Debugging
#if USE_PPE_RHIPROFILING
    virtual bool DumpStatistics(FFrameStatistics* pStats) const = 0;
#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
