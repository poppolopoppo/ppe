#include "stdafx.h"

#include "Application/ApplicationWindow.h"

#include "Input/InputService.h"
#include "Window/WindowService.h"
#include "Window/MainWindow.h"

#include "RHIModule.h"
#include "HAL/RHIService.h"

#include "Diagnostic/Logger.h"
#include "Thread/ThreadPool.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATION_API, Application)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
TPtrRef<const ITargetRHI> RetrieveTargetRHI_(const FModularDomain& domain, bool needRHI) {
    if (needRHI) {
        const TPtrRef<const ITargetRHI> rhiTarget = FRHIModule::Get(domain).Target();
        CLOG(nullptr == rhiTarget, Application, Error, L"could not find any RHI target available");
        return rhiTarget;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationWindow::FApplicationWindow(const FModularDomain& domain, FString&& name, bool needRHI)
:   FApplicationBase(domain, std::move(name))
,   _targetRHI(RetrieveTargetRHI_(domain, needRHI))
{}
//----------------------------------------------------------------------------
FApplicationWindow::~FApplicationWindow() = default;
//----------------------------------------------------------------------------
void FApplicationWindow::Start() {
    IInputService::MakeDefault(&_input);
    IWindowService::MakeDefault(&_window);

    _window->CreateMainWindow(&_main, ToWString(Name()) );
    _window->SetMainWindow(_main.get());

    _input->SetupWindow(*_main);

    FModularServices& services = Services();
    services.Add<IInputService>(_input.get());
    services.Add<IWindowService>(_window.get());

    if (_targetRHI) {
        ERHIFeature features = _targetRHI->RecommendedFeatures();

        const FRHIModule& rhiModule = FRHIModule::Get(Domain());
        features = rhiModule.RecommendedFeatures(features);

        if (not _targetRHI->CreateService(
            &_rhi,
            Domain(),
            RHI::FWindowHandle{ _main->NativeHandle() },
            features ))
            LOG(Application, Fatal, L"failed to create RHI service in '{0}::{1}' abort", Domain().Name(), Name());

        services.Add<IRHIService>(_rhi.get());
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

    _window->SetMainWindow(nullptr);

    auto& services = Services();
    if (_rhi) {
        services.CheckedRemove<IRHIService>(_rhi.get());
        _rhi.reset();
    }

    services.CheckedRemove<IWindowService>(_window.get());
    services.CheckedRemove<IInputService>(_input.get());

    _input.reset();
    _window.reset();
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
