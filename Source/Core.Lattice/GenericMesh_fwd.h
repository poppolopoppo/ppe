#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core/Maths/ScalarVector_fwd.h"

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class GenericMesh;
class GenericVertexData;
template <typename T>
class GenericVertexSubPart;
typedef GenericVertexSubPart<float3> Positions3f;
typedef GenericVertexSubPart<float4> Positions4f;
typedef GenericVertexSubPart<float2> TexCoords2f;
typedef GenericVertexSubPart<float3> TexCoords3f;
typedef GenericVertexSubPart<float4> TexCoords4f;
typedef GenericVertexSubPart<float4> Colors4f;
typedef GenericVertexSubPart<float3> Normals3f;
typedef GenericVertexSubPart<float3> Tangents3f;
typedef GenericVertexSubPart<float4> Tangents4f;
typedef GenericVertexSubPart<float3> Binormals3f;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
