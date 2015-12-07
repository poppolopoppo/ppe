#include "stdafx.h"

#include "ApplicationGraphics.h"

#include "Core/Time/Timeline.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ApplicationGraphics::ApplicationGraphics(
    const wchar_t *appname,
    int left, int top,
    const Graphics::PresentationParameters& pp,
    Graphics::DeviceAPI api,
    bool fixedTimeStep/* = true */)
:   ApplicationWindow(appname, left, top, pp.BackBufferWidth(), pp.BackBufferHeight())
,   _api(api)
,   _pp(pp)
,   _fixedTimeStep(fixedTimeStep) {
}
//----------------------------------------------------------------------------
ApplicationGraphics::~ApplicationGraphics() {
    Assert(nullptr == _deviceEncapsulator);
}
//----------------------------------------------------------------------------
void ApplicationGraphics::Start() {
    ApplicationWindow::Start();

    Assert(nullptr == _deviceEncapsulator);
    _deviceEncapsulator.reset(new Graphics::DeviceEncapsulator());
    _deviceEncapsulator->Create(_api, Window().Handle(), _pp);

    if (_deviceEncapsulator->Device())
        Services().Add<Graphics::IDeviceAPIEncapsulator>(*_deviceEncapsulator->Device());
    if (_deviceEncapsulator->Immediate())
        Services().Add<Graphics::IDeviceAPIContext>(*_deviceEncapsulator->Immediate());

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    if (_deviceEncapsulator->Diagnostics())
        Services().Add<Graphics::IDeviceAPIDiagnostics>(*_deviceEncapsulator->Diagnostics());
#endif
}
//----------------------------------------------------------------------------
void ApplicationGraphics::Shutdown() {
    Assert(nullptr != _deviceEncapsulator);

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    if (_deviceEncapsulator->Diagnostics())
        Services().Remove<Graphics::IDeviceAPIDiagnostics>(*_deviceEncapsulator->Diagnostics());
#endif

    if (_deviceEncapsulator->Immediate())
        Services().Remove<Graphics::IDeviceAPIContext>(*_deviceEncapsulator->Immediate());
    if (_deviceEncapsulator->Device())
        Services().Remove<Graphics::IDeviceAPIEncapsulator>(*_deviceEncapsulator->Device());

    _deviceEncapsulator->Destroy();
    _deviceEncapsulator.reset();

    ApplicationWindow::Shutdown();
}
//----------------------------------------------------------------------------
void ApplicationGraphics::RenderLoop() {
    Assert(_deviceEncapsulator);

    Timeline clock = Timeline::StartNow();
    Timeline realtime = clock;

    LoadContent();

    LOG(Information, L"[Application][Graphics] Start render loop");

    Graphics::IDeviceAPIEncapsulator *const device = _deviceEncapsulator->Device();
    device->SetRenderTarget(device->BackBufferRenderTarget(),
                            device->BackBufferDepthStencil() );

    Update_BeforeDispatch();

    while (false == PumpAllMessages_ReturnIfQuit()) {
        Timespan elapsed;
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

            _deviceEncapsulator->Present();
        }
    }

    Update_AfterDispatch();

    LOG(Information, L"[Application][Graphics] Stop render loop");

    UnloadContent();
}
//----------------------------------------------------------------------------
void ApplicationGraphics::LoadContent() {
    LOG(Information, L"[Application][Graphics] LoadContent()");
}
//----------------------------------------------------------------------------
void ApplicationGraphics::UnloadContent() {
    LOG(Information, L"[Application][Graphics] UnloadContent()");
}
//----------------------------------------------------------------------------
void ApplicationGraphics::Update(const Timeline&/* time */) {}
//----------------------------------------------------------------------------
void ApplicationGraphics::Draw(const Timeline&/* time */) {}
//----------------------------------------------------------------------------
void ApplicationGraphics::Present() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core