#pragma once

#include "MeshBuilder_fwd.h"

#include "Maths/MathHelpers.h"
#include "Maths/ScalarBoundingBox_fwd.h"
#include "Maths/ScalarMatrix_fwd.h"
#include "Memory/MemoryView.h"

namespace PPE {
class FTransform;
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAabb3f ComputeBounds(const FGenericMesh& mesh, size_t index);
FAabb3f ComputeSubPartBounds(const TGenericVertexSubPart<float3>& subPart);
FAabb4f ComputeSubPartBounds(const TGenericVertexSubPart<float4>& subPart);
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
bool TangentSpaceToQuaternion(FGenericMesh& mesh, size_t index, bool removeTBN = true);
void TangentSpaceToQuaternion(const FGenericMesh& mesh, const FNormals3f& normals, const FBinormals3f& binormals, const FTangents3f& tangents, const FNormals4f& quaternions);
void TangentSpaceToQuaternion(const FGenericMesh& mesh, const FNormals3f& normals, const FTangents4f& tangentsWithHandedness, const FNormals4f& quaternions);
//----------------------------------------------------------------------------
size_t MergeCloseVertices(FGenericMesh& mesh, size_t index, float minDistance = Epsilon);
size_t MergeDuplicateVertices(FGenericMesh& mesh);
size_t RemoveZeroAreaTriangles(FGenericMesh& mesh, size_t index, float minArea = Epsilon);
size_t RemoveUnusedVertices(FGenericMesh& mesh);
//----------------------------------------------------------------------------
void Transform(FGenericMesh& mesh, size_t index, const float4x4& transform);
void Transform(FGenericMesh& mesh, size_t index, const FTransform& transform);
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
} //!namespace ContentPipeline
} //!namespace PPE
