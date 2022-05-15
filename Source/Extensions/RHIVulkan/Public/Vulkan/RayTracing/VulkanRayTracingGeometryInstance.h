#pragma once

#include "Vulkan/VulkanCommon.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanRayTracingGeometryInstance {
    // 4x3 row-major matrix
    float4 TransformRow0;
    float4 TransformRow1;
    float4 TransformRow2;

    u32 CustomIndex     : 24;
    u32 Mask            :  8;

    u32 InstanceOffset  : 24;
    u32 Flags           :  8;

    FVulkanBLASHandle BlasHandle;
};
//----------------------------------------------------------------------------
struct FVulkanRayTracingSceneInstance {
    FInstanceID InstanceId;
    FRTGeometryID GeometryId;
    u32 IndexOffset{ UMax };

    bool operator ==(const FVulkanRayTracingSceneInstance& other) const { return operator ==(other.InstanceId); }
    bool operator !=(const FVulkanRayTracingSceneInstance& other) const { return operator !=(other.InstanceId); }

    bool operator < (const FVulkanRayTracingSceneInstance& other) const { return operator < (other.InstanceId); }
    bool operator >=(const FVulkanRayTracingSceneInstance& other) const { return operator >=(other.InstanceId); }

    bool operator ==(const FInstanceID& other) const { return (InstanceId == other); }
    bool operator !=(const FInstanceID& other) const { return (not operator ==(other)); }

    bool operator < (const FInstanceID& other) const { return (InstanceId < other); }
    bool operator >=(const FInstanceID& other) const { return (not operator < (other)); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
