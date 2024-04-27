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
NODISCARD PPE_MESHBUILDER_API FAabb3f ComputeBounds(const FGenericMesh& mesh, size_t index);
NODISCARD PPE_MESHBUILDER_API FAabb3f ComputeSubPartBounds(const TGenericVertexSubPart<float3>& subPart);
NODISCARD PPE_MESHBUILDER_API FAabb4f ComputeSubPartBounds(const TGenericVertexSubPart<float4>& subPart);
//----------------------------------------------------------------------------
NODISCARD PPE_MESHBUILDER_API bool ComputeNormals(FGenericMesh& mesh, size_t index);
PPE_MESHBUILDER_API void ComputeNormals(const FGenericMesh& mesh, const FPositions3f& positions, const FNormals3f& normals);
PPE_MESHBUILDER_API void ComputeNormals(const FGenericMesh& mesh, const FPositions4f& positions, const FNormals3f& normals);
//----------------------------------------------------------------------------
NODISCARD PPE_MESHBUILDER_API bool ComputeTangentSpace(FGenericMesh& mesh, size_t index, bool packHandedness = false);
PPE_MESHBUILDER_API void ComputeTangentSpace(const FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords2f& uv, const FNormals3f& normals, const FTangents3f& tangents, const FBinormals3f& binormals);
PPE_MESHBUILDER_API void ComputeTangentSpace(const FGenericMesh& mesh, const FPositions4f& positions, const FTexcoords2f& uv, const FNormals3f& normals, const FTangents3f& tangents, const FBinormals3f& binormals);
PPE_MESHBUILDER_API void ComputeTangentSpace(const FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords2f& uv, const FNormals3f& normals, const FTangents4f& tangents);
PPE_MESHBUILDER_API void ComputeTangentSpace(const FGenericMesh& mesh, const FPositions4f& positions, const FTexcoords2f& uv, const FNormals3f& normals, const FTangents4f& tangents);
//----------------------------------------------------------------------------
NODISCARD PPE_MESHBUILDER_API bool TangentSpaceToQuaternion(FGenericMesh& mesh, size_t index, bool removeTBN = true);
PPE_MESHBUILDER_API void TangentSpaceToQuaternion(const FGenericMesh& mesh, const FNormals3f& normals, const FBinormals3f& binormals, const FTangents3f& tangents, const FNormals4f& quaternions);
PPE_MESHBUILDER_API void TangentSpaceToQuaternion(const FGenericMesh& mesh, const FNormals3f& normals, const FTangents4f& tangentsWithHandedness, const FNormals4f& quaternions);
//----------------------------------------------------------------------------
PPE_MESHBUILDER_API size_t MergeCloseVertices(FGenericMesh& mesh, size_t index, float minDistance = Epsilon);
PPE_MESHBUILDER_API size_t MergeDuplicateVertices(FGenericMesh& mesh);
PPE_MESHBUILDER_API size_t RemoveZeroAreaTriangles(FGenericMesh& mesh, size_t index, float minArea = Epsilon);
PPE_MESHBUILDER_API size_t RemoveUnusedVertices(FGenericMesh& mesh);
//----------------------------------------------------------------------------
PPE_MESHBUILDER_API void Transform(FGenericMesh& mesh, size_t index, const float4x4& transform);
PPE_MESHBUILDER_API void Transform(FGenericMesh& mesh, size_t index, const FTransform& transform);
//----------------------------------------------------------------------------
NODISCARD PPE_MESHBUILDER_API bool PNTriangles(FGenericMesh& mesh, size_t index, size_t recursions);
PPE_MESHBUILDER_API void PNTriangles(FGenericMesh& mesh, const FPositions3f& positions, const FNormals3f& normals, size_t recursions);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// http://fgiesen.wordpress.com/2013/12/14/simple-lossless-index-buffer-compression/
//----------------------------------------------------------------------------
PPE_MESHBUILDER_API void OptimizeIndicesOrder(const TMemoryView<u32>& indices, size_t vertexCount);
PPE_MESHBUILDER_API void OptimizeIndicesOrder(FGenericMesh& mesh);
//----------------------------------------------------------------------------
PPE_MESHBUILDER_API void OptimizeVerticesOrder(FGenericMesh& mesh);
//----------------------------------------------------------------------------
PPE_MESHBUILDER_API void OptimizeIndicesAndVerticesOrder(FGenericMesh& mesh);
//----------------------------------------------------------------------------
NODISCARD PPE_MESHBUILDER_API float VertexAverageCacheMissRate(const TMemoryView<u32>& indices, bool fifo = true, size_t cacheSize = 16);
NODISCARD PPE_MESHBUILDER_API float VertexAverageCacheMissRate(const FGenericMesh& mesh, bool fifo = true, size_t cacheSize = 16);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
