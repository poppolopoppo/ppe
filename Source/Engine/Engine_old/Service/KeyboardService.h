#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/Service_fwd.h"

#include "Core.Engine/Input/State/KeyboardInputHandler.h"

namespace Core {
struct FGuid;

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

    virtual Engine::FKeyboardInputHandler *FKeyboardInputHandler() = 0;
    virtual const Engine::FKeyboardInputHandler *FKeyboardInputHandler() const = 0;

    ENGINESERVICE_GUID_DECL(IKeyboardService);
};
//----------------------------------------------------------------------------
class FDefaultKeyboardService : public IKeyboardService {
public:
    explicit FDefaultKeyboardService(Graphics::FGraphicsWindow *window);
    virtual ~FDefaultKeyboardService();

    const Graphics::FGraphicsWindow *Window() const { return _window.get(); }

    virtual Engine::FKeyboardInputHandler *FKeyboardInputHandler() override;
    virtual const Engine::FKeyboardInputHandler *FKeyboardInputHandler() const override;

    virtual void Start(IServiceProvider *provider, const FGuid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const FGuid& guid) override;

private:
    Graphics::SGraphicsWindow _window;
    Engine::FKeyboardInputHandler _KeyboardInputHandler;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
