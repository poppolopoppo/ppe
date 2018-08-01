#pragma once

#include "Lattice.h"

#include "Maths/ScalarVector_fwd.h"

namespace PPE {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGenericMesh;
class FGenericVertexData;
template <typename T>
class TGenericVertexSubPart;
typedef TGenericVertexSubPart<float3> FPositions3f;
typedef TGenericVertexSubPart<float4> FPositions4f;
typedef TGenericVertexSubPart<float2> FTexCoords2f;
typedef TGenericVertexSubPart<float3> FTexCoords3f;
typedef TGenericVertexSubPart<float4> FTexCoords4f;
typedef TGenericVertexSubPart<float4> FColors4f;
typedef TGenericVertexSubPart<float3> FNormals3f;
typedef TGenericVertexSubPart<float4> FNormals4f;
typedef TGenericVertexSubPart<float3> FTangents3f;
typedef TGenericVertexSubPart<float4> FTangents4f;
typedef TGenericVertexSubPart<float3> FBinormals3f;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace PPE
