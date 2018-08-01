#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

namespace Core {
namespace Graphics {
enum class ESurfaceFormatType : u32;
class FPresentationParameters;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DXGI_FORMAT SurfaceFormatTypeToDXGIFormat(ESurfaceFormatType value);
//----------------------------------------------------------------------------
ESurfaceFormatType DXGIFormatToSurfaceFormatType(DXGI_FORMAT value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
