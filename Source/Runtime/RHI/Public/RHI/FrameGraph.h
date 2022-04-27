#pragma once

#include "RHI_fwd.h"

#include "HAL/TargetRHI.h"

#include "RHI/CommandBatch.h"
#include "RHI/DeviceProperties.h"
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
template <typename T>
class TAutoResource;
//----------------------------------------------------------------------------
class PPE_RHI_API IFrameTask {
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

    // Returns bitmask for all available queues.
    virtual EQueueUsage AvailableQueues() const NOEXCEPT = 0;

    // Returns device features, properties and limits.
    // Some parameters in commands must comply with these restrictions.
    virtual FDeviceProperties DeviceProperties() const NOEXCEPT = 0;

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

    NODISCARD virtual bool IsSupported(FRawImageID image, const FImageViewDesc& desc) const NOEXCEPT = 0;
    NODISCARD virtual bool IsSupported(FRawBufferID buffer, const FBufferViewDesc& desc) const NOEXCEPT = 0;
    NODISCARD virtual bool IsSupported(const FImageDesc& desc, EMemoryType memType = EMemoryType::Default) const NOEXCEPT = 0;
    NODISCARD virtual bool IsSupported(const FBufferDesc& desc, EMemoryType memType = EMemoryType::Default) const NOEXCEPT = 0;

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
    NODISCARD virtual const FImageDesc& Description(FRawImageID id) const = 0;
    NODISCARD virtual const FBufferDesc& Description(FRawBufferID id) const = 0;

    NODISCARD virtual FImageID CreateImage(
        const FImageDesc& desc,
        FExternalImage externalImage,
        FOnReleaseExternalImage&& onRelease,
        TMemoryView<const u32> queueFamilyIndices,
        EResourceState defaultState
        ARGS_IF_RHIDEBUG(FConstChar debugName)) = 0;
    NODISCARD virtual FBufferID CreateBuffer(
        const FBufferDesc& desc,
        FExternalBuffer externalBuffer,
        FOnReleaseExternalBuffer&& onRelease,
        TMemoryView<const u32> queueFamilyIndices
        ARGS_IF_RHIDEBUG(FConstChar debugName)) = 0;

    // api-specific variant
    NODISCARD virtual FExternalBuffer ExternalDescription(FRawBufferID id) const NOEXCEPT = 0;
    NODISCARD virtual FExternalImage ExternalDescription(FRawImageID id) const NOEXCEPT = 0;

    // Returns 'true' if resource is not deleted.
    NODISCARD virtual bool IsResourceAlive(FRawGPipelineID id) const NOEXCEPT = 0;
    NODISCARD virtual bool IsResourceAlive(FRawCPipelineID id) const NOEXCEPT = 0;
    NODISCARD virtual bool IsResourceAlive(FRawMPipelineID id) const NOEXCEPT = 0;
    NODISCARD virtual bool IsResourceAlive(FRawRTPipelineID id) const NOEXCEPT = 0;
    NODISCARD virtual bool IsResourceAlive(FRawImageID id) const NOEXCEPT = 0;
    NODISCARD virtual bool IsResourceAlive(FRawBufferID id) const NOEXCEPT = 0;
    NODISCARD virtual bool IsResourceAlive(FRawSwapchainID id) const NOEXCEPT = 0;
    NODISCARD virtual bool IsResourceAlive(FRawRTGeometryID id) const NOEXCEPT = 0;
    NODISCARD virtual bool IsResourceAlive(FRawRTSceneID id) const NOEXCEPT = 0;
    NODISCARD virtual bool IsResourceAlive(FRawRTShaderTableID id) const NOEXCEPT = 0;

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

    // Must be called before every frame for duty cycle
    virtual void PrepareNewFrame() = 0;

    // Begin command buffer recording.
    NODISCARD virtual FCommandBufferBatch Begin(const FCommandBufferDesc& desc, TMemoryView<const SCommandBatch> dependsOn = {}) = 0;

    // Compile framegraph for current command buffer and append it to the pending command buffer queue (that are waiting for submitting to GPU).
    virtual bool Execute(FCommandBufferBatch& cmdBatch) = 0;

    // Wait until all commands complete execution on the GPU or until time runs out.
    virtual bool Wait(TMemoryView<const SCommandBatch> commands, FNanoseconds timeout = MaxTimeout) = 0;

    // Submit all pending command buffers and present all pending swapchain images.
    virtual bool Flush(EQueueUsage queues = EQueueUsage::All) = 0;

    // Wait until all commands will complete their work on GPU, trigger events for 'FReadImage' and 'FReadBuffer' tasks.
    virtual bool WaitIdle(FNanoseconds timeout = MaxTimeout) = 0;

    static CONSTEXPR FNanoseconds MaxTimeout{ 60'000'000'000 };

    // Debugging
#if USE_PPE_RHIDEBUG
    virtual void LogFrame() const = 0;
    NODISCARD virtual bool DumpFrame(FStringBuilder* log) const = 0;
    NODISCARD virtual bool DumpGraph(FStringBuilder* log) const = 0;
    virtual bool DumpStatistics(FFrameStatistics* pStats) const = 0;
#endif

public: // helpers
    template <typename _RawId>
    NODISCARD TAutoResource<details::TResourceWrappedId<_RawId>> ScopedResource(details::TResourceWrappedId<_RawId>&& resource);

    template <typename _Id0, typename... _Ids>
    void ReleaseResources(_Id0& resource0, _Ids&... resources) {
        (void)ReleaseResource(resource0);
        IF_CONSTEXPR(sizeof...(_Ids) > 0) {
            ReleaseResources(std::forward<_Ids&>(resources)...);
        }
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _RawId>
class TAutoResource< details::TResourceWrappedId<_RawId> > {
public:
    TAutoResource() = default;

    TAutoResource(TAutoResource&&) = default;
    TAutoResource& operator =(TAutoResource&&) = default;

    TAutoResource(const TAutoResource&) = delete;
    TAutoResource& operator =(const TAutoResource&) = delete;

    TAutoResource(IFrameGraph& fg, details::TResourceWrappedId<_RawId>&& resource) NOEXCEPT
    :   _frameGraph(&fg)
    ,   _resource(std::move(resource)) {
    }

    ~TAutoResource() {
        if (_resource.Valid())
            Unused(_frameGraph->ReleaseResource(_resource));
    }

    bool Valid() const { return _resource.Valid(); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid();  }

    _RawId Raw() const { return *_resource; }
    operator _RawId () const { return *_resource; }
    operator const details::TResourceWrappedId<_RawId>& () const { return _resource; }

    details::TResourceWrappedId<_RawId>& Get() { return _resource; }
    const details::TResourceWrappedId<_RawId>& Get() const { return _resource; }

    details::TResourceWrappedId<_RawId>& operator *() { return _resource; }
    details::TResourceWrappedId<_RawId>* operator ->() { return &_resource; }

    const details::TResourceWrappedId<_RawId>& operator *() const { return _resource; }
    const details::TResourceWrappedId<_RawId>* operator ->() const { return &_resource; }

private:
    SFrameGraph _frameGraph;
    details::TResourceWrappedId<_RawId> _resource;
};
//----------------------------------------------------------------------------
template <typename _RawId>
TAutoResource(IFrameGraph&, details::TResourceWrappedId<_RawId>&&) -> TAutoResource<_RawId>;
//----------------------------------------------------------------------------
template <typename _RawId>
TAutoResource<details::TResourceWrappedId<_RawId>> IFrameGraph::ScopedResource(details::TResourceWrappedId<_RawId>&& resource) {
    return TAutoResource<details::TResourceWrappedId<_RawId>>{ *this, std::move(resource) };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
