// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
FDefaultDeviceEncapsulatorService::FDefaultDeviceEncapsulatorService(
    Graphics::EDeviceAPI api,
    const Graphics::FGraphicsWindow *window,
    const Graphics::FPresentationParameters& presentationParameters
    )
:   IDeviceEncapsulatorService(ENGINESERVICE_CONSTRUCT(IDeviceEncapsulatorService))
,   _deviceAPI(api)
,   _window(window)
,   _presentationParameters(presentationParameters) {
    Assert(window);
}
//----------------------------------------------------------------------------
FDefaultDeviceEncapsulatorService::~FDefaultDeviceEncapsulatorService() {
    Assert(!_deviceEncapsulator.Available());
}
//----------------------------------------------------------------------------
Graphics::FDeviceEncapsulator *FDefaultDeviceEncapsulatorService::FDeviceEncapsulator() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_deviceEncapsulator;
}
//----------------------------------------------------------------------------
const Graphics::FDeviceEncapsulator *FDefaultDeviceEncapsulatorService::FDeviceEncapsulator() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_deviceEncapsulator;
}
//----------------------------------------------------------------------------
void FDefaultDeviceEncapsulatorService::Start(IServiceProvider *provider, const FGuid& guid) {
    IDeviceEncapsulatorService::Start(provider, guid);

    _deviceEncapsulator.Create(_deviceAPI, _window->Handle(), _presentationParameters);
}
//----------------------------------------------------------------------------
void FDefaultDeviceEncapsulatorService::Shutdown(IServiceProvider *provider, const FGuid& guid) {
    IDeviceEncapsulatorService::Shutdown(provider, guid);

    _deviceEncapsulator.Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
