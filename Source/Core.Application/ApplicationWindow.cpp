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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
