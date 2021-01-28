#include "stdafx.h"

#include "ApplicationWindow.h"

#include "Service/InputService.h"
#include "Service/RHIService.h"
#include "Service/WindowService.h"

#include "Window/WindowBase.h"
#include "Window/WindowRHI.h"

#include "Thread/ThreadPool.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename... _Args>
static void CreateApplicationWindow_(
    UInputService* pInput,
    URHIService* pRHI,
    UWindowService* pWindow,
    PWindowBase* pMain,
    const FWString& name,
    bool needRHI,
    _Args&&... args) {

    IInputService::MakeDefault(pInput);
    IWindowService::MakeDefault(pWindow);

    if (needRHI) {
        IRHIService::MakeDefault(pRHI);

        PWindowRHI windowRHI;
        (*pWindow)->CreateRHIWindow(&windowRHI, FWString(name), std::forward<_Args>(args)...);

        *pMain = std::move(windowRHI);
    }
    else {
        PWindowBare windowBare;
        (*pWindow)->CreateMainWindow(&windowBare, FWString(name), std::forward<_Args>(args)...);

        *pMain = std::move(windowBare);
    }

    (*pInput)->SetupWindow(**pMain);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationWindow::FApplicationWindow(const FModularDomain& domain, FWString&& name, bool needRHI)
:   FApplicationBase(domain, std::move(name)) {
    CreateApplicationWindow_(&_input, &_rhi, &_window, &_main, Name(), needRHI);
}
//----------------------------------------------------------------------------
FApplicationWindow::FApplicationWindow(const FModularDomain& domain, FWString&& name, bool needRHI, size_t width, size_t height)
:   FApplicationBase(domain, std::move(name)) {
    CreateApplicationWindow_(&_input, &_rhi, &_window, &_main, Name(), needRHI, width, height);
}
//----------------------------------------------------------------------------
FApplicationWindow::FApplicationWindow(const FModularDomain& domain, FWString&& name, bool needRHI, int left, int top, size_t width, size_t height)
:   FApplicationBase(domain, std::move(name)) {
    CreateApplicationWindow_(&_input, &_rhi, &_window, &_main, Name(), needRHI, left, top, width, height);
}
//----------------------------------------------------------------------------
FApplicationWindow::~FApplicationWindow() = default;
//----------------------------------------------------------------------------
void FApplicationWindow::Start() {
    _window->SetMainWindow(_main.get());

    auto& services = Services();
    services.Add<IInputService>(_input.get());
    services.Add<IWindowService>(_window.get());

    if (_rhi) {
        services.Add<IRHIService>(_rhi.get());

        _rhi->CreateMainFrameGraph(checked_cast<FWindowRHI*>(_main.get()));
    }

    VerifyRelease(_main->Show());
    VerifyRelease(_main->SetFocus());

    FApplicationBase::Start();
}
//----------------------------------------------------------------------------
void FApplicationWindow::Shutdown() {
    FApplicationBase::Shutdown();

    if (_main->Visible())
        VerifyRelease(_main->Close());

    auto& services = Services();
    if (_rhi) {
        FWindowRHI* const windowRHI = checked_cast<FWindowRHI*>(_main.get());

        _rhi->DestroyMainFrameGraph(windowRHI);

        services.CheckedRemove<IRHIService>(_rhi.get());
    }

    services.CheckedRemove<IWindowService>(_window.get());
    services.CheckedRemove<IInputService>(_input.get());

    _window->SetMainWindow(nullptr);
}
//----------------------------------------------------------------------------
bool FApplicationWindow::PumpMessages() NOEXCEPT {
    if (FApplicationBase::PumpMessages() &&
        _main->PumpMessages() ) {
        _input->Poll();
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
void FApplicationWindow::Tick(FTimespan dt) {
    FApplicationBase::Tick(dt);

    _input->Update(dt);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
