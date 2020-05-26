#pragma once

#include "DirectX11/DX11Includes.h"

#include "Device/Geometry/PrimitiveType.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_PRIMITIVE_TOPOLOGY PrimitiveTypeToDX11PrimitiveTopology(EPrimitiveType value);
//----------------------------------------------------------------------------
EPrimitiveType DX11PrimitiveTopologyToPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
