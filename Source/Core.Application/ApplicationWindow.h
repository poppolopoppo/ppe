#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/ApplicationBase.h"

#include "Core.Graphics/Window/BasicWindow.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
namespace Application {
class GamepadInputHandler;
class KeyboardInputHandler;
class MouseInputHandler;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ApplicationWindow :
    public ApplicationBase
,   protected Graphics::BasicWindow
{
public:
    ApplicationWindow(  const wchar_t *appname,
                        int left, int top,
                        size_t width, size_t height );
    virtual ~ApplicationWindow();

    const GamepadInputHandler& Gamepad() const { return *_gamepad; }
    const KeyboardInputHandler& Keyboard() const { return *_keyboard; }
    const MouseInputHandler& Mouse() const { return *_mouse; }

    const Graphics::BasicWindow& Window() const { return *this; }

    virtual void Start() override;
    virtual void Shutdown() override;

private:
    UniquePtr<GamepadInputHandler> _gamepad;
    UniquePtr<KeyboardInputHandler> _keyboard;
    UniquePtr<MouseInputHandler> _mouse;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
