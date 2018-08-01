#pragma once

#include "Lattice.h"

#include "Lattice_fwd.h"
#include "GenericMesh.h"

namespace PPE {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern PPE_LATTICE_API template class TGenericVertexSubPart<float2>;
extern PPE_LATTICE_API template class TGenericVertexSubPart<float3>;
extern PPE_LATTICE_API template class TGenericVertexSubPart<float4>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace PPE
