#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core.Lattice/GenericMesh_fwd.h"

#include "Core/Memory/MemoryView.h"

namespace Core {
namespace Lattice {
class GenericMesh;
template <typename T>
class GenericVertexSubPart;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool ComputeNormals(GenericMesh& mesh, size_t index);
void ComputeNormals(const GenericMesh& mesh, const Positions3f& positions, const Normals3f& normals);
void ComputeNormals(const GenericMesh& mesh, const Positions4f& positions, const Normals3f& normals);
//----------------------------------------------------------------------------
bool ComputeTangentSpace(GenericMesh& mesh, size_t index, bool packHandedness = false);
void ComputeTangentSpace(const GenericMesh& mesh, const Positions3f& positions, const TexCoords2f& uv, const Normals3f& normals, const Tangents3f& tangents, const Binormals3f& binormals);
void ComputeTangentSpace(const GenericMesh& mesh, const Positions4f& positions, const TexCoords2f& uv, const Normals3f& normals, const Tangents3f& tangents, const Binormals3f& binormals);
void ComputeTangentSpace(const GenericMesh& mesh, const Positions3f& positions, const TexCoords2f& uv, const Normals3f& normals, const Tangents4f& tangents);
void ComputeTangentSpace(const GenericMesh& mesh, const Positions4f& positions, const TexCoords2f& uv, const Normals3f& normals, const Tangents4f& tangents);
//----------------------------------------------------------------------------
bool MergeDuplicateVertices(GenericMesh& mesh);
//----------------------------------------------------------------------------
bool PNTriangles(GenericMesh& mesh, size_t index, size_t recursions);
void PNTriangles(GenericMesh& mesh, const Positions3f& positions, const Normals3f& normals, size_t recursions);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// http://fgiesen.wordpress.com/2013/12/14/simple-lossless-index-buffer-compression/
//----------------------------------------------------------------------------
void OptimizeIndicesOrder(const MemoryView<u32>& indices, size_t vertexCount);
void OptimizeIndicesOrder(GenericMesh& mesh);
//----------------------------------------------------------------------------
void OptimizeVerticesOrder(GenericMesh& mesh);
//----------------------------------------------------------------------------
void OptimizeIndicesAndVerticesOrder(GenericMesh& mesh);
//----------------------------------------------------------------------------
float VertexAverageCacheMissRate(const MemoryView<u32>& indices, bool fifo = true, size_t cacheSize = 16);
float VertexAverageCacheMissRate(const GenericMesh& mesh, bool fifo = true, size_t cacheSize = 16);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
