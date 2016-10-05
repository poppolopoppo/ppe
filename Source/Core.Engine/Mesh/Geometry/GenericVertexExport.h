#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Mesh/Geometry/GeometricPrimitives.h"

#include "Core/Container/RawStorage.h"

namespace Core {
namespace Engine {
class FGenericVertex;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ExportVertices(
    FGenericVertex& vertices,

    const GeometricPrimitive::Positions *positions0,
    const GeometricPrimitive::Positions *positions1,
    const GeometricPrimitive::Positions *positions2,

    const GeometricPrimitive::Colors *colors0,
    const GeometricPrimitive::Colors *colors1,
    const GeometricPrimitive::Colors *colors2,

    const GeometricPrimitive::TexCoords *texCoords0,
    const GeometricPrimitive::TexCoords *texCoords1,
    const GeometricPrimitive::TexCoords *texCoords2,

    const GeometricPrimitive::Normals *normals0,
    const GeometricPrimitive::Normals *normals1,
    const GeometricPrimitive::Normals *normals2,

    const GeometricPrimitive::Tangents *tangents0,
    const GeometricPrimitive::Tangents *tangents1,
    const GeometricPrimitive::Tangents *tangents2,

    const GeometricPrimitive::Binormals *binormals0,
    const GeometricPrimitive::Binormals *binormals1,
    const GeometricPrimitive::Binormals *binormals2 );
//----------------------------------------------------------------------------
void ExportVertices(
    FGenericVertex& vertices,

    const GeometricPrimitive::Positions *positions0,
    const GeometricPrimitive::Colors *colors0,
    const GeometricPrimitive::TexCoords *texCoords0,
    const GeometricPrimitive::Normals *normals0,
    const GeometricPrimitive::Tangents *tangents0,
    const GeometricPrimitive::Binormals *binormals0 );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void ExportVertices(
    TRawStorage<T, _Allocator>& vertices,
    const Graphics::FVertexDeclaration *vertexDeclaration,
    const size_t vertexCount,

    const GeometricPrimitive::Positions *positions0,
    const GeometricPrimitive::Positions *positions1,
    const GeometricPrimitive::Positions *positions2,

    const GeometricPrimitive::Colors *colors0,
    const GeometricPrimitive::Colors *colors1,
    const GeometricPrimitive::Colors *colors2,

    const GeometricPrimitive::TexCoords *texCoords0,
    const GeometricPrimitive::TexCoords *texCoords1,
    const GeometricPrimitive::TexCoords *texCoords2,

    const GeometricPrimitive::Normals *normals0,
    const GeometricPrimitive::Normals *normals1,
    const GeometricPrimitive::Normals *normals2,

    const GeometricPrimitive::Tangents *tangents0,
    const GeometricPrimitive::Tangents *tangents1,
    const GeometricPrimitive::Tangents *tangents2,

    const GeometricPrimitive::Binormals *binormals0,
    const GeometricPrimitive::Binormals *binormals1,
    const GeometricPrimitive::Binormals *binormals2
    ) {
    Assert(vertexDeclaration);
    Assert(vertexCount);

    vertices.Resize_DiscardData(vertexDeclaration->SizeInBytes() * vertexCount);

    FGenericVertex vertex(vertexDeclaration);
    vertex.SetDestination(vertices.Pointer(), vertices.SizeInBytes());
    Assert(vertexCount == vertex.VertexCountRemaining());

    ExportVertices(vertex,
        positions0, positions1, positions2,
        colors0, colors1, colors2,
        texCoords0, texCoords1, texCoords2,
        normals0, normals1, normals2,
        tangents0, tangents1, tangents2,
        binormals0, binormals1, binormals2 );
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void ExportVertices(
    TRawStorage<T, _Allocator>& vertices,
    const Graphics::FVertexDeclaration *vertexDeclaration,
    const size_t vertexCount,

    const GeometricPrimitive::Positions *positions0,
    const GeometricPrimitive::Colors *colors0,
    const GeometricPrimitive::TexCoords *texCoords0,
    const GeometricPrimitive::Normals *normals0,
    const GeometricPrimitive::Tangents *tangents0,
    const GeometricPrimitive::Binormals *binormals0
    ) {
    Assert(vertexDeclaration);
    Assert(vertexCount);

    vertices.Resize_DiscardData(vertexDeclaration->SizeInBytes() * vertexCount);

    FGenericVertex vertex(vertexDeclaration);
    vertex.SetDestination(vertices.Pointer(), vertices.SizeInBytes());
    Assert(vertexCount == vertex.VertexCountRemaining());

    ExportVertices(vertex, positions0, colors0, texCoords0, normals0, tangents0, binormals0);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ComputeTangentSpace(FGenericVertex& vertices, const TMemoryView<const u32>& indices);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
