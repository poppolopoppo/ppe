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
static TPtrRef<const ITargetRHI> RetrieveTargetRHI_(const FModularDomain& domain, bool needRHI) {
    if (needRHI) {
        const TPtrRef<const ITargetRHI> rhiTarget = FRHIModule::Get(domain).Target();
        CLOG(nullptr == rhiTarget, Application, Error, L"could not find any RHI target available");
        return rhiTarget;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
static void RecommendedDeviceInfo_(
    FRHIDeviceCreateInfo* outDeviceInfo,
    const FModularDomain& domain,
    const ITargetRHI& targetRHI ) NOEXCEPT {
    const FRHIModule& rhiModule = FRHIModule::Get(domain);

    outDeviceInfo->Features = targetRHI.RecommendedFeatures();
    outDeviceInfo->Features = rhiModule.RecommendedFeatures(outDeviceInfo->Features);
    outDeviceInfo->MaxStagingBufferMemory = rhiModule.MaxStagingBufferMemory();
    outDeviceInfo->StagingBufferSize = rhiModule.StagingBufferSize();
}
//----------------------------------------------------------------------------
static void RecommendedSurfaceInfo_(
    FRHISurfaceCreateInfo* outSurfaceInfo,
    ERHIFeature deviceFeatures,
    const FMainWindow& window ) NOEXCEPT {

    outSurfaceInfo->Hwnd.Assign(window.NativeHandle());
    outSurfaceInfo->Dimensions = window.Dimensions();
    outSurfaceInfo->EnableFullscreen = window.Fullscreen();
    outSurfaceInfo->EnableVSync = (deviceFeatures & ERHIFeature::VSync);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationWindow::FApplicationWindow(const FModularDomain& domain, FString&& name, bool needRHI)
    : FApplicationBase(domain, std::move(name))
      , _targetRHI(RetrieveTargetRHI_(domain, needRHI)) {}

//----------------------------------------------------------------------------
FApplicationWindow::~FApplicationWindow() = default;
//----------------------------------------------------------------------------
void FApplicationWindow::Start() {
    IInputService::MakeDefault(&_input);
    IWindowService::MakeDefault(&_window);

    _window->CreateMainWindow(&_main, ToWString(Name()));
    _window->SetMainWindow(_main.get());

    _main->AddListener(*this);
    _input->SetupWindow(*_main);

    FModularServices& services = Services();
    services.Add<IInputService>(_input.get());
    services.Add<IWindowService>(_window.get());

    if (_targetRHI) {
        FRHIDeviceCreateInfo deviceInfo;
        RecommendedDeviceInfo_(&deviceInfo, Domain(), *_targetRHI);

        FRHISurfaceCreateInfo surfaceInfo;
        RecommendedSurfaceInfo_(&surfaceInfo, deviceInfo.Features, *_main);

        if (not _targetRHI->CreateService(&_rhi, Domain(), deviceInfo, &surfaceInfo))
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

    _main->RemoveListener(*this);
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
        _main->PumpMessages()) {
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
void FApplicationWindow::OnWindowFocus(bool enabled) {
    SetFocus(enabled);
}
//----------------------------------------------------------------------------
void FApplicationWindow::OnWindowPaint() {

}
//----------------------------------------------------------------------------
void FApplicationWindow::OnWindowResize(const uint2& size) {
    FRHISurfaceCreateInfo surfaceInfo;
    RecommendedSurfaceInfo_(&surfaceInfo, _rhi->Features(), *_main);

    UNUSED(size);
    Assert_NoAssume(size == surfaceInfo.Dimensions);

    _rhi->ResizeWindow(surfaceInfo);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
