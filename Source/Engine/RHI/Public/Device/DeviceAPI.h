#pragma once

#include "Graphics.h"

#include "Device/DeviceAPI_fwd.h"
#include "Device/IDeviceAPIContext.h"
#include "Device/IDeviceAPIEncapsulator.h"
#include "Device/IDeviceAPIDiagnostics.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EDeviceAPI {
    DirectX11   = 0,
    OpenGL4     = 1,

    Unknown,
};
//----------------------------------------------------------------------------
FStringView DeviceAPIToCStr(EDeviceAPI api);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
