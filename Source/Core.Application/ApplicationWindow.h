#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/ApplicationBase.h"

#include "Core.Graphics/Window/BasicWindow.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
namespace Application {
class FGamepadInputHandler;
class FKeyboardInputHandler;
class FMouseInputHandler;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FApplicationWindow :
    public FApplicationBase
,   protected Graphics::FBasicWindow
{
public:
    FApplicationWindow(  const wchar_t *appname,
                        int left, int top,
                        size_t width, size_t height );
    virtual ~FApplicationWindow();

    const FGamepadInputHandler& Gamepad() const { return *_gamepad; }
    const FKeyboardInputHandler& Keyboard() const { return *_keyboard; }
    const FMouseInputHandler& Mouse() const { return *_mouse; }

    const Graphics::FBasicWindow& Window() const { return *this; }

    virtual void Start() override;
    virtual void Shutdown() override;

private:
    TUniquePtr<FGamepadInputHandler> _gamepad;
    TUniquePtr<FKeyboardInputHandler> _keyboard;
    TUniquePtr<FMouseInputHandler> _mouse;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
