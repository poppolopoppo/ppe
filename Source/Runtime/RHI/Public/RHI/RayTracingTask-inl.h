#pragma once

#include "RayTracingTask.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FBuildRayTracingGeometry::FTriangles& FBuildRayTracingGeometry::FTriangles::SetVertexBuffer(FRawBufferID buffer, u32 offset) {
    Assert(buffer);
    Assert(VertexData.empty());
    VertexBuffer = buffer;
    VertexOffset = offset;
    return (*this);
}
//----------------------------------------------------------------------------
inline FBuildRayTracingGeometry::FTriangles& FBuildRayTracingGeometry::FTriangles::SetVertexData(FRawMemoryConst data) {
    Assert(not data.empty());
    Assert(not VertexBuffer);
    VertexData = data;
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T>
FBuildRayTracingGeometry::FTriangles& FBuildRayTracingGeometry::FTriangles::SetVertexData(TMemoryView<const T> vertices) {
    Assert(not vertices.empty());
    VertexBuffer = Default;
    VertexData = vertices.template Cast<const u8>();
    VertexCount = checked_cast<u32>(vertices.size());
    VertexFormat = VertexAttrib<T>();
    VertexStride = sizeof(T);
    return (*this);
}
//----------------------------------------------------------------------------
inline FBuildRayTracingGeometry::FTriangles& FBuildRayTracingGeometry::FTriangles::SetVertices(u32 count, EVertexFormat fmt, u32 stride) {
    Assert(count > 0);
    VertexCount = count;
    VertexStride = stride;
    VertexFormat = fmt;
    return (*this);
}
//----------------------------------------------------------------------------
inline FBuildRayTracingGeometry::FTriangles& FBuildRayTracingGeometry::FTriangles::SetIndexBuffer(FRawBufferID buffer, u32 offset) {
    Assert(buffer);
    Assert(IndexData.empty());
    IndexBuffer = buffer;
    IndexOffset = offset;
    return (*this);
}
//----------------------------------------------------------------------------
inline FBuildRayTracingGeometry::FTriangles& FBuildRayTracingGeometry::FTriangles::SetIndexData(FRawMemoryConst data) {
    Assert(not IndexBuffer);
    IndexData = data;
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T>
FBuildRayTracingGeometry::FTriangles& FBuildRayTracingGeometry::FTriangles::SetIndexData(TMemoryView<const T> indices) {
    Assert(not indices.empty());
    IndexBuffer = Default;
    IndexData = indices.template Cast<const u8>();
    IndexCount = checked_cast<u32>(indices.size());
    IndexFormat = IndexAttrib<T>();
    return (*this);
}
//----------------------------------------------------------------------------
inline FBuildRayTracingGeometry::FTriangles& FBuildRayTracingGeometry::FTriangles::SetIndices(u32 count, EIndexFormat fmt) {
    Assert(count > 0);
    IndexCount = count;
    IndexFormat = fmt;
    return (*this);
}
//----------------------------------------------------------------------------
inline FBuildRayTracingGeometry::FTriangles& FBuildRayTracingGeometry::FTriangles::SetTransformBuffer(FRawBufferID buffer, u32 offset) {
    Assert(buffer);
    TransformBuffer = buffer;
    TransformOffset = offset;
    return (*this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FBuildRayTracingGeometry::FBoundingVolumes& FBuildRayTracingGeometry::FBoundingVolumes::SetBuffer(FRawBufferID buffer, u32 offset) {
    Assert(buffer);
    Assert(AabbData.empty());
    AabbBuffer = buffer;
    AabbOffset = offset;
    return (*this);
}
//----------------------------------------------------------------------------
inline FBuildRayTracingGeometry::FBoundingVolumes& FBuildRayTracingGeometry::FBoundingVolumes::SetData(FRawMemoryConst data) {
    Assert(not AabbBuffer);
    AabbData = data;
    return (*this);
}
//----------------------------------------------------------------------------
inline FBuildRayTracingGeometry::FBoundingVolumes& FBuildRayTracingGeometry::FBoundingVolumes::SetCount(u32 count, u32 stride) {
    Assert(count > 0);
    AabbCount = count;
    AabbStride = stride;
    return (*this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FBuildRayTracingScene::FInstance& FBuildRayTracingScene::FInstance::SetInstanceId(const FInstanceID& value) {
    Assert(value);
    InstanceId = value;
    return (*this);
}
//----------------------------------------------------------------------------
inline FBuildRayTracingScene::FInstance& FBuildRayTracingScene::FInstance::SetGeometry(const FRawRTGeometryID& value) {
    Assert(value);
    GeometryId = value;
    return (*this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FUpdateRayTracingShaderTable& FUpdateRayTracingShaderTable::SetRayGenShader(const FRTShaderID& shader) {
    Assert(shader);
    RayGenShader = { shader };
    return (*this);
}
//----------------------------------------------------------------------------
inline FUpdateRayTracingShaderTable& FUpdateRayTracingShaderTable::AddMissShader(const FRTShaderID& shader, u32 missIndex) {
    Assert(shader);
    ShaderGroups.emplace_back(shader, missIndex);
    return (*this);
}
//----------------------------------------------------------------------------
inline FUpdateRayTracingShaderTable& FUpdateRayTracingShaderTable::AddHitShader(const FInstanceID& instance, u32 offset, const FRTShaderID& closestHit, const FRTShaderID& anyHit) {
    Assert(instance);
    Assert(closestHit);
    ShaderGroups.emplace_back(instance, Default, offset, closestHit, anyHit);
    return (*this);
}
//----------------------------------------------------------------------------
inline FUpdateRayTracingShaderTable& FUpdateRayTracingShaderTable::AddHitShader(const FInstanceID& instance, const FGeometryID& geometry, u32 offset, const FRTShaderID& closestHit, const FRTShaderID& anyHit) {
    Assert(instance);
    Assert(geometry);
    Assert(closestHit);
    ShaderGroups.emplace_back(instance, geometry, offset, closestHit, anyHit);
    return (*this);
}
//----------------------------------------------------------------------------
inline FUpdateRayTracingShaderTable& FUpdateRayTracingShaderTable::AddProceduralHitShader(const FInstanceID& instance, const FGeometryID& geometry, u32 offset, const FRTShaderID& closestHit, const FRTShaderID& anyHit, const FRTShaderID& intersection) {
    Assert(instance);
    Assert(closestHit);
    ShaderGroups.emplace_back(instance, geometry, offset, closestHit, anyHit, intersection);
    return (*this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FTraceRays& FTraceRays::AddResources(const FDescriptorSetID& id, const FPipelineResources* res) {
    Assert(id);
    Assert(res);
    Resources.insert(id, PCPipelineResources(res));
    return (*this);
}
//----------------------------------------------------------------------------
inline FTraceRays& FTraceRays::SetShaderTable(FRawRTShaderTableID value) {
    Assert(value);
    ShaderTable = value;
    return (*this);
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
inline FTraceRays& FTraceRays::EnableShaderDebugTrace(const uint3& launchId) {
    DebugMode.Mode = EShaderDebugMode::Trace;
    DebugMode.LaunchId = launchId;
    return (*this);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
inline FTraceRays& FTraceRays::EnableShaderProfiling(const uint3& launchId) {
    DebugMode.Mode = EShaderDebugMode::Profiling;
    DebugMode.LaunchId = launchId;
    return (*this);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
