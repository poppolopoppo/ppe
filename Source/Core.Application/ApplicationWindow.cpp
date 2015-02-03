#include "stdafx.h"

#include "ApplicationWindow.h"

#include "Core.Engine/Service/DeviceEncapsulatorService.h"
#include "Core.Engine/Service/KeyboardService.h"
#include "Core.Engine/Service/MouseService.h"

#include "Core.Graphics/Device/Texture/SurfaceFormat.h"
#include "Core.Graphics/Window/WindowMessage.h"

#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ApplicationWindow::ApplicationWindow(
    const wchar_t *appname,
    const Graphics::DeviceAPI deviceAPI,
    const Graphics::PresentationParameters& presentationParameters,
    int left, int top)
:   ApplicationBase(appname)
,   GraphicsWindow(appname, left, top, presentationParameters.BackBufferWidth(), presentationParameters.BackBufferHeight())
,   _keyboardService(new Engine::DefaultKeyboardService(this))
,   _mouseService(new Engine::DefaultMouseService(this))
,   _deviceService(new Engine::DefaultDeviceEncapsulatorService(deviceAPI, this, presentationParameters)) {

    ENGINESERVICE_REGISTER(Engine::IKeyboardService, Services(), _keyboardService.get());
    ENGINESERVICE_REGISTER(Engine::IMouseService, Services(), _mouseService.get());
    ENGINESERVICE_REGISTER(Engine::IDeviceEncapsulatorService, Services(), _deviceService.get());
}
//----------------------------------------------------------------------------
ApplicationWindow::~ApplicationWindow() {
    
    ENGINESERVICE_UNREGISTER(Engine::IDeviceEncapsulatorService, Services(), _deviceService.get());
    ENGINESERVICE_UNREGISTER(Engine::IMouseService, Services(), _mouseService.get());
    ENGINESERVICE_UNREGISTER(Engine::IKeyboardService, Services(), _keyboardService.get());
}
//----------------------------------------------------------------------------
const Engine::KeyboardInputHandler& ApplicationWindow::Keyboard() const {
    Assert(_keyboardService->ServiceAvailable());
    const Engine::KeyboardInputHandler *pkeyboard = _keyboardService->KeyboardInputHandler();
    Assert(pkeyboard);
    return *pkeyboard;
}
//----------------------------------------------------------------------------
const Engine::MouseInputHandler& ApplicationWindow::Mouse() const {
    Assert(_mouseService->ServiceAvailable());
    const Engine::MouseInputHandler *pmouse = _mouseService->MouseInputHandler();
    Assert(pmouse);
    return *pmouse;
}
//----------------------------------------------------------------------------
const Graphics::DeviceEncapsulator& ApplicationWindow::DeviceEncapsulator() const {
    Assert(_deviceService->ServiceAvailable());
    const Graphics::DeviceEncapsulator *pdevice = _deviceService->DeviceEncapsulator();
    Assert(pdevice);
    return *pdevice;
}
//----------------------------------------------------------------------------
void ApplicationWindow::Start() {
    GraphicsWindow::Show();

    ApplicationBase::Start(); // creates engine services, including this device service

    GraphicsWindow::RenderLoop(_deviceService->DeviceEncapsulator());
}
//----------------------------------------------------------------------------
void ApplicationWindow::Shutdown() {
    ApplicationBase::Shutdown(); // destroys engine services, including this device service

    GraphicsWindow::Close();
}
//----------------------------------------------------------------------------
void ApplicationWindow::LoadContent() {
    GraphicsWindow::LoadContent();
}
//----------------------------------------------------------------------------
void ApplicationWindow::UnloadContent() {
    GraphicsWindow::UnloadContent();
}
//----------------------------------------------------------------------------
void ApplicationWindow::Update(const Timeline& time) {
    GraphicsWindow::Update(time);
}
//----------------------------------------------------------------------------
void ApplicationWindow::Draw(const Timeline& time) {
    GraphicsWindow::Draw(time);
}
//----------------------------------------------------------------------------
void ApplicationWindow::Present() {
    GraphicsWindow::Present();
}
//----------------------------------------------------------------------------
void ApplicationWindow::OnLoseFocus() {
    GraphicsWindow::OnLoseFocus();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
