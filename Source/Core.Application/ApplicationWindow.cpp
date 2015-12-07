#include "stdafx.h"

#include "ApplicationWindow.h"

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
ApplicationWindow::~ApplicationWindow() {}
//----------------------------------------------------------------------------
void ApplicationWindow::Start() {
    ApplicationBase::Start();

    BasicWindow::Show();

    Assert(nullptr == _keyboard);
    _keyboard.reset(new KeyboardInputHandler());
    RegisterMessageHandler(_keyboard.get());
    Services().Add<IKeyboardService>(*_keyboard);

    Assert(nullptr == _mouse);
    _mouse.reset(new MouseInputHandler());
    RegisterMessageHandler(_mouse.get());
    Services().Add<IMouseService>(*_mouse);
}
//----------------------------------------------------------------------------
void ApplicationWindow::Shutdown() {
    ApplicationBase::Shutdown(); // destroys engine services, including this device service

    Assert(nullptr != _mouse);
    Services().Remove<IMouseService>(*_mouse);
    UnregisterMessageHandler(_mouse.get());
    _mouse.reset();

    Assert(nullptr != _keyboard);
    Services().Remove<IKeyboardService>(*_keyboard);
    UnregisterMessageHandler(_keyboard.get());
    _keyboard.reset();

    BasicWindow::Close();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
