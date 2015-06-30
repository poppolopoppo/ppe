#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/Geometry/PrimitiveType.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_PRIMITIVE_TOPOLOGY PrimitiveTypeToDX11PrimitiveTopology(PrimitiveType value);
//----------------------------------------------------------------------------
PrimitiveType DX11PrimitiveTopologyToPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
