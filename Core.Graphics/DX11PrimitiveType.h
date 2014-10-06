#pragma once

#include "DX11Includes.h"

#include "PrimitiveType.h"

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_PRIMITIVE_TOPOLOGY PrimitiveTypeToDX11PrimitiveTopology(Graphics::PrimitiveType value);
//----------------------------------------------------------------------------
Graphics::PrimitiveType DX11PrimitiveTopologyToPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
