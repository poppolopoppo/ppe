#include "stdafx.h"

#include "DeviceEncapsulatorService.h"

#include "Core.Graphics/Window/GraphicsWindow.h"

#include "Core/Meta/Guid.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ENGINESERVICE_GUID_DEF(IDeviceEncapsulatorService);
//----------------------------------------------------------------------------
DefaultDeviceEncapsulatorService::DefaultDeviceEncapsulatorService(
    Graphics::DeviceAPI api,
    const Graphics::GraphicsWindow *window,
    const Graphics::PresentationParameters& presentationParameters
    )
:   IDeviceEncapsulatorService(ENGINESERVICE_CONSTRUCT(IDeviceEncapsulatorService))
,   _deviceAPI(api)
,   _window(window)
,   _presentationParameters(presentationParameters) {
    Assert(window);
}
//----------------------------------------------------------------------------
DefaultDeviceEncapsulatorService::~DefaultDeviceEncapsulatorService() {
    Assert(!_deviceEncapsulator.Available());
}
//----------------------------------------------------------------------------
Graphics::DeviceEncapsulator *DefaultDeviceEncapsulatorService::DeviceEncapsulator() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_deviceEncapsulator;
}
//----------------------------------------------------------------------------
const Graphics::DeviceEncapsulator *DefaultDeviceEncapsulatorService::DeviceEncapsulator() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_deviceEncapsulator;
}
//----------------------------------------------------------------------------
void DefaultDeviceEncapsulatorService::Start(IServiceProvider *provider, const Guid& guid) {
    IDeviceEncapsulatorService::Start(provider, guid);

    _deviceEncapsulator.Create(_deviceAPI, _window->Handle(), _presentationParameters);
}
//----------------------------------------------------------------------------
void DefaultDeviceEncapsulatorService::Shutdown(IServiceProvider *provider, const Guid& guid) {
    IDeviceEncapsulatorService::Shutdown(provider, guid);

    _deviceEncapsulator.Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
