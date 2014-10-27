#include "stdafx.h"

#include "GraphicsWindow.h"

#include "Device/DeviceEncapsulator.h"
#include "Device/PresentationParameters.h"
#include "Device/Texture/SurfaceFormat.h"

#include "Core/Diagnostic/CurrentProcess.h"
#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
GraphicsWindow::GraphicsWindow(
    const wchar_t *title,
    int left, int top,
    size_t width, size_t height,
    BasicWindow *parent /* = nullptr */)
:   BasicWindow(title, left, top, width, height, parent) {}
//----------------------------------------------------------------------------
GraphicsWindow::~GraphicsWindow() {}
//----------------------------------------------------------------------------
void GraphicsWindow::RenderLoop(DeviceEncapsulator *deviceEncapsulator) {
    Assert(deviceEncapsulator);

    Timeline clock = Timeline::StartNow();
    Timeline realtime = clock;

    Initialize(clock);
    LoadContent();

    LOG(Information, L"[GraphicsWindow] Start render loop");

    deviceEncapsulator->Context()->SetRenderTarget(
        deviceEncapsulator->Device()->BackBufferRenderTarget(),
        deviceEncapsulator->Device()->BackBufferDepthStencil() );

    Update_BeforeDispatch();

    bool keepLooping = true;
    do {
        WindowMessage msg;
        MessageLParam lparam;
        MessageWParam wparam;
        while (PumpMessage(msg, lparam, wparam)) {
            keepLooping = (msg != WindowMessage::Quit);
        }

        Timespan elapsed;
        if (clock.Tick_Target60FPS(elapsed)) {
            realtime.Tick(clock);

            Update_AfterDispatch();
            Update(realtime);
            Update_BeforeDispatch();

            Draw(realtime);
            Present();

            deviceEncapsulator->Present();
        }
    }
    while (keepLooping);

    Update_AfterDispatch();

    LOG(Information, L"[GraphicsWindow] Stop render loop");

    UnloadContent();
    Destroy();
}
//----------------------------------------------------------------------------
void GraphicsWindow::Initialize(const Timeline& time) {
    LOG(Information, L"[GraphicsWindow] Initialize()");
}
//----------------------------------------------------------------------------
void GraphicsWindow::Destroy() {
    LOG(Information, L"[GraphicsWindow] Destroy()");
}
//----------------------------------------------------------------------------
void GraphicsWindow::LoadContent() {
    LOG(Information, L"[GraphicsWindow] LoadContent()");
}
//----------------------------------------------------------------------------
void GraphicsWindow::UnloadContent() {
    LOG(Information, L"[GraphicsWindow] UnloadContent()");
}
//----------------------------------------------------------------------------
void GraphicsWindow::Update(const Timeline& time) {}
//----------------------------------------------------------------------------
void GraphicsWindow::Draw(const Timeline& time) {}
//----------------------------------------------------------------------------
void GraphicsWindow::Present() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
