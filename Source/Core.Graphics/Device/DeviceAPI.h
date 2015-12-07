#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPI_fwd.h"
#include "Core.Graphics/Device/IDeviceAPIContext.h"
#include "Core.Graphics/Device/IDeviceAPIEncapsulator.h"
#include "Core.Graphics/Device/IDeviceAPIDiagnostics.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class DeviceAPI {
    DirectX11   = 0,
    OpenGL4     = 1,

    Unknown,
};
//----------------------------------------------------------------------------
const char *DeviceAPIToCStr(DeviceAPI api);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
