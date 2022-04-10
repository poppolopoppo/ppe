#pragma once

#include "RHI_fwd.h"

#include "RHI/FrameTask.h"

#include "RHI/RayTracingEnums.h"
#include "RHI/ResourceId.h"
#include "RHI/VertexDesc.h"

#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarMatrix.h"
#include "Meta/Optional.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
struct FRayTracingShaderDebugMode {
    EShaderDebugMode Mode{ Default };
    uint3 LaunchId{ ~0u };
};
#endif
//----------------------------------------------------------------------------
// FBuildRayTracingGeometry
//----------------------------------------------------------------------------
struct FBuildRayTracingGeometry final : details::TFrameTaskDesc<FBuildRayTracingGeometry> {

    struct FTriangles {
        FGeometryID Geometry;

        FRawMemoryConst VertexData;
        FRawBufferID VertexBuffer;
        u32 VertexOffset{ 0 };
        u32 VertexStride{ 0 };
        u32 VertexCount{ 0 };
        EVertexFormat VertexFormat{ Default };

        // optional indices
        FRawMemoryConst IndexData;
        FRawBufferID IndexBuffer;
        u32 IndexOffset{ 0 };
        u32 IndexCount{ 0 };
        EIndexFormat IndexFormat{ Default };

        // optional transforms
        Meta::TOptional<float3x4> TransformData;
        FRawBufferID TransformBuffer; // 3x4 row major affine transformations matrix
        u32 TransformOffset{ 0 };

        FTriangles() = default;
        explicit FTriangles(const FGeometryID& geometry) NOEXCEPT {
            SetGeometry(geometry);
        }

        FTriangles& SetGeometry(const FGeometryID& geometry) { Assert(geometry); Geometry = geometry; return (*this); }

        template <typename T>
        FTriangles& SetVertices(u32 count, u32 stride = 0) { return SetVertices(count, VertexAttrib<T>(), stride); }
        FTriangles& SetVertices(u32 count, EVertexFormat fmt, u32 stride = 0);
        FTriangles& SetVertexBuffer(FRawBufferID buffer, u32 offset = 0);

        template <typename T>
        FTriangles& SetVertexData(TMemoryView<const T> vertices);
        FTriangles& SetVertexData(FRawMemoryConst data);

        FTriangles& SetIndices(u32 count, EIndexFormat fmt);
        FTriangles& SetIndexBuffer(FRawBufferID buffer, u32 offset = 0);

        template <typename T>
        FTriangles& SetIndexData(TMemoryView<const T> indices);
        FTriangles& SetIndexData(FRawMemoryConst data);

        FTriangles& SetTransformData(const float3x4& transform) { TransformData = transform; return (*this); }
        FTriangles& SetTransformBuffer(FRawBufferID buffer, u32 offset);
    };

    using FAabb = FAabb3f;

    struct FBoundingVolumes {
        FGeometryID Geometry;
        FRawMemoryConst AabbData;
        FRawBufferID AabbBuffer;
        u32 AabbOffset{ 0 };
        u32 AabbStride{ 0 };
        u32 AabbCount{ 0 };

        FBoundingVolumes() = default;
        explicit FBoundingVolumes(const FGeometryID& geometry) NOEXCEPT {
            SetGeometry(geometry);
        }

        FBoundingVolumes& SetGeometry(const FGeometryID& geometry) { Assert(geometry); Geometry = geometry; return (*this); }

        FBoundingVolumes& SetCount(u32 count, u32 stride = 0);
        FBoundingVolumes& SetBuffer(FRawBufferID buffer, u32 offset = 0);
        FBoundingVolumes& SetData(FRawMemoryConst data);
    };

    FRawRTGeometryID Geometry;
    TArray<FTriangles> Triangles;
    TArray<FBoundingVolumes> Aabbs;

#if USE_PPE_RHITASKNAME
    FBuildRayTracingGeometry() NOEXCEPT : TFrameTaskDesc<FBuildRayTracingGeometry>("BuildRayTracingGeometry", FDebugColorScheme::Get().BuildRayTracingStruct) {}
#endif

