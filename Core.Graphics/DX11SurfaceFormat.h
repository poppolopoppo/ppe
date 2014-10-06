#pragma once

#include "DX11Includes.h"

namespace Core {
namespace Graphics {
enum class SurfaceFormatType : u32;
class PresentationParameters;
} //!namespace Graphics
} //!namespace Core

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DXGI_FORMAT SurfaceFormatTypeToDXGIFormat(SurfaceFormatType value);
//----------------------------------------------------------------------------
SurfaceFormatType DXGIFormatToSurfaceFormatType(DXGI_FORMAT value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
