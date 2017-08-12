#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/ApplicationBase.h"
#include "Core.Application/Input/GamepadState.h"
#include "Core.Application/Input/KeyboardState.h"
#include "Core.Application/Input/MouseState.h"

#include "Core.Graphics/Window/BasicWindow.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/Delegate.h"
#include "Core/Meta/Event.h"

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

    FGamepadInputHandler& Gamepad() { return *_gamepad; }
    FKeyboardInputHandler& Keyboard() { return *_keyboard; }
    FMouseInputHandler& Mouse() { return *_mouse; }

    const FGamepadInputHandler& Gamepad() const { return *_gamepad; }
    const FKeyboardInputHandler& Keyboard() const { return *_keyboard; }
    const FMouseInputHandler& Mouse() const { return *_mouse; }

    const Graphics::FBasicWindow& Window() const { return *this; }

    virtual void Start() override;
    virtual void Shutdown() override;

    PUBLIC_EVENT(OnGamepadButtonUp,        void, FApplicationWindow* app, const FGamepadState& gamepad, size_t index);
    PUBLIC_EVENT(OnGamepadButtonPressed,void, FApplicationWindow* app, const FGamepadState& gamepad, size_t index);
    PUBLIC_EVENT(OnGamepadButtonDown,    void, FApplicationWindow* app, const FGamepadState& gamepad, size_t index);
    PUBLIC_EVENT(OnGamepadLeftStick,    void, FApplicationWindow* app, const FGamepadState& gamepad, size_t index);
    PUBLIC_EVENT(OnGamepadRightStick,    void, FApplicationWindow* app, const FGamepadState& gamepad, size_t index);
    PUBLIC_EVENT(OnGamepadLeftTrigger,    void, FApplicationWindow* app, const FGamepadState& gamepad, size_t index);
    PUBLIC_EVENT(OnGamepadRightTrigger,    void, FApplicationWindow* app, const FGamepadState& gamepad, size_t index);
    PUBLIC_EVENT(OnGamepadConnect,        void, FApplicationWindow* app, const FGamepadState& gamepad, size_t index);
    PUBLIC_EVENT(OnGamepadDisconnect,    void, FApplicationWindow* app, const FGamepadState& gamepad, size_t index);

    PUBLIC_EVENT(OnKeyboardKeyUp,        void, FApplicationWindow* app, const FKeyboardState& keyboard);
    PUBLIC_EVENT(OnKeyboardKeyPressed,    void, FApplicationWindow* app, const FKeyboardState& keyboard);
    PUBLIC_EVENT(OnKeyboardKeyDown,        void, FApplicationWindow* app, const FKeyboardState& keyboard);

    PUBLIC_EVENT(OnMouseButtonUp,        void, FApplicationWindow* app, const FMouseState& mouse);
    PUBLIC_EVENT(OnMouseButtonPressed,    void, FApplicationWindow* app, const FMouseState& mouse);
    PUBLIC_EVENT(OnMouseButtonDown,        void, FApplicationWindow* app, const FMouseState& mouse);
    PUBLIC_EVENT(OnMouseMove,            void, FApplicationWindow* app, const FMouseState& mouse);
    PUBLIC_EVENT(OnMouseEnter,            void, FApplicationWindow* app, const FMouseState& mouse);
    PUBLIC_EVENT(OnMouseLeave,            void, FApplicationWindow* app, const FMouseState& mouse);

protected:
    virtual void Update_AfterDispatch() override;

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
