#pragma once

#include "Application.h"

#include "ApplicationBase.h"
#include "Window/WindowListener.h"

#include "RHI_fwd.h"
#include "HAL/TargetRHI.h"

#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"
#include "Modular/ModularDomain.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FApplicationWindow : public FApplicationBase, private IWindowListener {
public:
    FApplicationWindow(FModularDomain& domain, FString&& name, bool needRHI);
    virtual ~FApplicationWindow() override;

    FMainWindow& Main() const { return *_main; }
    IInputService& Input() const { return *_input; }
    IRHIService& RHI() const { return *_rhi; }
    IWindowService& Window() const { return *_window; }

    virtual void Start() override;
    virtual void Shutdown() override;

    virtual bool PumpMessages() NOEXCEPT override;
    virtual void Tick(FTimespan dt) override;

protected:
    virtual void OnWindowFocus(bool enabled) NOEXCEPT override;
    virtual void OnWindowMove(const int2& pos) NOEXCEPT override;
    virtual void OnWindowResize(const uint2& size) NOEXCEPT override;

private:
    void UpdateTickRateFromRefreshRate_();

    const TPtrRef<const ITargetRHI> _targetRHI;

    PMainWindow _main;
    UInputService _input;
    URHIService _rhi;
    UWindowService _window;

    bool _windowWasResized{ false };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
