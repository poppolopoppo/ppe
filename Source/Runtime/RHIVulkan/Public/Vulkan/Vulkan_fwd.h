#pragma once

#include "RHIVulkan_fwd.h"

#include "RHI/Config.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Image:
//----------------------------------------------------------------------------
class FVulkanImage;
class FVulkanLocalImage;
class FVulkanSampler;
//----------------------------------------------------------------------------
// Buffer:
//----------------------------------------------------------------------------
class FVulkanBuffer;
class FVulkanLocalBuffer;
//----------------------------------------------------------------------------
// Command:
//----------------------------------------------------------------------------
class FVulkanBarrierManager;
FWD_REFPTR(VulkanCommandBatch);
FWD_REFPTR(VulkanCommandBuffer);
class FVulkanCommandPool;
class FVulkanSubmitted;
template <typename _VulkanTask>
class TVulkanDrawTask;
class IVulkanDrawTask;
template <typename _VulkanTask>
class TVulkanFrameTask;
class IVulkanFrameTask;
using PVulkanFrameTask = TPtrRef<IVulkanFrameTask>;
class FVulkanTaskProcessor;
//----------------------------------------------------------------------------
// Descriptors:
//----------------------------------------------------------------------------
class FVulkanDescriptorManager;
struct FVulkanDescriptorSet;
class FVulkanDescriptorSetLayout;
class FVulkanPipelineResources;
//----------------------------------------------------------------------------
// Instance:
//----------------------------------------------------------------------------
struct FVulkanDeviceQueueInfo;
struct FVulkanDeviceQueue;
using PVulkanDeviceQueue = TPtrRef<const FVulkanDeviceQueue>;
struct FVulkanDeviceInfo;
class FVulkanDevice;
class FVulkanInstance;
class FVulkanResourceManager;
class FVulkanSwapchain;
FWD_REFPTR(VulkanFrameGraph);
using FVulkanWindowHandle = void*;
//----------------------------------------------------------------------------
// Memory:
//----------------------------------------------------------------------------
class FVulkanMemoryManager;
struct FVulkanMemoryInfo;
class FVulkanMemoryObject;
//----------------------------------------------------------------------------
// Pipeline:
//----------------------------------------------------------------------------
FWD_REFPTR(VulkanComputePipeline);
FWD_REFPTR(VulkanGraphicsPipeline);
FWD_REFPTR(VulkanMeshPipeline);
FWD_REFPTR(VulkanRayTracingPipeline);
class FVulkanPipelineCache;
class FVulkanPipelineLayout;
//----------------------------------------------------------------------------
// Raytracing:
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(u64, FVulkanBLASHandle);
struct FVulkanRayTracingGeometryInstance;
class FVulkanRayTracingGeometry;
class FVulkanRayTracingLocalGeometry;
class FVulkanRayTracingScene;
class FVulkanRayTracingLocalScene;
class FVulkanRayTracingShaderTable;
using FVulkanRTGeometry = FVulkanRayTracingGeometry;
using FVulkanRTLocalGeometry = FVulkanRayTracingLocalGeometry;
using FVulkanRTScene = FVulkanRayTracingScene;
using FVulkanRTLocalScene = FVulkanRayTracingLocalScene;
using FVulkanRTShaderTable = FVulkanRayTracingShaderTable;
//----------------------------------------------------------------------------
// RenderPass:
//----------------------------------------------------------------------------
class FVulkanFramebuffer;
class FVulkanLogicalRenderPass;
class FVulkanRenderPass;
//----------------------------------------------------------------------------
// Debugging:
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
class FVulkanDebugger;
class FVulkanLocalDebugger;
#endif
//----------------------------------------------------------------------------
// Frame tasks:
//----------------------------------------------------------------------------
using FVulkanSubmitRenderPassTask = TVulkanFrameTask<FSubmitRenderPass>;
using FVulkanDispatchComputeTask = TVulkanFrameTask<FDispatchCompute>;
using FVulkanDispatchComputeIndirectTask = TVulkanFrameTask<FDispatchComputeIndirect>;
using FVulkanCopyBufferTask = TVulkanFrameTask<FCopyBuffer>;
using FVulkanCopyImageTask = TVulkanFrameTask<FCopyImage>;
using FVulkanCopyBufferToImageTask = TVulkanFrameTask<FCopyBufferToImage>;
using FVulkanCopyImageToBufferTask = TVulkanFrameTask<FCopyImageToBuffer>;
using FVulkanBlitImageTask = TVulkanFrameTask<FBlitImage>;
using FVulkanResolveImageTask = TVulkanFrameTask<FResolveImage>;
using FVulkanGenerateMipmapsTask = TVulkanFrameTask<FGenerateMipmaps>;
using FVulkanFillBufferTask = TVulkanFrameTask<FFillBuffer>;
using FVulkanClearColorImageTask = TVulkanFrameTask<FClearColorImage>;
using FVulkanClearDepthStencilImageTask = TVulkanFrameTask<FClearDepthStencilImage>;
using FVulkanUpdateBufferTask = TVulkanFrameTask<FUpdateBuffer>;
// using FVulkanUpdateImageTask = TVulkanFrameTask<FUpdateImage>;
// using FVulkanReadBufferTask = TVulkanFrameTask<FReadBuffer>;
// using FVulkanReadImageTask = TVulkanFrameTask<FReadImage>;
using FVulkanPresentTask = TVulkanFrameTask<FPresent>;
using FVulkanCustomTaskTask = TVulkanFrameTask<FCustomTask>;
using FVulkanUpdateRayTracingShaderTableTask = TVulkanFrameTask<FUpdateRayTracingShaderTable>;
using FVulkanBuildRayTracingGeometryTask = TVulkanFrameTask<FBuildRayTracingGeometry>;
using FVulkanBuildRayTracingSceneTask = TVulkanFrameTask<FBuildRayTracingScene>;
using FVulkanTraceRaysTask = TVulkanFrameTask<FTraceRays>;
//----------------------------------------------------------------------------
// Draw tasks:
//----------------------------------------------------------------------------
using FVulkanDrawVerticesTask = TVulkanDrawTask<FDrawVertices>;
using FVulkanDrawIndexedTask = TVulkanDrawTask<FDrawIndexed>;
using FVulkanDrawVerticesIndirectTask = TVulkanDrawTask<FDrawVerticesIndirect>;
using FVulkanDrawIndexedIndirectTask = TVulkanDrawTask<FDrawIndexedIndirect>;
using FVulkanDrawMeshesTask = TVulkanDrawTask<FDrawMeshes>;
using FVulkanDrawMeshesIndirectTask = TVulkanDrawTask<FDrawMeshesIndirect>;
using FVulkanCustomDrawTask = TVulkanDrawTask<FCustomDraw>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
