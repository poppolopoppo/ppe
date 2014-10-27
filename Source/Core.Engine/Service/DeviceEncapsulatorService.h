#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Graphics/Device/DeviceEncapsulator.h"

#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/Service_fwd.h"

namespace Core {
struct Guid;

namespace Graphics {
FWD_REFPTR(GraphicsWindow);
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IDeviceEncapsulatorService : public IService {
protected:
    IDeviceEncapsulatorService(const char *serviceName, int servicePriority)
    :   IService(serviceName, servicePriority) {}
public:
    virtual ~IDeviceEncapsulatorService() {}

    virtual Graphics::DeviceEncapsulator *DeviceEncapsulator() = 0;
    virtual const Graphics::DeviceEncapsulator *DeviceEncapsulator() const = 0;

    ENGINESERVICE_GUID_DECL(IDeviceEncapsulatorService);
};
//----------------------------------------------------------------------------
class DefaultDeviceEncapsulatorService : public IDeviceEncapsulatorService {
public:
    DefaultDeviceEncapsulatorService(   Graphics::DeviceAPI api,
                                        const Graphics::GraphicsWindow *window,
                                        const Graphics::PresentationParameters& presentationParameters);
    virtual ~DefaultDeviceEncapsulatorService();

    const Graphics::DeviceAPI DeviceAPI() const { return _deviceAPI; }
    const Graphics::GraphicsWindow *Window() const { return _window.get(); }
    const Graphics::PresentationParameters& PresentationParameters() const { return _presentationParameters; }

    virtual Graphics::DeviceEncapsulator *DeviceEncapsulator() override;
    virtual const Graphics::DeviceEncapsulator *DeviceEncapsulator() const override;

    virtual void Start(IServiceProvider *provider, const Guid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const Guid& guid) override;

private:
    Graphics::DeviceAPI _deviceAPI;
    Graphics::SCGraphicsWindow _window;
    Graphics::PresentationParameters _presentationParameters;

    Graphics::DeviceEncapsulator _deviceEncapsulator;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
