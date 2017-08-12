#include "stdafx.h"

#include "ApplicationGraphics.h"

#include "Core/Time/Timeline.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationGraphics::FApplicationGraphics(
    const wchar_t *appname,
    int left, int top,
    const Graphics::FPresentationParameters& pp,
    Graphics::EDeviceAPI api,
    FTimespan tickRate/* = Timespan_60hz()*/ )
:   FApplicationWindow(appname, left, top, pp.BackBufferWidth(), pp.BackBufferHeight())
,   _pp(pp)
,   _api(api)
,   _tickRate(tickRate) {
}
//----------------------------------------------------------------------------
FApplicationGraphics::~FApplicationGraphics() {
    Assert(nullptr == _deviceEncapsulator);
}
//----------------------------------------------------------------------------
void FApplicationGraphics::Start() {
    FApplicationWindow::Start();

    Assert(nullptr == _deviceEncapsulator);
    _deviceEncapsulator.reset(new Graphics::FDeviceEncapsulator());
    _deviceEncapsulator->Create(_api, Window().Handle(), _pp);

    if (_deviceEncapsulator->Device())
        Services().Register<Graphics::IDeviceAPIEncapsulator>(_deviceEncapsulator->Device());
    if (_deviceEncapsulator->Immediate())
        Services().Register<Graphics::IDeviceAPIContext>(_deviceEncapsulator->Immediate());

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    if (_deviceEncapsulator->Diagnostics())
        Services().Register<Graphics::IDeviceAPIDiagnostics>(_deviceEncapsulator->Diagnostics());
#endif
}
//----------------------------------------------------------------------------
void FApplicationGraphics::Shutdown() {
    Assert(nullptr != _deviceEncapsulator);

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    if (_deviceEncapsulator->Diagnostics())
        Services().Unregister<Graphics::IDeviceAPIDiagnostics>(_deviceEncapsulator->Diagnostics());
#endif

    if (_deviceEncapsulator->Immediate())
        Services().Unregister<Graphics::IDeviceAPIContext>(_deviceEncapsulator->Immediate());
    if (_deviceEncapsulator->Device())
        Services().Unregister<Graphics::IDeviceAPIEncapsulator>(_deviceEncapsulator->Device());

    _deviceEncapsulator->Destroy();
    _deviceEncapsulator.reset();

    FApplicationWindow::Shutdown();
}
//----------------------------------------------------------------------------
void FApplicationGraphics::RenderLoop() {
    Assert(_deviceEncapsulator);

    FTimeline clock = FTimeline::StartNow();
    FTimeline realtime = clock;

    LoadContent();

    LOG(Info, L"[Application][Graphics] Start render loop");

    Graphics::IDeviceAPIEncapsulator *const device = _deviceEncapsulator->Device();

    device->SetViewport(_pp.Viewport());

    device->SetRenderTarget(device->BackBufferRenderTarget(),
                            device->BackBufferDepthStencil() );

    Update_BeforeDispatch();

    while (false == PumpAllMessages_ReturnIfQuit()) {
        FTimespan elapsed;
        bool run = true;
        if (_tickRate > 0)
            run = clock.Tick_Every(_tickRate, elapsed);
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

    LOG(Info, L"[Application][Graphics] Stop render loop");

    UnloadContent();
}
//----------------------------------------------------------------------------
void FApplicationGraphics::LoadContent() {
    LOG(Info, L"[Application][Graphics] LoadContent()");
}
//----------------------------------------------------------------------------
void FApplicationGraphics::UnloadContent() {
    LOG(Info, L"[Application][Graphics] UnloadContent()");
}
//----------------------------------------------------------------------------
void FApplicationGraphics::Update(const FTimeline&/* time */) {}
//----------------------------------------------------------------------------
void FApplicationGraphics::Draw(const FTimeline&/* time */) {}
//----------------------------------------------------------------------------
void FApplicationGraphics::Present() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