    FBuildRayTracingGeometry& SetTarget(FRawRTGeometryID id) { Assert(id); Geometry = id; return (*this); }
    FBuildRayTracingGeometry& Add(const FTriangles& triangles) { Emplace_Back(Triangles, triangles); return (*this); }
    FBuildRayTracingGeometry& Add(const FBoundingVolumes& aabbs) { Emplace_Back(Aabbs, aabbs); return (*this); }
};
//----------------------------------------------------------------------------
// FBuildRayTracingScene
//----------------------------------------------------------------------------
struct FBuildRayTracingScene final : details::TFrameTaskDesc<FBuildRayTracingScene> {

    using EFlags = ERayTracingInstanceFlags;

    struct FInstance {
        FInstanceID InstanceId;
        FRawRTGeometryID GeometryId;
        float4x3 Transform{ float4x3::Identity() };
        u32 CustomId{ 0 }; // 'gl_InstanceCustomIndexNV' in the shader
        EFlags Flags{ Default };
        u8 Mask{ UMax };

        FInstance() = default;
        explicit FInstance(const FInstanceID& id) NOEXCEPT { SetInstanceId(id); }

        FInstance& SetInstanceId(const FInstanceID& value);
        FInstance& SetGeometry(const FRawRTGeometryID& value);
        FInstance& SetInstanceIndex(u32 value) { CustomId = value; return (*this); }
        FInstance& SetTransform(const float4x3& value) { Transform = value; return (*this); }
        FInstance& AddFlags(EFlags value) { Flags = Flags | value; return (*this); }
        FInstance& SetMask(u8 value) { Mask = value; return (*this); }
    };

    FRawRTSceneID Scene;
    TArray<FInstance> Instances;
    u32 HitShadersPerInstance{ 1 }; // same as 'sbtRecordStride' in ray gen shader

    FBuildRayTracingScene& SetTarget(FRawRTSceneID id) { Assert(id); Scene = id; return (*this); }
    FBuildRayTracingScene& SetHitShadersPerInstance(u32 count) { Assert(count > 0); HitShadersPerInstance = count; return (*this); }
    FBuildRayTracingScene& Add(const FInstance& instance) { Emplace_Back(Instances, instance); return (*this); }
};
//----------------------------------------------------------------------------
// FUpdateRayTracingShaderTable
//----------------------------------------------------------------------------
struct FUpdateRayTracingShaderTable final : details::TFrameTaskDesc<FUpdateRayTracingShaderTable> {

    struct FRayGenShader {
        FRTShaderID Shader;
    };

    using EGroupType = ERayTracingGroupType;

    struct FShaderGroup {
        EGroupType Type{ Default };
        FInstanceID InstanceId;
        FGeometryID GeometryId;
        u32 RecordOffset{ 0 }; // 'sbtRecordOffset'
        FRTShaderID MainShader; // miss or closest hit shader
        FRTShaderID AnyHitShader; // optional
        FRTShaderID IntersectionShader; // procedural geometry

        FShaderGroup(const FRTShaderID& missShader, u32 missIndex) NOEXCEPT
        :   Type(ERayTracingGroupType::MissShader)
        ,   RecordOffset(missIndex)
        ,   MainShader(missShader)
        {}

        FShaderGroup(const FInstanceID& instance, const FGeometryID& geometry, u32 offset, const FRTShaderID& closestHit, const FRTShaderID& anyHit) NOEXCEPT
        :   Type(ERayTracingGroupType::TriangleHitShader)
        ,   InstanceId(instance)
        ,   GeometryId(geometry)
        ,   RecordOffset(offset)
        ,   MainShader(closestHit)
        ,   AnyHitShader(anyHit)
        {}

        FShaderGroup(const FInstanceID& instance, const FGeometryID& geometry, u32 offset, const FRTShaderID& closestHit, const FRTShaderID& anyHit, const FRTShaderID& intersection) NOEXCEPT
        :   Type(ERayTracingGroupType::ProceduralHitShader)
        ,   InstanceId(instance)
        ,   GeometryId(geometry)
        ,   RecordOffset(offset)
        ,   MainShader(closestHit)
        ,   AnyHitShader(anyHit)
        ,   IntersectionShader(intersection)
        {}

        friend bool operator ==(const FShaderGroup& lhs, const FShaderGroup& rhs) NOEXCEPT {
            return (lhs.Type == rhs.Type && lhs.MainShader == rhs.MainShader &&
                lhs.AnyHitShader == rhs.AnyHitShader && lhs.IntersectionShader == rhs.IntersectionShader );
        }
        friend bool operator !=(const FShaderGroup& lhs, const FShaderGroup& rhs) NOEXCEPT {
            return (not operator ==(lhs, rhs));
        }

