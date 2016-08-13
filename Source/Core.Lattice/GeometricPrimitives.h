#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core.Lattice/GenericMesh_fwd.h"

#include "Core/Maths/ScalarMatrix_fwd.h"

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Cube(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords);
void Cube(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
void Pyramid(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords);
void Pyramid(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
void Octahedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords);
void Octahedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
void Icosahedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords);
void Icosahedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
void ContellatedTetraHedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords);
void ContellatedTetraHedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
void HemiContellatedTetraHedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords);
void HemiContellatedTetraHedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, const float4x4& transform);
//----------------------------------------------------------------------------
void Geosphere(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, size_t divisions);
void Geosphere(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, size_t divisions, const float4x4& transform);
//----------------------------------------------------------------------------
void HemiGeosphere(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, size_t divisions);
void HemiGeosphere(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, size_t divisions, const float4x4& transform);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
