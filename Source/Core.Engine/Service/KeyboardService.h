#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/Service_fwd.h"

#include "Core.Engine/Input/State/KeyboardInputHandler.h"

namespace Core {
struct Guid;

namespace Graphics {
FWD_REFPTR(GraphicsWindow);
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IKeyboardService : public IService {
protected:
    IKeyboardService(const char *serviceName, int servicePriority)
    :   IService(serviceName, servicePriority) {}
public:
    virtual ~IKeyboardService() {}

    virtual Engine::KeyboardInputHandler *KeyboardInputHandler() = 0;
    virtual const Engine::KeyboardInputHandler *KeyboardInputHandler() const = 0;

    ENGINESERVICE_GUID_DECL(IKeyboardService);
};
//----------------------------------------------------------------------------
class DefaultKeyboardService : public IKeyboardService {
public:
    explicit DefaultKeyboardService(Graphics::GraphicsWindow *window);
    virtual ~DefaultKeyboardService();

    const Graphics::GraphicsWindow *Window() const { return _window.get(); }

    virtual Engine::KeyboardInputHandler *KeyboardInputHandler() override;
    virtual const Engine::KeyboardInputHandler *KeyboardInputHandler() const override;

    virtual void Start(IServiceProvider *provider, const Guid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const Guid& guid) override;

private:
    Graphics::SGraphicsWindow _window;
    Engine::KeyboardInputHandler _KeyboardInputHandler;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
