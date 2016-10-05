#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Graphics/Device/DeviceEncapsulator.h"

#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/Service_fwd.h"

namespace Core {
struct FGuid;

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

    virtual Graphics::FDeviceEncapsulator *FDeviceEncapsulator() = 0;
    virtual const Graphics::FDeviceEncapsulator *FDeviceEncapsulator() const = 0;

    ENGINESERVICE_GUID_DECL(IDeviceEncapsulatorService);
};
//----------------------------------------------------------------------------
class FDefaultDeviceEncapsulatorService : public IDeviceEncapsulatorService {
public:
    FDefaultDeviceEncapsulatorService(   Graphics::EDeviceAPI api,
                                        const Graphics::FGraphicsWindow *window,
                                        const Graphics::FPresentationParameters& presentationParameters);
    virtual ~FDefaultDeviceEncapsulatorService();

    const Graphics::EDeviceAPI EDeviceAPI() const { return _deviceAPI; }
    const Graphics::FGraphicsWindow *Window() const { return _window.get(); }
    const Graphics::FPresentationParameters& FPresentationParameters() const { return _presentationParameters; }

    virtual Graphics::FDeviceEncapsulator *FDeviceEncapsulator() override;
    virtual const Graphics::FDeviceEncapsulator *FDeviceEncapsulator() const override;

    virtual void Start(IServiceProvider *provider, const FGuid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const FGuid& guid) override;

private:
    Graphics::EDeviceAPI _deviceAPI;
    Graphics::SCGraphicsWindow _window;
    Graphics::FPresentationParameters _presentationParameters;

    Graphics::FDeviceEncapsulator _deviceEncapsulator;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
