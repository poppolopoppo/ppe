﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Application/ApplicationWindow.h"

#include "Input/InputService.h"
#include "UI/UIService.h"
#include "Window/WindowService.h"
#include "Window/MainWindow.h"

#include "RHIModule.h"
#include "RHI/FrameGraph.h"
#include "HAL/RHIService.h"
#include "HAL/TargetRHI.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformDialog.h"
#include "HAL/PlatformSurvey.h"
#include "Thread/ThreadPool.h"

#if USE_PPE_LOGGER
#   include "IO/Format.h"
#   include "IO/FormatHelpers.h"
#endif

#if USE_PPE_RHIDEBUG
#   include "Diagnostic/CurrentProcess.h"
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
        PPE_CLOG(nullptr == rhiTarget, Application, Error, "could not find any RHI target available");
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

    PPE_SLOG(Application, Info, "create RHI service with device info", {
        {"Features", Opaq::Format(outDeviceInfo->Features)},
        {"MaxStagingBufferMemory", Opaq::Format(Fmt::SizeInBytes(outDeviceInfo->MaxStagingBufferMemory))},
        {"StagingBufferSize", Opaq::Format(Fmt::SizeInBytes(outDeviceInfo->StagingBufferSize))},
    });
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

    PPE_SLOG(Application, Info, "create RHI framebuffer with surface info", {
        {"Handle", Opaq::Format(Fmt::Pointer(outSurfaceInfo->Hwnd.Value))},
        {"Width", outSurfaceInfo->Dimensions.x},
        {"Height", outSurfaceInfo->Dimensions.y},
        {"EnableFullscreen", outSurfaceInfo->EnableFullscreen},
        {"EnableVSync", outSurfaceInfo->EnableVSync},
    });
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationWindow::FApplicationWindow(FModularDomain& domain, FString&& name, bool needRHI)
:   FApplicationBase(domain, std::move(name))
,   _targetRHI(RetrieveTargetRHI_(domain, needRHI)) {

    // open splash screen window
    FPlatformDialog::PushSplashScreen_ReturnIfOpened();
}
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

    FModularServices& services = Services_();
    services.Add<IInputService>(_input.get());
    services.Add<IWindowService>(_window.get());

    if (_targetRHI) {
        FRHIDeviceCreateInfo deviceInfo;
        RecommendedDeviceInfo_(&deviceInfo, Domain(), *_targetRHI);

        FRHISurfaceCreateInfo surfaceInfo;
        RecommendedSurfaceInfo_(&surfaceInfo, deviceInfo.Features, *_main);

        if (not _targetRHI->CreateService(&_rhi, Domain(), deviceInfo, &surfaceInfo))
            PPE_LOG(Application, Fatal, "failed to create RHI service in '{0}::{1}' abort!", Domain().Name(), Name());

#if USE_PPE_RHIDEBUG
        const auto& process = FCurrentProcess::Get();
        if (not process.HasArgument(L"-NoUnitTest")) {
            _rhi->UnitTest();
            _rhi->ReleaseMemory();
        }
#endif

        services.Add<IRHIService>(_rhi.get());

        _onViewportResized = _rhi->OnWindowResized().Add([this](const IRHIService& , const FRHISurfaceCreateInfo& surface) {
            ViewportResized(surface);
        });
    }

    if (_rhi->Features() & ERHIFeature::HighDPIAwareness)
        FPlatformApplicationMisc::SetHighDPIAwareness();

    VerifyRelease(_main->Show());
    VerifyRelease(_main->SetFocus());

    UpdateTickRateFromRefreshRate_();

    FApplicationBase::Start();
}
//----------------------------------------------------------------------------
void FApplicationWindow::Run() {
    FApplicationBase::Run();

    // close splash screen window
    FPlatformDialog::PopSplashScreen_ReturnIfOpened();

    // bring focus to main window
    _main->BringToFront();
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

    FModularServices& services = Services_();
    if (_rhi) {
        _rhi->OnWindowResized().Remove(_onViewportResized);

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
    const FMovingAverageTimer::FScope timedScope{_messageTime};
    Unused(timedScope);

    if (FApplicationBase::PumpMessages() &&
        _main->PumpMessages()) {

        _input->PollInputEvents();

        if (_windowWasResized) {
            _windowWasResized = false;

            FRHISurfaceCreateInfo surfaceInfo;
            RecommendedSurfaceInfo_(&surfaceInfo, _rhi->Features(), *_main);

            _rhi->ResizeWindow(surfaceInfo);
        }

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
void FApplicationWindow::Tick(FTimespan dt) {
    const FMovingAverageTimer::FScope timedScope{_tickTime};
    Unused(timedScope);

    _input->PostInputMessages(ApplicationTime());

    Update(dt);

    FApplicationBase::Tick(dt);

    if (not _main->Visible())
        return;

    if (_rhi->BeginFrame(dt)) {
        Render(*_rhi->FrameGraph(), dt);

        _rhi->EndFrame();
    }
}
//----------------------------------------------------------------------------
void FApplicationWindow::Update(FTimespan dt) {
    Unused(dt);
}
//----------------------------------------------------------------------------
void FApplicationWindow::Render(RHI::IFrameGraph& fg, FTimespan dt) {
    Unused(fg, dt);
}
//----------------------------------------------------------------------------
void FApplicationWindow::ViewportResized(const FRHISurfaceCreateInfo& surface) {
    Unused(surface);
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
void FApplicationWindow::OnWindowShow(bool visible) NOEXCEPT {
    PPE_LOG(Application, Info, "application {0} main window show({1:a})", Name(), visible);
}
//----------------------------------------------------------------------------
void FApplicationWindow::OnWindowClose() NOEXCEPT {
    PPE_LOG(Application, Info, "application {0} main window was closed", Name());
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
