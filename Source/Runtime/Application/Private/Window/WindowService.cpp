// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Window/WindowService.h"

#include "Viewport/ViewportClient.h"
#include "Window/MainWindow.h"

#include "Diagnostic/CurrentProcess.h"
#include "HAL/PlatformWindow.h"
#include "HAL/PlatformNotification.h"
#include "Misc/Function.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FDefaultWindowService_ final : public IWindowService {
public:
    FDefaultWindowService_() NOEXCEPT;
    ~FDefaultWindowService_() override = default;

    virtual PMainWindow CreateMainWindow(FWString&& title) override final;
    virtual PMainWindow CreateMainWindow(FWString&& title, size_t width, size_t height) override final;
    virtual PMainWindow CreateMainWindow(FWString&& title, int left, int top, size_t width, size_t height) override final;

    virtual SMainWindow MainWindow() const NOEXCEPT override final;
    virtual void SetMainWindow(const SMainWindow& window) override final;

    virtual SViewportClient MainViewport() const NOEXCEPT override final;
    virtual void SetMainViewport(const SViewportClient& viewport) override final;

    virtual void ShowSystray() override final;
    virtual void HideSystray() override final;

    virtual void NotifySystrayNone(const FWStringView& title, const FWStringView& text) override final;
    virtual void NotifySystrayInfo(const FWStringView& title, const FWStringView& text) override final;
    virtual void NotifySystrayWarning(const FWStringView& title, const FWStringView& text) override final;
    virtual void NotifySystrayError(const FWStringView& title, const FWStringView& text) override final;

    virtual size_t AddSystrayCommand(
        const FWStringView& category,
        const FWStringView& label,
        FSystrayDelegate&& cmd ) override final;
    virtual bool RemoveSystrayCommand(size_t userCmd) override final;

    virtual void SetTaskbarStateNormal() override final;
    virtual void SetTaskbarStatePaused() override final;
    virtual void SetTaskbarStateError() override final;
    virtual void SetTaskbarStateIndeterminate() override final;

    virtual void BeginTaskbarProgress() override final;
    virtual void SetTaskbarProgress(size_t completed, size_t total) override final;
    virtual void EndTaskbarProgress() override final;

private:
    SMainWindow _mainWindow;
    SViewportClient _mainViewport;
    bool _systrayAvailable{ true };
};
//----------------------------------------------------------------------------
FDefaultWindowService_::FDefaultWindowService_() NOEXCEPT {
#if !USE_PPE_FINAL_RELEASE
    // disable systray when debugger is attached for non-final builds
    _systrayAvailable &= (not FCurrentProcess::Get().StartedWithDebugger());
#endif
}
//----------------------------------------------------------------------------
PMainWindow FDefaultWindowService_::CreateMainWindow(FWString&& title) {
    return NEW_REF(Window, FMainWindow, std::move(title), FMainWindow::Definition());
}
//----------------------------------------------------------------------------
PMainWindow FDefaultWindowService_::CreateMainWindow(FWString&& title, size_t width, size_t height) {
    return NEW_REF(Window, FMainWindow, std::move(title), FMainWindow::Definition(width, height));
}
//----------------------------------------------------------------------------
PMainWindow FDefaultWindowService_::CreateMainWindow(FWString&& title, int left, int top, size_t width, size_t height) {
    return NEW_REF(Window, FMainWindow, std::move(title), FMainWindow::Definition(left, top, width, height));
}
//----------------------------------------------------------------------------
SMainWindow FDefaultWindowService_::MainWindow() const NOEXCEPT {
    return _mainWindow;
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::SetMainWindow(const SMainWindow& window) {
    _mainWindow.reset(window);
}
//----------------------------------------------------------------------------
SViewportClient FDefaultWindowService_::MainViewport() const NOEXCEPT {
    return _mainViewport;
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::SetMainViewport(const SViewportClient& viewport) {
    _mainViewport.reset(viewport);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::ShowSystray() {
    if (_systrayAvailable)
        FPlatformNotification::ShowSystray();
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::HideSystray() {
    if (_systrayAvailable)
        FPlatformNotification::HideSystray();
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::NotifySystrayNone(const FWStringView& title, const FWStringView& text) {
    if (_systrayAvailable)
        FPlatformNotification::NotifySystray(FPlatformNotification::ENotificationIcon::None, title, text);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::NotifySystrayInfo(const FWStringView& title, const FWStringView& text) {
    if (_systrayAvailable)
        FPlatformNotification::NotifySystray(FPlatformNotification::ENotificationIcon::Info, title, text);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::NotifySystrayWarning(const FWStringView& title, const FWStringView& text) {
    if (_systrayAvailable)
        FPlatformNotification::NotifySystray(FPlatformNotification::ENotificationIcon::Warning, title, text);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::NotifySystrayError(const FWStringView& title, const FWStringView& text) {
    if (_systrayAvailable)
        FPlatformNotification::NotifySystray(FPlatformNotification::ENotificationIcon::Error, title, text);
}
//----------------------------------------------------------------------------
size_t FDefaultWindowService_::AddSystrayCommand(
    const FWStringView& category,
    const FWStringView& label,
    FSystrayDelegate&& cmd ) {
    if (_systrayAvailable)
        return FPlatformNotification::AddSystrayCommand(category, label, std::move(cmd));
    return INDEX_NONE;
}
//----------------------------------------------------------------------------
bool FDefaultWindowService_::RemoveSystrayCommand(size_t userCmd) {
    if (_systrayAvailable)
        return FPlatformNotification::RemoveSystrayCommand(userCmd);
    return true;
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::SetTaskbarStateNormal() {
    if (_systrayAvailable)
        FPlatformNotification::SetTaskbarState(FPlatformNotification::ETaskbarState::Normal);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::SetTaskbarStatePaused() {
    if (_systrayAvailable)
        FPlatformNotification::SetTaskbarState(FPlatformNotification::ETaskbarState::Paused);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::SetTaskbarStateError() {
    if (_systrayAvailable)
        FPlatformNotification::SetTaskbarState(FPlatformNotification::ETaskbarState::Error);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::SetTaskbarStateIndeterminate() {
    if (_systrayAvailable)
        FPlatformNotification::SetTaskbarState(FPlatformNotification::ETaskbarState::Indeterminate);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::BeginTaskbarProgress() {
    if (_systrayAvailable)
        FPlatformNotification::SetTaskbarState(FPlatformNotification::ETaskbarState::Progress);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::SetTaskbarProgress(size_t completed, size_t total) {
    if (_systrayAvailable)
        FPlatformNotification::SetTaskbarProgress(completed, total);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::EndTaskbarProgress() {
    if (_systrayAvailable)
        FPlatformNotification::SetTaskbarState(FPlatformNotification::ETaskbarState::NoProgress);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
//----------------------------------------------------------------------------
void IWindowService::MakeDefault(UWindowService* window) {
    Assert(window);
    window->create<Application::FDefaultWindowService_>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
