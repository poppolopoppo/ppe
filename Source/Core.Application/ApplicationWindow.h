#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/ApplicationBase.h"

#include "Core.Engine/Service/Service_fwd.h"

#include "Core.Graphics/Window/GraphicsWindow.h"

#include "Core/Memory/RefPtr.h"

#include "Core.Application/Input/State/KeyboardInputHandler.h"
#include "Core.Application/Input/State/MouseInputHandler.h"

namespace Core {
namespace Graphics {
class DeviceEncapsulator;
}

namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ApplicationWindow :
    public ApplicationBase
,   protected Graphics::GraphicsWindow
{
public:
    ApplicationWindow(  const wchar_t *appname,
                        const Graphics::DeviceAPI deviceAPI,
                        const Graphics::PresentationParameters& presentationParameters,
                        int left, int top );
    virtual ~ApplicationWindow();

    const MouseInputHandler& Mouse() const { return _mouse; }
    const KeyboardInputHandler& Keyboard() const { return _keyboard; }

    const Graphics::DeviceEncapsulator *DeviceEncapsulator() const;

    virtual void Start() override;
    virtual void Shutdown() override;

protected:
    virtual void LoadContent() override;
    virtual void UnloadContent() override;

    virtual void Update(const Timeline& time) override;
    virtual void Draw(const Timeline& time) override;

    virtual void Present() override;

    virtual void OnLoseFocus() override;

private:
    MouseInputHandler _mouse;
    KeyboardInputHandler _keyboard;
    Engine::PDeviceEncapsulatorService _deviceService;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
