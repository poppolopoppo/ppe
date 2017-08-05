#include "stdafx.h"

#include "ApplicationWindow.h"

#include "Input/GamepadInputHandler.h"
#include "Input/KeyboardInputHandler.h"
#include "Input/MouseInputHandler.h"

#include "Core.Graphics/Window/WindowMessage.h"

#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationWindow::FApplicationWindow(
    const wchar_t *appname,
    int left, int top,
    size_t width, size_t height )
:   FApplicationBase(appname)
,   FBasicWindow(appname, left, top, width, height)
{}
//----------------------------------------------------------------------------
FApplicationWindow::~FApplicationWindow() {
    Assert(nullptr == _keyboard);
    Assert(nullptr == _mouse);
}
//----------------------------------------------------------------------------
void FApplicationWindow::Start() {
    FApplicationBase::Start();

    FBasicWindow::Show();

    Services().Create<IGamepadService>(_gamepad);
    RegisterMessageHandler(_gamepad.get());

    Services().Create<IKeyboardService>(_keyboard);
    RegisterMessageHandler(_keyboard.get());

    Services().Create<IMouseService>(_mouse);
    RegisterMessageHandler(_mouse.get());
}
//----------------------------------------------------------------------------
void FApplicationWindow::Shutdown() {
    FApplicationBase::Shutdown(); // destroys engine services, including this device service

    UnregisterMessageHandler(_mouse.get());
    Services().Destroy<IMouseService>(_mouse);

    UnregisterMessageHandler(_keyboard.get());
    Services().Destroy<IKeyboardService>(_keyboard);

    UnregisterMessageHandler(_gamepad.get());
    Services().Destroy<IGamepadService>(_gamepad);

    FBasicWindow::Close();
}
//----------------------------------------------------------------------------
void FApplicationWindow::Update_AfterDispatch() {
	Graphics::FBasicWindow::Update_AfterDispatch();

	// Gamepad events
	forrange(i, 0, FMultiGamepadState::MaxConnected) {
		const FGamepadState& gamepad = _gamepad->State().Gamepads()[i];

		if (gamepad.OnConnect())
			_OnGamepadConnect.Invoke(this, gamepad, i);

		if (gamepad.HasButtonDown())
			_OnGamepadButtonUp.Invoke(this, gamepad, i);
		if (gamepad.HasButtonPressed())
			_OnGamepadButtonPressed.Invoke(this, gamepad, i);
		if (gamepad.HasButtonUp())
			_OnGamepadButtonUp.Invoke(this, gamepad, i);
		
		if (gamepad.LeftStickX().Smoothed() != 0 || gamepad.LeftStickY().Smoothed() != 0)
			_OnGamepadLeftStick.Invoke(this, gamepad, i);
		if (gamepad.RightStickX().Smoothed() != 0 || gamepad.RightStickY().Smoothed() != 0)
			_OnGamepadRightStick.Invoke(this, gamepad, i);

		if (gamepad.LeftTrigger().Smoothed() != 0)
			_OnGamepadLeftTrigger.Invoke(this, gamepad, i);
		if (gamepad.RightTrigger().Smoothed() != 0)
			_OnGamepadRightTrigger.Invoke(this, gamepad, i);

		if (gamepad.OnDisconnect())
			_OnGamepadDisconnect.Invoke(this, gamepad, i);
	}

	// Keyboard events
	{
		const FKeyboardState& keyboard = _keyboard->State();

		if (keyboard.HasKeyDown())
			_OnKeyboardKeyDown.Invoke(this, keyboard);
		if (keyboard.HasKeyPressed())
			_OnKeyboardKeyPressed.Invoke(this, keyboard);
		if (keyboard.HasKeyUp())
			_OnKeyboardKeyUp.Invoke(this, keyboard);
	}

	// Mouse events
	{
		const FMouseState& mouse = _mouse->State();

		if (mouse.OnEnter())
			_OnMouseEnter.Invoke(this, mouse);
		
		if (mouse.HasButtonDown())
			_OnMouseButtonDown.Invoke(this, mouse);
		if (mouse.HasButtonPressed())
			_OnMouseButtonPressed.Invoke(this, mouse);
		if (mouse.HasButtonUp())
			_OnMouseButtonUp.Invoke(this, mouse);
		
		if (mouse.HasMoved())
			_OnMouseMove.Invoke(this, mouse);

		if (mouse.OnLeave())
			_OnMouseLeave.Invoke(this, mouse);
	}
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
