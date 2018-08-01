#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core.Lattice/GenericMesh_fwd.h"

#include "Core/Maths/ScalarMatrix_fwd.h"

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Cube(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords);
void Cube(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
void Pyramid(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords);
void Pyramid(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
void Octahedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords);
void Octahedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
void Icosahedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords);
void Icosahedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
void ContellatedTetraHedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords);
void ContellatedTetraHedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
void HemiContellatedTetraHedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords);
void HemiContellatedTetraHedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
void Geosphere(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords, size_t divisions);
void Geosphere(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords, size_t divisions, const float4x4& transform);
//----------------------------------------------------------------------------
void HemiGeosphere(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords, size_t divisions);
void HemiGeosphere(FGenericMesh& mesh, const FPositions3f& positions, const FTexCoords3f& texcoords, size_t divisions, const float4x4& transform);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
