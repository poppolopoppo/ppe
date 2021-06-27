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

    NODISCARD bool Construct(const FVulkanRayTracingGeometry* geometryData);
    void TearDown();

    void AddPendingState(const FGeometryState& state) const;
    void CommitBarrier(FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default)) const;
    void ResetState(EVulkanExecutionOrder index, FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default));

    const FVulkanRayTracingGeometry* GlobalData() const { return _rtGeometry; }
    auto InternalData() const { return _rtGeometry->Read(); }

    FVulkanBLASHandle BLAS() const { return _rtGeometry->BLAS(); }
    VkAccelerationStructureKHR Handle() const { return _rtGeometry->Handle(); }

    TMemoryView<const FAabbs> Aabbs() const { return _rtGeometry->Aabbs(); }
    TMemoryView<const FTriangles> Triangles() const { return _rtGeometry->Triangles(); }
    ERayTracingBuildFlags Flags() const { return _rtGeometry->Flags(); }
    u32 MaxGeometryCount() const { return checked_cast<u32>(Triangles().size() + Aabbs().size()); }

private:
    FVulkanRayTracingGeometry* _rtGeometry;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
