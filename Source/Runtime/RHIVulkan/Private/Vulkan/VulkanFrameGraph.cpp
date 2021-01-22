#include "stdafx.h"

#include "Vulkan/VulkanFrameGraph.h"

#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanInstance.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanFrameGraph::FVulkanFrameGraph(FVulkanDevice* pDevice, FVulkanWindowSurface mainWindow) NOEXCEPT
:   _pDevice(std::move(pDevice))
,   _mainWindow(mainWindow) {
    Assert(_pDevice);
}
//----------------------------------------------------------------------------
FVulkanFrameGraph::~FVulkanFrameGraph() {
    Assert(nullptr == _pDevice);
    Assert(nullptr == _mainWindow);
}
//----------------------------------------------------------------------------
// Create/TearDown
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::Create(PVulkanFrameGraph* pfg, FVulkanInstance& instance, ERHIFeature features, FWindowHandle mainWindow) {
    Assert(pfg);

    VkSurfaceKHR vkSurface{ VK_NULL_HANDLE };
    if (mainWindow)
        vkSurface = instance.CreateWindowSurface(mainWindow);

    FVulkanDevice* const pDevice = instance.CreateLogicalDevice(features, vkSurface);)
    if (not pDevice) {
        if (vkSurface)
            instance.DestroyWindowSurface(vkSurface);
        return false;
    }

    *pfg = NEW_REF(RHIMisc, FVulkanFrameGraph, pDevice, vkSurface);
    Assert(*pfg);
    return true;
}
//----------------------------------------------------------------------------
void FVulkanFrameGraph::TearDown() {
    Assert(_pDevice);

    FVulkanInstance& instance = _pDevice->Instance();

    instance.DestroyLogicalDevice(_pDevice);
    _pDevice = nullptr;

    if (_mainWindow) {
        instance.DestroyWindowSurface(_mainWindow);
        _mainWindow = VK_NULL_HANDLE;
    }
}
//----------------------------------------------------------------------------
// Misc
//----------------------------------------------------------------------------
void* FVulkanFrameGraph::ExternalDevice() const {
    return _pDevice->vkDevice();
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::AddPipelineCompiler(const PPipelineCompiler& pcompiler) {}
//----------------------------------------------------------------------------
EQueueUsage FVulkanFrameGraph::AvailableQueues() const {}
//----------------------------------------------------------------------------
// CreatePipeline
//----------------------------------------------------------------------------
FMPipelineID FVulkanFrameGraph::CreatePipeline(FMeshPipelineDesc& desc, FStringView dbgName) {}
//----------------------------------------------------------------------------
FRTPipelineID FVulkanFrameGraph::CreatePipeline(FRayTracingPipelineDesc& desc, FStringView dbgName) {}
//----------------------------------------------------------------------------
FGPipelineID FVulkanFrameGraph::CreatePipeline(FGraphicsPipelineDesc& desc, FStringView dbgName) {}
//----------------------------------------------------------------------------
FCPipelineID FVulkanFrameGraph::CreatePipeline(FComputePipelineDesc& desc, FStringView dbgName) {}
//----------------------------------------------------------------------------
// Create resources
//----------------------------------------------------------------------------
FImageID FVulkanFrameGraph::CreateImage(const FImageDesc& desc, const FMemoryDesc& mem, FStringView dbgName) {}
//----------------------------------------------------------------------------
FImageID FVulkanFrameGraph::CreateImage(const FImageDesc& desc, const FMemoryDesc& mem, EResourceState defaultState, FStringView dbgName) {}
//----------------------------------------------------------------------------
FBufferID FVulkanFrameGraph::CreateBuffer(const FBufferDesc& desc, const FMemoryDesc& mem, FStringView dbgName) {}
//----------------------------------------------------------------------------
FSamplerID FVulkanFrameGraph::CreateSampler(const FSamplerDesc& desc, FStringView dbgName) {}
//----------------------------------------------------------------------------
FSwapchainID FVulkanFrameGraph::CreateSwapchain(const FSwapchainDesc&, FRawSwapchainID oldSwapchain, FStringView dbgName) {}
//----------------------------------------------------------------------------
FRTGeometryID FVulkanFrameGraph::CreateRayTracingGeometry(const FRayTracingGeometryDesc& desc, const FMemoryDesc& mem, FStringView dbgName) {}
//----------------------------------------------------------------------------
FRTSceneID FVulkanFrameGraph::CreateRayTracingScene(const FRayTracingSceneDesc& desc, const FMemoryDesc& mem, FStringView dbgName) {}
//----------------------------------------------------------------------------
FRTShaderTableID FVulkanFrameGraph::CreateRayTracingShaderTable(FStringView dbgName) {}
//----------------------------------------------------------------------------
// InitPipelineResources
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::InitPipelineResources(FRawGPipelineID pipeline, const FDescriptorSetID& id, const PPipelineResources& presources) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::InitPipelineResources(FRawCPipelineID pipeline, const FDescriptorSetID& id, const PPipelineResources& presources) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::InitPipelineResources(FRawMPipelineID pipeline, const FDescriptorSetID& id, const PPipelineResources& presources) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::InitPipelineResources(FRawRTPipelineID pipeline, const FDescriptorSetID& id, const PPipelineResources& presources) const {}
//----------------------------------------------------------------------------
// IsSupported
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsSupported(FRawImageID image, const FImageViewDesc& desc) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsSupported(FRawBufferID buffer, const FBufferViewDesc& desc) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsSupported(const FImageDesc& desc, EMemoryType memType) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsSupported(const FBufferDesc& desc, EMemoryType memType) const {}
//----------------------------------------------------------------------------
// CachePipelineResources
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::CachePipelineResources(FPipelineResources& resources) {}
//----------------------------------------------------------------------------
// ReleaseResource
//----------------------------------------------------------------------------
void FVulkanFrameGraph::ReleaseResource(FPipelineResources& resources) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::ReleaseResource(FGPipelineID& id) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::ReleaseResource(FCPipelineID& id) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::ReleaseResource(FMPipelineID& id) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::ReleaseResource(FRTPipelineID& id) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::ReleaseResource(FImageID& id) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::ReleaseResource(FBufferID& id) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::ReleaseResource(FSamplerID& id) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::ReleaseResource(FSwapchainID& id) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::ReleaseResource(FRTGeometryID& id) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::ReleaseResource(FRTSceneID& id) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::ReleaseResource(FRTShaderTableID& id) {}
//----------------------------------------------------------------------------
// Description
//----------------------------------------------------------------------------
const FBufferDesc& FVulkanFrameGraph::Description(FRawBufferID id) const {}
//----------------------------------------------------------------------------
const FImageDesc& FVulkanFrameGraph::Description(FRawImageID id) const {}
//----------------------------------------------------------------------------
void* FVulkanFrameGraph::ExternalDescription(FRawBufferID id) const {}
//----------------------------------------------------------------------------
void* FVulkanFrameGraph::ExternalDescription(FRawImageID id) const {}
//----------------------------------------------------------------------------
// IsResourceAlive
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsResourceAlive(FRawGPipelineID id) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsResourceAlive(FRawCPipelineID id) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsResourceAlive(FRawMPipelineID id) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsResourceAlive(FRawRTPipelineID id) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsResourceAlive(FRawImageID id) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsResourceAlive(FRawBufferID id) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsResourceAlive(FRawSwapchainID id) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsResourceAlive(FRawRTGeometryID id) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsResourceAlive(FRawRTSceneID id) const {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsResourceAlive(FRawRTShaderTableID id) const {}
//----------------------------------------------------------------------------
// AcquireResource
//----------------------------------------------------------------------------
FGPipelineID FVulkanFrameGraph::AcquireResource(FRawGPipelineID id) {}
//----------------------------------------------------------------------------
FCPipelineID FVulkanFrameGraph::AcquireResource(FRawCPipelineID id) {}
//----------------------------------------------------------------------------
FMPipelineID FVulkanFrameGraph::AcquireResource(FRawMPipelineID id) {}
//----------------------------------------------------------------------------
FRTPipelineID FVulkanFrameGraph::AcquireResource(FRawRTPipelineID id) {}
//----------------------------------------------------------------------------
FImageID FVulkanFrameGraph::AcquireResource(FRawImageID id) {}
//----------------------------------------------------------------------------
FBufferID FVulkanFrameGraph::AcquireResource(FRawBufferID id) {}
//----------------------------------------------------------------------------
FSwapchainID FVulkanFrameGraph::AcquireResource(FRawSwapchainID id) {}
//----------------------------------------------------------------------------
FRTGeometryID FVulkanFrameGraph::AcquireResource(FRawRTGeometryID id) {}
//----------------------------------------------------------------------------
FRTSceneID FVulkanFrameGraph::AcquireResource(FRawRTSceneID id) {}
//----------------------------------------------------------------------------
FRTShaderTableID FVulkanFrameGraph::AcquireResource(FRawRTShaderTableID id) {}
//----------------------------------------------------------------------------
// Memory transfer
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::UpdateHostBuffer(FRawBufferID id, size_t offset, size_t size, const void* data) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::MapBufferRange(FRawBufferID id, size_t offset, size_t& size, void** data) {}
//----------------------------------------------------------------------------
// Commands
//----------------------------------------------------------------------------
FCommandBufferRef FVulkanFrameGraph::Begin(const FCommandBufferDesc&, TMemoryView<FCommandBufferRef> dependsOn) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::Execute(FCommandBufferRef&) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::Wait(TMemoryView<FCommandBufferRef> commands, FNanoseconds timeout) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::Flush(EQueueUsage queues) {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::WaitIdle() {}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::DumpStatistics(FFrameGraphStatistics* pstats) const {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
