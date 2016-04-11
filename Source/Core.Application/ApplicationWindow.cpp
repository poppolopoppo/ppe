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
ApplicationWindow::ApplicationWindow(
    const wchar_t *appname,
    int left, int top,
    size_t width, size_t height )
:   ApplicationBase(appname)
,   BasicWindow(appname, left, top, width, height)
{}
//----------------------------------------------------------------------------
ApplicationWindow::~ApplicationWindow() {
    Assert(nullptr == _keyboard);
    Assert(nullptr == _mouse);
}
//----------------------------------------------------------------------------
void ApplicationWindow::Start() {
    ApplicationBase::Start();

    BasicWindow::Show();

    Services().Create<IGamepadService>(_gamepad);
    RegisterMessageHandler(_gamepad.get());

    Services().Create<IKeyboardService>(_keyboard);
    RegisterMessageHandler(_keyboard.get());

    Services().Create<IMouseService>(_mouse);
    RegisterMessageHandler(_mouse.get());
}
//----------------------------------------------------------------------------
void ApplicationWindow::Shutdown() {
    ApplicationBase::Shutdown(); // destroys engine services, including this device service

    UnregisterMessageHandler(_mouse.get());
    Services().Destroy<IMouseService>(_mouse);

    UnregisterMessageHandler(_keyboard.get());
    Services().Destroy<IKeyboardService>(_keyboard);

    UnregisterMessageHandler(_gamepad.get());
    Services().Destroy<IGamepadService>(_gamepad);

    BasicWindow::Close();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
