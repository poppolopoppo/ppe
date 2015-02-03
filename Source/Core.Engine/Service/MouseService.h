#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/Service_fwd.h"

#include "Core.Engine/Input/State/MouseInputHandler.h"

namespace Core {
struct Guid;

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

    virtual Engine::MouseInputHandler *MouseInputHandler() = 0;
    virtual const Engine::MouseInputHandler *MouseInputHandler() const = 0;

    ENGINESERVICE_GUID_DECL(IMouseService);
};
//----------------------------------------------------------------------------
class DefaultMouseService : public IMouseService {
public:
    explicit DefaultMouseService(Graphics::GraphicsWindow *window);
    virtual ~DefaultMouseService();

    const Graphics::GraphicsWindow *Window() const { return _window.get(); }

    virtual Engine::MouseInputHandler *MouseInputHandler() override;
    virtual const Engine::MouseInputHandler *MouseInputHandler() const override;

    virtual void Start(IServiceProvider *provider, const Guid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const Guid& guid) override;

private:
    Graphics::SGraphicsWindow _window;
    Engine::MouseInputHandler _mouseInputHandler;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
