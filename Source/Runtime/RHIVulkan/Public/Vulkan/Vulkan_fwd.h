#pragma once

#include "RHIVulkan_fwd.h"

#include "Vulkan/VulkanIncludes_fwd.h"

#include "RHI/Config.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Buffer:
//----------------------------------------------------------------------------
FWD_REFPTR(VulkanBuffer);
class FVulkanLocalBuffer;
//----------------------------------------------------------------------------
// Command:
//----------------------------------------------------------------------------
FWD_REFPTR(VulkanBarrierManager);
FWD_REFPTR(VulkanCommandBatch);
FWD_REFPTR(VulkanCommandBuffer);
FWD_REFPTR(VulkanCommandPool);
class FVulkanSubmitted;
template <typename _VulkanTask>
class TVulkanDrawTask;
class IVulkanDrawTask;
template <typename _VulkanTask>
class TVulkanFrameTask;
class IVulkanFrameTask;
using PVulkanFrameTask = TPtrRef<IVulkanDrawTask>;
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
using PVulkanDeviceQueue = TPtrRef<FVulkanDeviceQueue>;
class FVulkanDevice;
FWD_REFPTR(VulkanResourceManager);
struct FVulkanSwapchainDesc;
FWD_REFPTR(VulkanSwapchain);
//----------------------------------------------------------------------------
// Image:
//----------------------------------------------------------------------------
FWD_REFPTR(VulkanImage);
class FVulkanLocalImage;
class FVulkanSampler;
//----------------------------------------------------------------------------
// Memory:
//----------------------------------------------------------------------------
FWD_REFPTR(VulkanMemoryManager);
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
FWD_REFPTR(VulkanRayTracingGeometry);
class FVulkanRayTracingLocalGeometry;
FWD_REFPTR(VulkanRayTracingScene);
class FVulkanRayTracingLocalScene;
class FVulkanRayTracingShaderTable;
//----------------------------------------------------------------------------
// RenderPass:
//----------------------------------------------------------------------------
class FVulkanFramebuffer;
class FVulkanLogicalRenderPass;
class FVulkanRenderPass;
//----------------------------------------------------------------------------
// Debugging:
//----------------------------------------------------------------------------
FWD_REFPTR(VulkanDebugger); // #TODO %_NOCOMMIT%
class FVulkanLocalDebugger; // #TODO %_NOCOMMIT%
//----------------------------------------------------------------------------
// Framegraph:
//----------------------------------------------------------------------------
FWD_REFPTR(VulkanFrameGraph);
using FVulkanWindowHandle = void*;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
