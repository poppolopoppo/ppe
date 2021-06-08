#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/RayTracingDesc.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanRayTracingGeometry final {
public:
    using EFlags = ERayTracingGeometryFlags;

    struct FTriangles {
        FGeometryID GeometryId;
        EFlags Flags{ Default };
        u32 MaxVertexCount{ 0 };
        EVertexFormat VertexFormat{ Default };
        u32 MaxIndexCount{ 0 };
        EIndexFormat IndexFormat{ Default };
        u16 VertexSize{ 0 };
        u16 IndexSize{ 0 };

        FTriangles() = default;

        bool operator < (const FTriangles& other) const { return GeometryId < other.GeometryId; }
        bool operator >=(const FTriangles& other) const { return (not operator < (other)); }

        bool operator < (const FGeometryID& id) const { return GeometryId < id; }
        bool operator >=(const FGeometryID& id) const { return (not operator < (id)); }

        bool operator ==(const FGeometryID& id) const { return (GeometryId == id); }
        bool operator !=(const FGeometryID& id) const { return (GeometryId != id); }
    };

    struct FAabbs {
        FGeometryID GeometryId;
        u32 MaxAabbCount{ 0 };
        EFlags Flags{ Default };

        FAabbs() = default;

        bool operator < (const FAabbs& other) const { return GeometryId < other.GeometryId; }
        bool operator >=(const FAabbs& other) const { return (not operator < (other)); }

        bool operator < (const FGeometryID& id) const { return GeometryId < id; }
        bool operator >=(const FGeometryID& id) const { return (not operator < (id)); }

        bool operator ==(const FGeometryID& id) const { return (GeometryId == id); }
        bool operator !=(const FGeometryID& id) const { return (GeometryId != id); }
    };

    struct FInternalData {
        VkAccelerationStructureKHR BottomLevelAS{ VK_NULL_HANDLE };
        FMemoryID MemoryId;
        FVulkanBLASHandle BLAS{ 0 };
        VECTOR(RHIRayTracing, FTriangles) Triangles;
        VECTOR(RHIRayTracing, FAabbs) Aabbs;
        ERayTracingBuildFlags Flags{ Default };
    };

    FVulkanRayTracingGeometry() = default;
    ~FVulkanRayTracingGeometry();

    auto Read() const { return _data.LockShared(); }

    FVulkanBLASHandle BLAS() const { return Read()->BLAS; }
    VkAccelerationStructureKHR Handle() const { return Read()->BottomLevelAS; }

    TMemoryView<const FAabbs> Aabbs() const { return Read()->Aabbs.MakeConstView(); }
    TMemoryView<const FTriangles> Triangles() const { return Read()->Triangles.MakeConstView(); }
    ERayTracingBuildFlags Flags() const { return Read()->Flags; }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    NODISCARD bool Construct(
        FVulkanResourceManager& resources,
        const FRayTracingGeometryDesc& desc,
        FRawMemoryID memoryId,
        FVulkanMemoryObject& pobj
        ARGS_IF_RHIDEBUG(FConstChar debugName) );
    void TearDown(FVulkanResourceManager& resources);

    size_t GeometryIndex(FGeometryID* pgeometryId) const;

private:
    TRHIThreadSafe<FInternalData> _data;

#if USE_PPE_RHITASKNAME
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
