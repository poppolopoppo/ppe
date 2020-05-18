#include "stdafx.h"

#include "ApplicationWindow.h"

#include "Service/InputService.h"
#include "Service/WindowService.h"
#include "Window/WindowMain.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename... _Args>
static void CreateApplicationWindow_(
    UInputService* input,
    UWindowService* window,
    PWindowBase* main,
    const FWString& name,
    _Args&&... args) {

    IInputService::MakeDefault(input);
    IWindowService::MakeDefault(window);

    (*window)->CreateMainWindow(main, FWString(name), std::forward<_Args>(args)...);
    (*input)->SetupWindow(**main);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationWindow::FApplicationWindow(const FModularDomain& domain, FWString&& name)
:   FApplicationBase(domain, std::move(name)) {
    CreateApplicationWindow_(&_input, &_window, &_main, Name());
}
//----------------------------------------------------------------------------
FApplicationWindow::FApplicationWindow(const FModularDomain& domain, FWString&& name, size_t width, size_t height)
:   FApplicationBase(domain, std::move(name)) {
    CreateApplicationWindow_(&_input, &_window, &_main, Name(), width, height);
}
//----------------------------------------------------------------------------
FApplicationWindow::FApplicationWindow(const FModularDomain& domain, FWString&& name, int left, int top, size_t width, size_t height)
:   FApplicationBase(domain, std::move(name)) {
    CreateApplicationWindow_(&_input, &_window, &_main, Name(), left, top, width, height);
}
//----------------------------------------------------------------------------
FApplicationWindow::~FApplicationWindow() = default;
//----------------------------------------------------------------------------
void FApplicationWindow::Start() {
    FApplicationBase::Start();

    _window->SetMainWindow(_main.get());

    auto& services = Services();
    services.Add<IInputService>(_input.get());
    services.Add<IWindowService>(_window.get());

    VerifyRelease(_main->Show());
    VerifyRelease(_main->SetFocus());
}
//----------------------------------------------------------------------------
void FApplicationWindow::Shutdown() {
    if (_main->Visible())
        VerifyRelease(_main->Close());

    auto& services = Services();
    services.CheckedRemove<IWindowService>(_window.get());
    services.CheckedRemove<IInputService>(_input.get());

    _window->SetMainWindow(nullptr);

    FApplicationBase::Shutdown();
}
//----------------------------------------------------------------------------
void FApplicationWindow::PumpMessages() {
    FApplicationBase::PumpMessages();

    _input->Poll();
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
