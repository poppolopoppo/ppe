#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/RawStorage.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
namespace Graphics {
class VertexDeclaration;
}

namespace Engine {
class GenericVertex;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// http://fgiesen.wordpress.com/2013/12/14/simple-lossless-index-buffer-compression/
//----------------------------------------------------------------------------
void OptimizeIndicesOrder(const MemoryView<u32>& indices, size_t vertexCount);
//----------------------------------------------------------------------------
void OptimizeVerticesOrder(GenericVertex& vertices, const MemoryView<u32>& indices);
//----------------------------------------------------------------------------
void OptimizeIndicesAndVerticesOrder(GenericVertex& vertices, const MemoryView<u32>& indices);
//----------------------------------------------------------------------------
float VertexAverageCacheMissRate(const MemoryView<u32>& indices, bool fifo = true, size_t cacheSize = 16);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void OptimizeIndicesAndVerticesOrder(
    RawStorage<T, _Allocator>& vertices,
    const Graphics::VertexDeclaration *vertexDeclaration,
    const size_t vertexCount,
    const MemoryView<u32>& indices ) {
    Assert(vertexDeclaration);
    Assert(vertexCount);
    Assert(vertexDeclaration->SizeInBytes() * vertexCount == vertices.SizeInBytes());

    GenericVertex vertex(vertexDeclaration);
    vertex.SetDestination(vertices.Pointer(), vertices.SizeInBytes());
    Assert(vertexCount == vertex.VertexCountRemaining());

    OptimizeIndicesAndVerticesOrder(vertex, indices);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
