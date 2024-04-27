#pragma once

#include "MeshBuilder_fwd.h"

#include "Maths/ScalarMatrix_fwd.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_MESHBUILDER_API void Cube(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords);
PPE_MESHBUILDER_API void Cube(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
PPE_MESHBUILDER_API void Pyramid(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords);
PPE_MESHBUILDER_API void Pyramid(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
PPE_MESHBUILDER_API void Octahedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords);
PPE_MESHBUILDER_API void Octahedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
PPE_MESHBUILDER_API void Icosahedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords);
PPE_MESHBUILDER_API void Icosahedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
PPE_MESHBUILDER_API void ContellatedTetraHedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords);
PPE_MESHBUILDER_API void ContellatedTetraHedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
PPE_MESHBUILDER_API void HemiContellatedTetraHedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords);
PPE_MESHBUILDER_API void HemiContellatedTetraHedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
PPE_MESHBUILDER_API void Geosphere(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, size_t divisions);
PPE_MESHBUILDER_API void Geosphere(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, size_t divisions, const float4x4& transform);
//----------------------------------------------------------------------------
PPE_MESHBUILDER_API void HemiGeosphere(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, size_t divisions);
PPE_MESHBUILDER_API void HemiGeosphere(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, size_t divisions, const float4x4& transform);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
