﻿#include "stdafx.h"

#include "Window/WindowService.h"

#include "Window/MainWindow.h"

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
    FDefaultWindowService_() = default;
    virtual ~FDefaultWindowService_() = default;

    virtual void CreateMainWindow(PMainWindow* window, FWString&& title) override final;
    virtual void CreateMainWindow(PMainWindow* window, FWString&& title, size_t width, size_t height) override final;
    virtual void CreateMainWindow(PMainWindow* window, FWString&& title, int left, int top, size_t width, size_t height) override final;

    virtual FMainWindow* MainWindow() const NOEXCEPT override final;
    virtual void SetMainWindow(FMainWindow* window) override final;

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
};
//----------------------------------------------------------------------------
void FDefaultWindowService_::CreateMainWindow(PMainWindow* window, FWString&& title) {
    Assert(window);
    *window = NEW_REF(Window, FMainWindow, std::move(title), FMainWindow::Definition());
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::CreateMainWindow(PMainWindow* window, FWString&& title, size_t width, size_t height) {
    Assert(window);
    *window = NEW_REF(Window, FMainWindow, std::move(title), FMainWindow::Definition(width, height));
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::CreateMainWindow(PMainWindow* window, FWString&& title, int left, int top, size_t width, size_t height) {
    Assert(window);
    *window = NEW_REF(Window, FMainWindow, std::move(title), FMainWindow::Definition(left, top, width, height));
}
//----------------------------------------------------------------------------
FMainWindow* FDefaultWindowService_::MainWindow() const NOEXCEPT {
    return _mainWindow.get();
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::SetMainWindow(FMainWindow* window) {
    _mainWindow.reset(window);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::NotifySystrayNone(const FWStringView& title, const FWStringView& text) {
    FPlatformNotification::NotifySystray(FPlatformNotification::ENotificationIcon::None, title, text);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::NotifySystrayInfo(const FWStringView& title, const FWStringView& text) {
    FPlatformNotification::NotifySystray(FPlatformNotification::ENotificationIcon::Info, title, text);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::NotifySystrayWarning(const FWStringView& title, const FWStringView& text) {
    FPlatformNotification::NotifySystray(FPlatformNotification::ENotificationIcon::Warning, title, text);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::NotifySystrayError(const FWStringView& title, const FWStringView& text) {
    FPlatformNotification::NotifySystray(FPlatformNotification::ENotificationIcon::Error, title, text);
}
//----------------------------------------------------------------------------
size_t FDefaultWindowService_::AddSystrayCommand(
    const FWStringView& category,
    const FWStringView& label,
    FSystrayDelegate&& cmd ) {
    return FPlatformNotification::AddSystrayCommand(category, label, std::move(cmd));
}
//----------------------------------------------------------------------------
bool FDefaultWindowService_::RemoveSystrayCommand(size_t userCmd) {
    return FPlatformNotification::RemoveSystrayCommand(userCmd);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::SetTaskbarStateNormal() {
    FPlatformNotification::SetTaskbarState(FPlatformNotification::ETaskbarState::Normal);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::SetTaskbarStatePaused() {
    FPlatformNotification::SetTaskbarState(FPlatformNotification::ETaskbarState::Paused);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::SetTaskbarStateError() {
    FPlatformNotification::SetTaskbarState(FPlatformNotification::ETaskbarState::Error);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::SetTaskbarStateIndeterminate() {
    FPlatformNotification::SetTaskbarState(FPlatformNotification::ETaskbarState::Indeterminate);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::BeginTaskbarProgress() {
    FPlatformNotification::SetTaskbarState(FPlatformNotification::ETaskbarState::Progress);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::SetTaskbarProgress(size_t completed, size_t total) {
    FPlatformNotification::SetTaskbarProgress(completed, total);
}
//----------------------------------------------------------------------------
void FDefaultWindowService_::EndTaskbarProgress() {
    FPlatformNotification::SetTaskbarState(FPlatformNotification::ETaskbarState::NoProgress);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void IWindowService::MakeDefault(UWindowService* window) {
    Assert(window);
    window->reset<FDefaultWindowService_>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE