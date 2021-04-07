#pragma once

#include "Vulkan_fwd.h"

#include "RHI/FrameGraph.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanFrameGraph final : public IFrameGraph {
public:
    FVulkanFrameGraph(FVulkanDevice* pDevice, VkSurfaceKHR mainWindow) NOEXCEPT;
    ~FVulkanFrameGraph() override;


    FVulkanDevice& Device() const { return (*_pDevice); }
    VkSurfaceKHR MainWindow() const { return (_mainWindow); }

    static bool Create(PVulkanFrameGraph* pfg, FVulkanInstance& instance, ERHIFeature features, FWindowHandle mainWindow);

    // IFrameGraph

    void TearDown() override;

    ETargetRHI TargetRHI() const override { return ETargetRHI::Vulkan; }
    void* ExternalDevice() const override;

    bool AddPipelineCompiler(const PPipelineCompiler& pcompiler) override;

    EQueueUsage AvailableQueues() const override;

    FMPipelineID CreatePipeline(FMeshPipelineDesc& desc ARGS_IF_RHIDEBUG(const FStringView& name)) override;
    FRTPipelineID CreatePipeline(FRayTracingPipelineDesc& desc ARGS_IF_RHIDEBUG(const FStringView& name)) override;
    FGPipelineID CreatePipeline(FGraphicsPipelineDesc& desc ARGS_IF_RHIDEBUG(const FStringView& name)) override;
    FCPipelineID CreatePipeline(FComputePipelineDesc& desc ARGS_IF_RHIDEBUG(const FStringView& name)) override;
    FImageID CreateImage(const FImageDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(const FStringView& name)) override;
    FImageID CreateImage(const FImageDesc& desc, const FMemoryDesc& mem, EResourceState defaultState ARGS_IF_RHIDEBUG(const FStringView& name)) override;
    FBufferID CreateBuffer(const FBufferDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(const FStringView& name)) override;
    FSamplerID CreateSampler(const FSamplerDesc& desc ARGS_IF_RHIDEBUG(const FStringView& name)) override;
    FSwapchainID CreateSwapchain(const FSwapchainDesc&, FRawSwapchainID oldSwapchain ARGS_IF_RHIDEBUG(const FStringView& name)) override;
    FRTGeometryID CreateRayTracingGeometry(const FRayTracingGeometryDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(const FStringView& name)) override;
    FRTSceneID CreateRayTracingScene(const FRayTracingSceneDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(const FStringView& name)) override;
    FRTShaderTableID CreateRayTracingShaderTable(ARG0_IF_RHIDEBUG(const FStringView& name)) override;

    bool InitPipelineResources(FRawGPipelineID pipeline, const FDescriptorSetID& id, const PPipelineResources& presources) const override;
    bool InitPipelineResources(FRawCPipelineID pipeline, const FDescriptorSetID& id, const PPipelineResources& presources) const override;
    bool InitPipelineResources(FRawMPipelineID pipeline, const FDescriptorSetID& id, const PPipelineResources& presources) const override;
    bool InitPipelineResources(FRawRTPipelineID pipeline, const FDescriptorSetID& id, const PPipelineResources& presources) const override;

    bool IsSupported(FRawImageID image, const FImageViewDesc& desc) const override;
    bool IsSupported(FRawBufferID buffer, const FBufferViewDesc& desc) const override;
    bool IsSupported(const FImageDesc& desc, EMemoryType memType) const override;
    bool IsSupported(const FBufferDesc& desc, EMemoryType memType) const override;

    bool CachePipelineResources(FPipelineResources& resources) override;

    void ReleaseResource(FPipelineResources& resources) override;
    bool ReleaseResource(FGPipelineID& id) override;
    bool ReleaseResource(FCPipelineID& id) override;
    bool ReleaseResource(FMPipelineID& id) override;
    bool ReleaseResource(FRTPipelineID& id) override;
    bool ReleaseResource(FImageID& id) override;
    bool ReleaseResource(FBufferID& id) override;
    bool ReleaseResource(FSamplerID& id) override;
    bool ReleaseResource(FSwapchainID& id) override;
    bool ReleaseResource(FRTGeometryID& id) override;
    bool ReleaseResource(FRTSceneID& id) override;
    bool ReleaseResource(FRTShaderTableID& id) override;

    const FBufferDesc& Description(FRawBufferID id) const override;
    const FImageDesc& Description(FRawImageID id) const override;

    void* ExternalDescription(FRawBufferID id) const override;
    void* ExternalDescription(FRawImageID id) const override;

    bool IsResourceAlive(FRawGPipelineID id) const override;
    bool IsResourceAlive(FRawCPipelineID id) const override;
    bool IsResourceAlive(FRawMPipelineID id) const override;
    bool IsResourceAlive(FRawRTPipelineID id) const override;
    bool IsResourceAlive(FRawImageID id) const override;
    bool IsResourceAlive(FRawBufferID id) const override;
    bool IsResourceAlive(FRawSwapchainID id) const override;
    bool IsResourceAlive(FRawRTGeometryID id) const override;
    bool IsResourceAlive(FRawRTSceneID id) const override;
    bool IsResourceAlive(FRawRTShaderTableID id) const override;

    FGPipelineID AcquireResource(FRawGPipelineID id) override;
    FCPipelineID AcquireResource(FRawCPipelineID id) override;
    FMPipelineID AcquireResource(FRawMPipelineID id) override;
    FRTPipelineID AcquireResource(FRawRTPipelineID id) override;
    FImageID AcquireResource(FRawImageID id) override;
    FBufferID AcquireResource(FRawBufferID id) override;
    FSwapchainID AcquireResource(FRawSwapchainID id) override;
    FRTGeometryID AcquireResource(FRawRTGeometryID id) override;
    FRTSceneID AcquireResource(FRawRTSceneID id) override;
    FRTShaderTableID AcquireResource(FRawRTShaderTableID id) override;

    bool UpdateHostBuffer(FRawBufferID id, size_t offset, size_t size, const void* data) override;
    bool MapBufferRange(FRawBufferID id, size_t offset, size_t& size, void** data) override;

    FCommandBufferBatch Begin(const FCommandBufferDesc&, TMemoryView<const FCommandBufferBatch> dependsOn) override;
    bool Execute(FCommandBufferBatch&) override;
    bool Wait(TMemoryView<const FCommandBufferBatch> commands, FNanoseconds timeout) override;
    bool Flush(EQueueUsage queues) override;
    bool WaitIdle() override;

    bool DumpStatistics(FFrameStatistics* pstats) const override;

private:
    FVulkanDevice* _pDevice;
    VkSurfaceKHR _mainWindow;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
