#pragma once

#include "Application.h"

#include "ApplicationBase.h"
#include "Window/WindowListener.h"

#include "RHI_fwd.h"
#include "HAL/TargetRHI.h"

#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"
#include "Modular/ModularDomain.h"
#include "Time/TimedScope.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FApplicationWindow : public FApplicationBase, private IWindowListener {
public:
    FApplicationWindow(FModularDomain& domain, FString&& name, bool needRHI);
    virtual ~FApplicationWindow() override;

    IInputService& Input() const { return *_input; }
    IRHIService& RHI() const { return *_rhi; }
    IWindowService& Window() const { return *_window; }

    const FMovingAverageTimer& MessageTime() const { return _messageTime; }
    const FMovingAverageTimer& TickTime() const { return _tickTime; }

    virtual void Start() override;
    virtual void Run() override;
    virtual void Shutdown() override;

    virtual bool PumpMessages() NOEXCEPT override;

protected:
    virtual void Tick(FTimespan dt) override;

    virtual void Update(FTimespan dt);
    virtual void Render(RHI::IFrameGraph& fg, FTimespan dt);
    virtual void ViewportResized(const FRHISurfaceCreateInfo& surface);

private:
    virtual void OnWindowShow(bool visible) NOEXCEPT override final;
    virtual void OnWindowFocus(bool enabled) NOEXCEPT override final;
    virtual void OnWindowMove(const int2& pos) NOEXCEPT override final;
    virtual void OnWindowResize(const uint2& size) NOEXCEPT override final;
    virtual void OnWindowClose() NOEXCEPT override final;

    void UpdateTickRateFromRefreshRate_();

    const TPtrRef<const ITargetRHI> _targetRHI;

    PMainWindow _main;
    UInputService _input;
    URHIService _rhi;
    UWindowService _window;

    FMovingAverageTimer _messageTime;
    FMovingAverageTimer _tickTime;

    FEventHandle _onViewportResized;

    bool _windowWasResized{ false };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
