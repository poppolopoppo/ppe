#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Command/VulkanFrameTask.h"
#include "Vulkan/RayTracing/VulkanRayTracingGeometryInstance.h"
#include "Vulkan/RayTracing/VulkanRayTracingScene.h"

#include "RHI/RayTracingTask.h"

#include "Allocator/SlabAllocator.h"
#include "Container/HashSet.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FUpdateRayTracingShaderTable:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FUpdateRayTracingShaderTable> final : public IVulkanFrameTask {
public:
    using FRayGenShader = FUpdateRayTracingShaderTable::FRayGenShader;
    using FShaderGroup = FUpdateRayTracingShaderTable::FShaderGroup;

    const FRawRTPipelineID Pipeline;
    const u32 MaxRecursionDepth;
    const TPtrRef<const FVulkanRayTracingLocalScene> RTScene;

    const TPtrRef<FVulkanRayTracingShaderTable> ShaderTable;
    const TMemoryView<const FShaderGroup> ShaderGroups;

    const FRayGenShader RayGenShader;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FUpdateRayTracingShaderTable& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!Pipeline && !!RTScene && !!ShaderTable); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
};
//----------------------------------------------------------------------------
// FBuildRayTracingGeometry:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FBuildRayTracingGeometry> final : public IVulkanFrameTask {
public:
    using FUsableBuffers = HASHSET_SLAB(RHICommand, TPtrRef<const FVulkanLocalBuffer>);

    TPtrRef<const FVulkanRayTracingLocalGeometry> RTGeometry{ nullptr };
    TPtrRef<const FVulkanLocalBuffer> ScratchBuffer{ nullptr };
    TMemoryView<const VkGeometryNV> Geometries;

    FUsableBuffers UsableBuffers;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBuildRayTracingGeometry& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return true/*(!!RTGeometry && !!ScratchBuffer && not Geometries.empty())*/; }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
};
//----------------------------------------------------------------------------
// FBuildRayTracingScene:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FBuildRayTracingScene> final : public IVulkanFrameTask {
public:
    using FInstance = FVulkanRayTracingSceneInstance;

    TPtrRef<const FVulkanRayTracingLocalScene> RTScene{ nullptr };
    TPtrRef<const FVulkanLocalBuffer> ScratchBuffer{ nullptr };

    TPtrRef<const FVulkanLocalBuffer> InstanceStagingBuffer{ nullptr };
    VkDeviceSize InstanceStagingBufferOffset{ 0 };

    const FVulkanLocalBuffer* InstanceBuffer{ nullptr };
    VkDeviceSize InstanceBufferOffset{ 0 };

    TMemoryView<FInstance> Instances;
    TMemoryView<TPtrRef<const FVulkanRayTracingLocalGeometry>> RTGeometries;

    u32 NumInstances{ 0 };
    u32 HitShadersPerInstance{ 0 };
    u32 MaxHitShaderCount{ 0 };

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBuildRayTracingScene& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const {
        //Assert_NoAssume(Instances.size() == NumInstances);
        //Assert_NoAssume(RTGeometries.size() == NumInstances);
        //return (!!RTScene && !!ScratchBuffer && !!InstanceStagingBuffer && InstanceBuffer && not Instances.empty() && not RTGeometries.empty());
        return true; // the resources are allocated *AFTER* the task
    }

    VkDeviceSize InstanceBufferSizeInBytes() const { return NumInstances * sizeof(FVulkanRayTracingGeometryInstance); }
    VkDeviceSize ScratchBufferOffset() const { return 0; }
    VkDeviceSize ScratchBufferSizeInBytes() const { return VK_WHOLE_SIZE; }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
};
//----------------------------------------------------------------------------
// FTraceRays:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FTraceRays> final : public IVulkanFrameTask {
public:
    const TPtrRef<const FVulkanRayTracingShaderTable> ShaderTable;
    const TMemoryView<const FPushConstantData> PushConstants;

    const uint3 GroupCount;

#if USE_PPE_RHIDEBUG
    EShaderDebugIndex DebugModeIndex{ Default };
#endif

    using FResource = FVulkanPipelineResourceSet::FResource;

    TMemoryView<u32> DynamicOffsets;
    TMemoryView<const FResource> Resources;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FTraceRays& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!ShaderTable && !!GroupCount.x && !!GroupCount.y && !!GroupCount.z); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
