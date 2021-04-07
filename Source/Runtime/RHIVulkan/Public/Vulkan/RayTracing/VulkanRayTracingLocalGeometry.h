#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/RayTracing/VulkanRayTracingGeometry.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanRayTracingLocalGeometry final : Meta::FNonCopyableNorMovable {
public:
    struct FGeometryState {
        EResourceState State{ Default };
        PFrameTask Task;
    };

    using FAabbs = FVulkanRayTracingGeometry::FAabbs;
    using FTriangles = FVulkanRayTracingGeometry::FTriangles;

    struct FGeometryAccess {
        VkPipelineStageFlags Stages{ 0 };
        VkAccessFlags Access{ 0 };
        EVulkanExecutionOrder Index{ EVulkanExecutionOrder::Initial };
        bool IsReadable : 1;
        bool IsWritable : 1;

        FGeometryAccess() NOEXCEPT : IsReadable(false), IsWritable(false) {}

        bool Valid() const { return !!(IsReadable | IsWritable); }
    };

    FVulkanRayTracingLocalGeometry() = default;
    ~FVulkanRayTracingLocalGeometry();

    bool Create(const FVulkanRayTracingGeometry* geometryData);
    void TearDown();

    void AddPendingState(const FGeometryState& state) const;
    void CommitBarrier(FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default));
    void ResetState(EVulkanExecutionOrder index, FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default));

    const SCVulkanRayTracingGeometry& GlobalData() const { return _rtGeomtry; }
    auto InternalData() const { return _rtGeomtry->Read(); }

    FVulkanBLASHandle BLAS() const { return _rtGeomtry->BLAS(); }
    VkAccelerationStructureKHR Handle() const { return _rtGeomtry->Handle(); }

    TMemoryView<const FAabbs> Aabbs() const { return _rtGeomtry->Aabbs(); }
    TMemoryView<const FTriangles> Triangles() const { return _rtGeomtry->Triangles(); }
    ERayTracingBuildFlags Flags() const { return _rtGeomtry->Flags(); }
    u32 MaxGeometryCount() const { return checked_cast<u32>(Triangles().size() + Aabbs().size()); }

private:
    SCVulkanRayTracingGeometry _rtGeomtry;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
