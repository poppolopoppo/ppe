#pragma once

#include "Application.h"

#include "ApplicationBase.h"
#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"

namespace PPE {
namespace Application {
FWD_REFPTR(WindowBase);
FWD_INTEFARCE_UNIQUEPTR(InputService);
FWD_INTEFARCE_UNIQUEPTR(WindowService);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FApplicationWindow : public FApplicationBase {
public:
    explicit FApplicationWindow(const FModularDomain& domain, FWString&& name);
    explicit FApplicationWindow(const FModularDomain& domain, FWString&& name, size_t width, size_t height);
    explicit FApplicationWindow(const FModularDomain& domain, FWString&& name, int left, int top, size_t width, size_t height);

    virtual ~FApplicationWindow();

    FWindowBase& Main() const { return *_main; }
    IInputService& Input() const { return *_input; }
    IWindowService& Window() const { return *_window; }

    virtual void Start() override;
    virtual void PumpMessages() override;
    virtual void Tick(FTimespan dt) override;
    virtual void Shutdown() override;

private:
    PWindowBase _main;
    UInputService _input;
    UWindowService _window;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
