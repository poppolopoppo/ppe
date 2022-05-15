#include "stdafx.h"

#include "Application/ApplicationWindow.h"

#include "Input/InputService.h"
#include "UI/UIService.h"
#include "Window/WindowService.h"
#include "Window/MainWindow.h"

#include "RHIModule.h"
#include "HAL/RHIService.h"
#include "HAL/TargetRHI.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformSurvey.h"
#include "Thread/ThreadPool.h"

#if USE_PPE_LOGGER
#   include "IO/Format.h"
#   include "IO/FormatHelpers.h"
#endif

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

    LOG(Application, Info, L"create RHI service with device info:\n"
        L"\tFeatures                : {0}\n"
        L"\tMaxStagingBufferMemory  : {1}\n"
        L"\tStagingBufferSize       : {2}",
        outDeviceInfo->Features,
        Fmt::SizeInBytes(outDeviceInfo->MaxStagingBufferMemory),
        Fmt::SizeInBytes(outDeviceInfo->StagingBufferSize) );
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

    LOG(Application, Info, L"create RHI framebuffer with surface info:\n"
        L"\tHandle                  : {0}\n"
        L"\tDimensions              : {1}\n"
        L"\tEnableFullscreen        : {2:a}\n"
        L"\tEnableVSync             : {3:a}",
        Fmt::Pointer(outSurfaceInfo->Hwnd.Value),
        outSurfaceInfo->Dimensions,
        outSurfaceInfo->EnableFullscreen,
        outSurfaceInfo->EnableVSync );
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationWindow::FApplicationWindow(FModularDomain& domain, FString&& name, bool needRHI)
:   FApplicationBase(domain, std::move(name))
,   _targetRHI(RetrieveTargetRHI_(domain, needRHI)) {}

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

    if (_rhi->Features() & ERHIFeature::HighDPIAwareness)
        FPlatformApplicationMisc::SetHighDPIAwareness();

    VerifyRelease(_main->Show());
    VerifyRelease(_main->SetFocus());

    UpdateTickRateFromRefreshRate_();

    FApplicationBase::Start();
}
//----------------------------------------------------------------------------
void FApplicationWindow::Shutdown() {
    FApplicationBase::Shutdown();

    if (_input->FocusedWindow() == _main.get())
        _input->SetWindowFocused(nullptr);

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

    if (_windowWasResized) {
        _windowWasResized = false;

        FRHISurfaceCreateInfo surfaceInfo;
        RecommendedSurfaceInfo_(&surfaceInfo, _rhi->Features(), *_main);

        _rhi->ResizeWindow(surfaceInfo);
    }

    _input->Update(dt);

    if (_main->Visible())
        _rhi->RenderFrame(dt);
}
//----------------------------------------------------------------------------
void FApplicationWindow::OnWindowFocus(bool enabled) NOEXCEPT {
    SetFocus(enabled);

    if (enabled)
        _input->SetWindowFocused(_main.get());
    else
        _input->SetWindowFocused(nullptr);
}
//----------------------------------------------------------------------------
void FApplicationWindow::OnWindowMove(const int2& pos) NOEXCEPT {
    Unused(pos);

    UpdateTickRateFromRefreshRate_();
}
//----------------------------------------------------------------------------
void FApplicationWindow::OnWindowResize(const uint2& size) NOEXCEPT {
    Unused(size);

    _windowWasResized = true;

    UpdateTickRateFromRefreshRate_();
}
//----------------------------------------------------------------------------
void FApplicationWindow::UpdateTickRateFromRefreshRate_() {
    FPlatformSurvey::FMonitorInfo primaryMonitor;
    if (_main && FPlatformSurvey::MonitorFromWindow(*_main, &primaryMonitor))
        SetTickRate(primaryMonitor.CurrentResolution.RefreshRate);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
