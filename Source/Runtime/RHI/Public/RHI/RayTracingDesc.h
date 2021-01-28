#pragma once

#include "RHI_fwd.h"
#include "VertexDesc.h"

#include "RHI/ResourceId.h"
#include "RHI/RayTracingEnums.h"
#include "RHI/VertexEnums.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FRayTracingGeometryDesc {
    using EFlags = ERayTracingGeometryFlags;

    struct FTriangles {
        FGeometryID GeometryId;
        EFlags Flags{Default};

        u32 VertexCount{0};
        EVertexFormat VertexFormat{ Default };

        // optional:
        u32 IndexCount{0};
        EIndexFormat IndexFormat{ Default };

        FTriangles() = default;

        explicit FTriangles(const FGeometryID& geometryId) : GeometryId(geometryId) {}

        FTriangles& SetId(const FGeometryID& id) {
            GeometryId = id;
            return (*this);
        }

        FTriangles& SetFlag(EFlags value) {
            Flags = Flags + value;
            return (*this);
        }

        template <typename _Vertex>
        FTriangles& SetVertices(u32 count) { return SetVertices(count, VertexAttrib<_Vertex>()); }
        FTriangles& SetVertices(u32 count, EVertexFormat fmt) {
            VertexCount = count;
            VertexFormat = fmt;
            return (*this);
        }

        FTriangles& SetIndices(u32 count) { return SetIndices(count, count <= TNumericLimits<u16>::MaxValue() ? EIndexFormat::UShort : EIndexFormat::UInt); }
        FTriangles& SetIndices(u32 count, EIndexFormat fmt) {
            IndexCount = count;
            IndexFormat = fmt;
            return (*this);
        }

    };

    struct FBoundingVolumes {
        FGeometryID GeometryId;
        EFlags Flags{Default};
        u32 AabbCount{0};

        FBoundingVolumes() = default;

        explicit FBoundingVolumes(const FGeometryID& geometryId) : GeometryId(geometryId) {}

        FBoundingVolumes& SetId(const FGeometryID& id) {
            GeometryId = id;
            return (*this);
        }

        FBoundingVolumes& SetFlag(EFlags value) {
            Flags = Flags + value;
            return (*this);
        }

        FBoundingVolumes& SetCount(u32 count) {
            AabbCount = count;
            return (*this);
        }
    };

    TMemoryView<const FTriangles> Triangles;
    TMemoryView<const FBoundingVolumes> Aabbs;
    ERayTracingPolicyFlags Flags{Default};

    FRayTracingGeometryDesc() = default;

    explicit FRayTracingGeometryDesc(TMemoryView<const FTriangles> triangles) : Triangles(triangles) {}
    explicit FRayTracingGeometryDesc(TMemoryView<const FBoundingVolumes> aabbs) : Aabbs(aabbs) {}

    FRayTracingGeometryDesc(TMemoryView<const FTriangles> triangles, TMemoryView<const FBoundingVolumes> aabbs) : Triangles(triangles), Aabbs(aabbs) {}

};
//----------------------------------------------------------------------------
struct FRayTracingSceneDesc {
    u32 MaxInstanceCount{ 0 };
    ERayTracingPolicyFlags Flags{ Default };

    FRayTracingSceneDesc() = default;

    explicit FRayTracingSceneDesc(u32 instanceCount, ERayTracingPolicyFlags flags = Default) : MaxInstanceCount(instanceCount), Flags(flags) {}

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
