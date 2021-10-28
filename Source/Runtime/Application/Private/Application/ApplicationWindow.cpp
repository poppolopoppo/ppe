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
        const FRHIModule& rhiModule = FRHIModule::Get(Domain());

        ERHIFeature features = _targetRHI->RecommendedFeatures();
        features = rhiModule.RecommendedFeatures(features);

        FRHISurfaceCreateInfo surfaceInfo;
        surfaceInfo.Hwnd.Assign(_main->NativeHandle());
        surfaceInfo.Dimensions = _main->Dimensions();
        surfaceInfo.EnableFullscreen = _main->Fullscreen();
        surfaceInfo.EnableVSync = (features ^ ERHIFeature::VSync);

        if (not _targetRHI->CreateService(&_rhi, Domain(), &surfaceInfo, features) )
            LOG(Application, Fatal, L"failed to create RHI service in '{0}::{1}' abort!", Domain().Name(), Name());

#if USE_PPE_RHIDEBUG
        _rhi->UnitTest();
        _rhi->ReleaseMemory();
#endif

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
        Assert(&services.Get<IRHIService>() == _rhi.get());
        services.Remove<IRHIService>();

        _rhi->TearDown();
        _rhi.reset();
    }

    Assert(&services.Get<IWindowService>() == _window.get());
    Assert(&services.Get<IInputService>() == _input.get());

    services.Remove<IWindowService>();
    services.Remove<IInputService>();

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
