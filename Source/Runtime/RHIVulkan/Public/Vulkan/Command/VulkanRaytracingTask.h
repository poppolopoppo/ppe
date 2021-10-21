#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Command/VulkanFrameTask.h"

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
    const FVulkanRayTracingLocalScene* RTScene;

    FVulkanRayTracingShaderTable* ShaderTable;
    const TMemoryView<const FShaderGroup> ShaderGroups;

    const FRayGenShader RayGenShader;
    const u32 MaxRecursionDepth;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FUpdateRayTracingShaderTable& desc, FProcessFunc process);

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
    using FUsableBuffers = HASHSET_SLAB(RHICommand, const FVulkanLocalBuffer*);

    const FVulkanRayTracingLocalGeometry* RTGeometry{ nullptr };
    const FVulkanLocalBuffer* ScratchBuffer{ nullptr };
    const TMemoryView<const VkAccelerationStructureGeometryKHR> Geometries;

    FUsableBuffers UsableBuffers;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBuildRayTracingGeometry& desc, FProcessFunc process);

    bool Valid() const { return (!!RTGeometry && !!ScratchBuffer && not Geometries.empty()); }

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
    struct FInstance {
        FInstanceID InstanceId;
        FRTGeometryID GeometryId;
        u32 Index{ 0 };
    };

    const FVulkanRayTracingLocalScene* RTScene{ nullptr };
    const FVulkanLocalBuffer* ScratchBuffer{ nullptr };

    const FVulkanLocalBuffer* InstanceStagingBuffer{ nullptr };
    VkDeviceSize InstanceStagingBufferOffset{ 0 };

    const FVulkanLocalBuffer* InstanceBuffer{ nullptr };
    VkDeviceSize InstanceBufferOffset{ 0 };

    TMemoryView<const FInstance> Instances;
    TMemoryView<const FVulkanRayTracingLocalGeometry*> RTGeometries;

    u32 NumInstances{ 0 };
    u32 HitShadersPerInstance{ 0 };
    u32 MaxHitShaderCount{ 0 };

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBuildRayTracingScene& desc, FProcessFunc process);

    bool Valid() const {
        Assert_NoAssume(Instances.size() == NumInstances);
        Assert_NoAssume(RTGeometries.size() == NumInstances);
        return (!!RTScene && !!ScratchBuffer && !!InstanceStagingBuffer && InstanceBuffer && not Instances.empty() && not RTGeometries.empty());
    }

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
    const FVulkanRayTracingShaderTable* ShaderTable;
    const FPushConstantDatas PushConstants;
    const uint3 GroupCount;

#if USE_PPE_RHIDEBUG
    EShaderDebugIndex DebugModeIndex{ Default };
#endif

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FTraceRays& desc, FProcessFunc process);

    bool Valid() const { return (!!ShaderTable && !!GroupCount.x && !!GroupCount.y && !!GroupCount.z); }

    const FVulkanPipelineResourceSet& Resources() const { return _resources; }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif

private:
    FVulkanPipelineResourceSet _resources;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
