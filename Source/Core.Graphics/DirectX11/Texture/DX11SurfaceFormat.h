#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

namespace Core {
namespace Graphics {
enum class SurfaceFormatType : u32;
class PresentationParameters;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DXGI_FORMAT SurfaceFormatTypeToDXGIFormat(SurfaceFormatType value);
//----------------------------------------------------------------------------
SurfaceFormatType DXGIFormatToSurfaceFormatType(DXGI_FORMAT value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
