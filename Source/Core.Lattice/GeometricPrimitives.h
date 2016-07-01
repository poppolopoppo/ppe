#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core/Color/Color.h"
#include "Core/Container/Vector.h"
#include "Core/Maths/Geometry/ScalarVector_fwd.h"

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace GeometricPrimitive {
//----------------------------------------------------------------------------
typedef VECTOR_THREAD_LOCAL(Geometry, u32)          Indices;
typedef VECTOR_THREAD_LOCAL(Geometry, float3)       Positions;
typedef VECTOR_THREAD_LOCAL(Geometry, ColorRGBA)    Colors;
typedef VECTOR_THREAD_LOCAL(Geometry, float2)       TexCoords;
typedef VECTOR_THREAD_LOCAL(Geometry, float3)       Normals;
typedef VECTOR_THREAD_LOCAL(Geometry, float3)       Tangents;
typedef VECTOR_THREAD_LOCAL(Geometry, float3)       Binormals;
//----------------------------------------------------------------------------
void Cube(Indices& indices, Positions& uvw);
void Pyramid(Indices& indices, Positions& uvw);
void Octahedron(Indices& indices, Positions& uvw);
void Icosahedron(Indices& indices, Positions& uvw);
//----------------------------------------------------------------------------
void Geosphere(size_t divisions, Indices& indices, Positions& uvw);
void HemiGeosphere(size_t divisions, Indices& indices, Positions& uvw);
//----------------------------------------------------------------------------
void DivideTriangles(Indices& dstIdx, const Indices& srcIdx, Positions& uvw);
//----------------------------------------------------------------------------
void SmoothNormals(Normals& normals, const Indices& indices, const Positions& uvw);
//----------------------------------------------------------------------------
} //!namespace GeometricPrimitive
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
