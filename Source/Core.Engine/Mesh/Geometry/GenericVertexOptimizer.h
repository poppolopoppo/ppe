#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/RawStorage.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
namespace Graphics {
class FVertexDeclaration;
}

namespace Engine {
class FGenericVertex;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// http://fgiesen.wordpress.com/2013/12/14/simple-lossless-index-buffer-compression/
//----------------------------------------------------------------------------
void MergeDuplicateVertices(FGenericVertex& vertices, const TMemoryView<u32>& indices);
//----------------------------------------------------------------------------
void OptimizeIndicesOrder(const TMemoryView<u32>& indices, size_t vertexCount);
//----------------------------------------------------------------------------
void OptimizeVerticesOrder(FGenericVertex& vertices, const TMemoryView<u32>& indices);
//----------------------------------------------------------------------------
void OptimizeIndicesAndVerticesOrder(FGenericVertex& vertices, const TMemoryView<u32>& indices);
//----------------------------------------------------------------------------
float VertexAverageCacheMissRate(const TMemoryView<u32>& indices, bool fifo = true, size_t cacheSize = 16);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void OptimizeIndicesAndVerticesOrder(
    TRawStorage<T, _Allocator>& vertices,
    const Graphics::FVertexDeclaration *vertexDeclaration,
    const size_t vertexCount,
    const TMemoryView<u32>& indices ) {
    Assert(vertexDeclaration);
    Assert(vertexCount);
    Assert(vertexDeclaration->SizeInBytes() * vertexCount == vertices.SizeInBytes());

    FGenericVertex vertex(vertexDeclaration);
    vertex.SetDestination(vertices.Pointer(), vertices.SizeInBytes());
    vertex.SeekVertex(vertexCount);
    Assert(vertexCount == vertex.VertexCountWritten());

    OptimizeIndicesAndVerticesOrder(vertex, indices);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
