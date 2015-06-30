#include "stdafx.h"

#include "AbstractDeviceAPIEncapsulator.h"

#include "DeviceResource.h"

#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
AbstractDeviceAPIEncapsulator::AbstractDeviceAPIEncapsulator(DeviceAPI api, DeviceEncapsulator *owner, const PresentationParameters& pp)
:   _api(api)
,   _owner(owner)
,   _parameters(pp) {
    Assert(owner);
}
//----------------------------------------------------------------------------
AbstractDeviceAPIEncapsulator::~AbstractDeviceAPIEncapsulator() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *DeviceAPIToCStr(DeviceAPI api) {
    switch (api)
    {
    case Core::Graphics::DeviceAPI::DirectX11:
        return "DirectX11";
    case Core::Graphics::DeviceAPI::OpenGL4:
        return "OpenGL4";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
