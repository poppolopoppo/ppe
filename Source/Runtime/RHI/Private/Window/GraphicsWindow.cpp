#include "stdafx.h"

#include "GraphicsWindow.h"

#include "Device/DeviceEncapsulator.h"
#include "Device/PresentationParameters.h"
#include "Device/Texture/SurfaceFormat.h"

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGraphicsWindow::FGraphicsWindow(
    const wchar_t *title,
    int left, int top,
    size_t width, size_t height,
    FBasicWindow *parent /* = nullptr */)
:   FBasicWindow(title, left, top, width, height, parent)
,   _fixedTimeStep(false) {}
//----------------------------------------------------------------------------
FGraphicsWindow::~FGraphicsWindow() {}
//----------------------------------------------------------------------------
void FGraphicsWindow::RenderLoop(FDeviceEncapsulator *deviceEncapsulator) {
    Assert(deviceEncapsulator);

    FTimeline clock = FTimeline::StartNow();
    FTimeline realtime = clock;

    Initialize(clock);
    LoadContent();

    LOG(Info, L"[FGraphicsWindow] Start render loop");

    IDeviceAPIEncapsulator *const device = deviceEncapsulator->Device();
    device->SetRenderTarget(device->BackBufferRenderTarget(),
                            device->BackBufferDepthStencil() );

    Update_BeforeDispatch();

    while (false == PumpAllMessages_ReturnIfQuit()) {
        FTimespan elapsed;
        bool run = true;
        if (_fixedTimeStep)
            run = clock.Tick_Target60FPS(elapsed);
        else
            clock.Tick();

        if (run) {
            realtime.Tick(clock);

            Update_AfterDispatch();
            Update(realtime);
            Update_BeforeDispatch();

            Draw(realtime);
            Present();

            deviceEncapsulator->Present();
        }
    }

    Update_AfterDispatch();

    LOG(Info, L"[FGraphicsWindow] Stop render loop");

    UnloadContent();
    Destroy();
}
//----------------------------------------------------------------------------
void FGraphicsWindow::Initialize(const FTimeline&/* time */) {
    LOG(Info, L"[FGraphicsWindow] Initialize()");
}
//----------------------------------------------------------------------------
void FGraphicsWindow::Destroy() {
    LOG(Info, L"[FGraphicsWindow] Destroy()");
}
//----------------------------------------------------------------------------
void FGraphicsWindow::LoadContent() {
    LOG(Info, L"[FGraphicsWindow] LoadContent()");
}
//----------------------------------------------------------------------------
void FGraphicsWindow::UnloadContent() {
    LOG(Info, L"[FGraphicsWindow] UnloadContent()");
}
//----------------------------------------------------------------------------
void FGraphicsWindow::Update(const FTimeline&/* time */) {}
//----------------------------------------------------------------------------
void FGraphicsWindow::Draw(const FTimeline&/* time */) {}
//----------------------------------------------------------------------------
void FGraphicsWindow::Present() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