        friend hash_t hash_value(const FShaderGroup& grp) NOEXCEPT  {
            return hash_tuple(grp.Type, grp.MainShader, grp.AnyHitShader, grp.IntersectionShader);
        }
    };

    using FShaderGroups = TArray<FShaderGroup>;

    FRawRTShaderTableID ShaderTable;
    FRawRTPipelineID Pipeline;
    FRawRTSceneID Scene;
    FRayGenShader RayGenShader;
    FShaderGroups ShaderGroups;
    u32 MaxRecursionDepth{ 0 };

#if USE_PPE_RHITASKNAME
    FUpdateRayTracingShaderTable() NOEXCEPT : TFrameTaskDesc<FUpdateRayTracingShaderTable>("UpdateRayTracingShaderTable", FDebugColorScheme::Get().HostToDeviceTransfer) {}
#endif

    FUpdateRayTracingShaderTable& SetTarget(const FRawRTShaderTableID& value) { Assert(value); ShaderTable = value; return (*this); }
    FUpdateRayTracingShaderTable& SetPipeline(const FRawRTPipelineID& value) { Assert(value); Pipeline = value; return (*this); }
    FUpdateRayTracingShaderTable& SetScene(const FRawRTSceneID& value) { Assert(value); Scene = value; return (*this); }
    FUpdateRayTracingShaderTable& SetMaxRecursionDepth(u32 value) { MaxRecursionDepth = value; return (*this); }

    FUpdateRayTracingShaderTable& SetRayGenShader(const FRTShaderID& shader);

    FUpdateRayTracingShaderTable& AddMissShader(const FRTShaderID& shader, u32 missIndex);
    FUpdateRayTracingShaderTable& AddHitShader(const FInstanceID& instance, u32 offset, const FRTShaderID& closestHit, const FRTShaderID& anyHit = Default);
    FUpdateRayTracingShaderTable& AddHitShader(const FInstanceID& instance, const FGeometryID& geometry, u32 offset, const FRTShaderID& closestHit, const FRTShaderID& anyHit = Default);
    FUpdateRayTracingShaderTable& AddProceduralHitShader(const FInstanceID& instance, const FGeometryID& geometry, u32 offset, const FRTShaderID& closestHit, const FRTShaderID& anyHit, const FRTShaderID& intersection);

};
//----------------------------------------------------------------------------
// FTraceRays
//----------------------------------------------------------------------------
struct FTraceRays final : details::TFrameTaskDesc<FTraceRays> {

    FPipelineResourceSet Resources;
    uint3 GroupCount{ 0 };
    FPushConstantDatas PushConstants;
    FRawRTShaderTableID ShaderTable;

#if USE_PPE_RHIDEBUG
    using FDebugMode = FRayTracingShaderDebugMode;
    FDebugMode DebugMode;
#endif

#if USE_PPE_RHITASKNAME
    FTraceRays() NOEXCEPT : TFrameTaskDesc<FTraceRays>("TraceRays", FDebugColorScheme::Get().RayTracing) {}
#endif

    FTraceRays& AddResources(const FDescriptorSetID& id, const PCPipelineResources& res);

    FTraceRays& SetGroupCount(const uint3& value) { GroupCount = value; return (*this); }
    FTraceRays& SetGroupCount(u32 x, u32 y = 1, u32 z = 1) { GroupCount = uint3(x, y, z); return (*this); }

    FTraceRays& SetShaderTable(FRawRTShaderTableID value);

    template <typename T>
    FTraceRays& AddPushConstant(const FPushConstantID& id, const T& value) { return AddPushConstant(id, &value, sizeof(value)); }
    FTraceRays& AddPushConstant(const FPushConstantID& id, const void* p, size_t size) { PushConstants.Push(id, p, size); return (*this); }

#if USE_PPE_RHIDEBUG
    FTraceRays& EnableShaderDebugTrace() { return EnableShaderDebugTrace(uint3(~0u)); }
    FTraceRays& EnableShaderDebugTrace(const uint3& launchId);
#endif

#if USE_PPE_RHIDEBUG
    FTraceRays& EnableShaderProfiling() { return EnableShaderProfiling(uint3(~0u)); }
    FTraceRays& EnableShaderProfiling(const uint3& launchId);
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#include "RayTracingTask-inl.h"
