#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core.Lattice/GenericMesh_fwd.h"

#include "Core/Memory/MemoryView.h"

namespace Core {
namespace Lattice {
class FGenericMesh;
template <typename T>
class TGenericVertexSubPart;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool ComputeNormals(FGenericMesh& mesh, size_t index);
void ComputeNormals(const FGenericMesh& mesh, const FPositions3f& positions, const FNormals3f& normals);
void ComputeNormals(const FGenericMesh& mesh, const FPositions4f& positions, const FNormals3f& normals);
//----------------------------------------------------------------------------
bool ComputeTangentSpace(FGenericMesh& mesh, size_t index, bool packHandedness = false);
void ComputeTangentSpace(const FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords2f& uv, const FNormals3f& normals, const FTangents3f& tangents, const FBinormals3f& binormals);
void ComputeTangentSpace(const FGenericMesh& mesh, const FPositions4f& positions, const FTexCoords2f& uv, const FNormals3f& normals, const FTangents3f& tangents, const FBinormals3f& binormals);
void ComputeTangentSpace(const FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords2f& uv, const FNormals3f& normals, const FTangents4f& tangents);
void ComputeTangentSpace(const FGenericMesh& mesh, const FPositions4f& positions, const FTexCoords2f& uv, const FNormals3f& normals, const FTangents4f& tangents);
//----------------------------------------------------------------------------
bool MergeDuplicateVertices(FGenericMesh& mesh);
//----------------------------------------------------------------------------
bool PNTriangles(FGenericMesh& mesh, size_t index, size_t recursions);
void PNTriangles(FGenericMesh& mesh, const FPositions3f& positions, const FNormals3f& normals, size_t recursions);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// http://fgiesen.wordpress.com/2013/12/14/simple-lossless-index-buffer-compression/
//----------------------------------------------------------------------------
void OptimizeIndicesOrder(const TMemoryView<u32>& indices, size_t vertexCount);
void OptimizeIndicesOrder(FGenericMesh& mesh);
//----------------------------------------------------------------------------
void OptimizeVerticesOrder(FGenericMesh& mesh);
//----------------------------------------------------------------------------
void OptimizeIndicesAndVerticesOrder(FGenericMesh& mesh);
//----------------------------------------------------------------------------
float VertexAverageCacheMissRate(const TMemoryView<u32>& indices, bool fifo = true, size_t cacheSize = 16);
float VertexAverageCacheMissRate(const FGenericMesh& mesh, bool fifo = true, size_t cacheSize = 16);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
