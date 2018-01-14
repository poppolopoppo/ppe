#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core.Lattice/Lattice_fwd.h"
#include "Core.Lattice/GenericMesh.h"

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern CORE_LATTICE_API template class TGenericVertexSubPart<float2>;
extern CORE_LATTICE_API template class TGenericVertexSubPart<float3>;
extern CORE_LATTICE_API template class TGenericVertexSubPart<float4>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
