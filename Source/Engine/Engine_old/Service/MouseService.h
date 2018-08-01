#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/Service_fwd.h"

#include "Core.Engine/Input/State/MouseInputHandler.h"

namespace Core {
struct FGuid;

namespace Graphics {
FWD_REFPTR(GraphicsWindow);
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IMouseService : public IService {
protected:
    IMouseService(const char *serviceName, int servicePriority)
    :   IService(serviceName, servicePriority) {}
public:
    virtual ~IMouseService() {}

    virtual Engine::FMouseInputHandler *FMouseInputHandler() = 0;
    virtual const Engine::FMouseInputHandler *FMouseInputHandler() const = 0;

    ENGINESERVICE_GUID_DECL(IMouseService);
};
//----------------------------------------------------------------------------
class FDefaultMouseService : public IMouseService {
public:
    explicit FDefaultMouseService(Graphics::FGraphicsWindow *window);
    virtual ~FDefaultMouseService();

    const Graphics::FGraphicsWindow *Window() const { return _window.get(); }

    virtual Engine::FMouseInputHandler *FMouseInputHandler() override;
    virtual const Engine::FMouseInputHandler *FMouseInputHandler() const override;

    virtual void Start(IServiceProvider *provider, const FGuid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const FGuid& guid) override;

private:
    Graphics::SGraphicsWindow _window;
    Engine::FMouseInputHandler _mouseInputHandler;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
