#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core.Lattice/Lattice_fwd.h"
#include "Core.Lattice/GenericMesh.h"

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern template class GenericVertexSubPart<float2>;
extern template class GenericVertexSubPart<float3>;
extern template class GenericVertexSubPart<float4>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
