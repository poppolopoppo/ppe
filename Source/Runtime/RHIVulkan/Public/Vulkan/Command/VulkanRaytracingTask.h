#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Command/VulkanFrameTask.h"

#include "RHI/RayTracingTask.h"

#include "Allocator/LinearAllocator.h"
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
    const FVulkanRayTracingLocalScene* const RTScene;

    FVulkanRayTracingShaderTable* const ShaderTable;
    const TMemoryView<const FShaderGroup> ShaderGroups;

    const FRayGenShader RayGenShader;
    const u32 MaxRecursionDepth;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FUpdateRayTracingShaderTable& desc, FProcessFunc process);

    bool Valid() const { return (!!Pipeline && !!RTScene && !!ShaderTable); }
};
//----------------------------------------------------------------------------
// FBuildRayTracingGeometry:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FBuildRayTracingGeometry> final : public IVulkanFrameTask {
public:
    using FUsableBuffers = HASHSET_LINEARHEAP(const FVulkanLocalBuffer*);

    const FVulkanRayTracingLocalGeometry* const RTGeometry;
    const FVulkanLocalBuffer* const ScratchBuffer;
    const TMemoryView<const VkAccelerationStructureGeometryKHR> Geometries;

    FUsableBuffers UsableBuffers;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBuildRayTracingGeometry& desc, FProcessFunc process);

    bool Valid() const { return (!!RTGeometry && !!ScratchBuffer && not Geometries.empty()); }
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

    const FVulkanRayTracingLocalScene* const RTScene;
    const FVulkanLocalBuffer* const ScratchBuffer;

    const FVulkanLocalBuffer* const InstanceStagingBuffer;
    const VkDeviceSize InstanceStagingBufferOffset;

    const FVulkanLocalBuffer* const InstanceBuffer;
    const VkDeviceSize InstanceBufferOffset;

    const TMemoryView<const FInstance> Instances;
    const TMemoryView<const FVulkanRayTracingLocalGeometry* const> RTGeometries;

    u32 HitShadersPerInstance{ 0 };
    u32 MaxHitShaderCount{ 0 };

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBuildRayTracingScene& desc, FProcessFunc process);

    bool Valid() const { return (!!RTScene && !!ScratchBuffer && !!InstanceStagingBuffer && InstanceBuffer && not Instances.empty() && not RTGeometries.empty()); }
};
//----------------------------------------------------------------------------
// FTraceRays:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FTraceRays> final : public IVulkanFrameTask {
public:
    const FVulkanRayTracingShaderTable* const ShaderTable;
    const FPushConstantDatas PushConstants;
    const uint3 GroupCount;

#if USE_PPE_RHIDEBUG
    EShaderDebugIndex DebugModeIndex{ Default };
#endif

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FTraceRays& desc, FProcessFunc process);

    bool Valid() const { return (!!ShaderTable && !!GroupCount.x && !!GroupCount.y && !!GroupCount.z); }

    const FVulkanPipelineResourceSet& Resources() const { return _resources; }

private:
    FVulkanPipelineResourceSet _resources;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
