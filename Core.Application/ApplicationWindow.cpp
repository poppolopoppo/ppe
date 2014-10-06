#include "stdafx.h"

#include "ApplicationWindow.h"

#include "Core.Engine/DeviceEncapsulatorService.h"

#include "Core.Graphics/SurfaceFormat.h"
#include "Core.Graphics/WindowMessage.h"

#include "Core/Logger.h"

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
,   _deviceService(new Engine::DefaultDeviceEncapsulatorService(deviceAPI, this, presentationParameters)) {
    ENGINESERVICE_REGISTER(Engine::IDeviceEncapsulatorService, Services(), _deviceService.get());
}
//----------------------------------------------------------------------------
ApplicationWindow::~ApplicationWindow() {
    ENGINESERVICE_UNREGISTER(Engine::IDeviceEncapsulatorService, Services(), _deviceService.get());
}
//----------------------------------------------------------------------------
const Graphics::DeviceEncapsulator *ApplicationWindow::DeviceEncapsulator() const {
    Assert(_deviceService->ServiceAvailable());
    return _deviceService->DeviceEncapsulator();
}
//----------------------------------------------------------------------------
void ApplicationWindow::Start() {
    GraphicsWindow::Show();

    GraphicsWindow::RegisterMessageHandler(&_mouse);
    GraphicsWindow::RegisterMessageHandler(&_keyboard);

    ApplicationBase::Start(); // creates engine services, including this device service

    GraphicsWindow::RenderLoop(_deviceService->DeviceEncapsulator());
}
//----------------------------------------------------------------------------
void ApplicationWindow::Shutdown() {
    ApplicationBase::Shutdown(); // destroys engine services, including this device service

    GraphicsWindow::UnregisterMessageHandler(&_keyboard);
    GraphicsWindow::UnregisterMessageHandler(&_mouse);

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
