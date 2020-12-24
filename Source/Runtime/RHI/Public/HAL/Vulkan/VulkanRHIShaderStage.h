#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHIShaderStage.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------//----------------------------------------------------------------------------
enum class EVulkanShaderStageFlags : u32 {
    Vertex = VK_SHADER_STAGE_VERTEX_BIT,
    TesselationControl = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    TesselationEvaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    Geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
    Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
    Compute = VK_SHADER_STAGE_COMPUTE_BIT,

    AllGraphics = Vertex|TesselationControl|TesselationEvaluation|Geometry|Fragment|Compute,

    RayCast = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
    RayAnyHit = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
    RayClosestHit = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
    RayMiss = VK_SHADER_STAGE_MISS_BIT_KHR,
    RayIntersection = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
    RayCallable = VK_SHADER_STAGE_CALLABLE_BIT_KHR,

    AllRaytrace = RayCast|RayAnyHit|RayClosestHit|RayMiss|RayIntersection|RayCallable,

    Mesh = VK_SHADER_STAGE_MESH_BIT_NV,
    Task = VK_SHADER_STAGE_TASK_BIT_NV,

    AllMeshShading = Mesh|Task,
};
ENUM_FLAGS(EVulkanShaderStageFlags);
//----------------------------------------------------------------------------
enum class EVulkanShaderStageCreateFlags : u32 {
    None = 0,
    AllowVaryingSubgroupSize = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT,
    RequireFullSubgroups = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT,
};
ENUM_FLAGS(EVulkanShaderStageCreateFlags);
//----------------------------------------------------------------------------
struct FVulkanShaderStage {
    EVulkanShaderStageFlags Stage;
    EVulkanShaderStageCreateFlags CreateFlags;

    FStringView EntryPoint;

    FVulkanShaderSpecialization Specialization;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
