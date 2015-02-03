#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/ApplicationBase.h"

#include "Core.Engine/Service/Service_fwd.h"
#include "Core.Engine/Input/State/KeyboardInputHandler.h"
#include "Core.Engine/Input/State/MouseInputHandler.h"

#include "Core.Graphics/Window/GraphicsWindow.h"

#include "Core/Memory/RefPtr.h"

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

    const Engine::KeyboardInputHandler& Keyboard() const;
    const Engine::MouseInputHandler& Mouse() const;
    const Graphics::DeviceEncapsulator& DeviceEncapsulator() const;

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
    Engine::PKeyboardService _keyboardService;
    Engine::PMouseService _mouseService;
    Engine::PDeviceEncapsulatorService _deviceService;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
