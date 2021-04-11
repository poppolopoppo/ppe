#include "stdafx.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxWindow.h"

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformIncludes.h"
#include "HAL/PlatformMessageHandler.h"
#include "HAL/PlatformMouse.h"
#include "HAL/PlatformNotification.h"
#include "Input/MouseButton.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLinuxWindow::FLinuxWindow()
{}
//----------------------------------------------------------------------------
FLinuxWindow::~FLinuxWindow() 
{}
//----------------------------------------------------------------------------
bool FLinuxWindow::Show() {
    return FGenericWindow::Show();
}
//----------------------------------------------------------------------------
bool FLinuxWindow::Close() {
    return FGenericWindow::Close();
}
//----------------------------------------------------------------------------
bool FLinuxWindow::PumpMessages() {
    return (FGenericWindow::PumpMessages() &&
        FPlatformMessageHandler::PumpMessages(this));
}
//----------------------------------------------------------------------------
bool FLinuxWindow::Center() {
    return FGenericWindow::Center();
}
//----------------------------------------------------------------------------
bool FLinuxWindow::Maximize() {
    return FGenericWindow::Maximize();
}
//----------------------------------------------------------------------------
bool FLinuxWindow::Minimize() {
    return FGenericWindow::Minimize();
}
//----------------------------------------------------------------------------
bool FLinuxWindow::Move(int x, int y) {
    return FGenericWindow::Move(x, y);
}
//----------------------------------------------------------------------------
bool FLinuxWindow::Resize(size_t w, size_t h) {
    return FGenericWindow::Resize(w, h);
}
//----------------------------------------------------------------------------
bool FLinuxWindow::SetFocus() {
    return FGenericWindow::SetFocus();
}
//----------------------------------------------------------------------------
bool FLinuxWindow::SetFullscreen(bool value) {
    return FGenericWindow::SetFullscreen(value);
}
//----------------------------------------------------------------------------
bool FLinuxWindow::SetTitle(FWString&& title) {
    return FGenericWindow::SetTitle(std::move(title));
}
//----------------------------------------------------------------------------
void FLinuxWindow::ScreenToClient(int* screenX, int* screenY) const {
    Verify(FPlatformMouse::ScreenToClient(*this, screenX, screenY));
}
//----------------------------------------------------------------------------
void FLinuxWindow::ClientToScreen(int* clientX, int* clientY) const {
    Verify(FPlatformMouse::ClientToScreen(*this, clientX, clientY));
}
//----------------------------------------------------------------------------
void FLinuxWindow::SetCursorCapture(bool enabled) const {
    if (enabled)
        FPlatformMouse::SetCapture(*this);
    else
        FPlatformMouse::ResetCapture();
}
//----------------------------------------------------------------------------
void FLinuxWindow::SetCursorOnWindowCenter() const {
    FPlatformMouse::CenterCursorOnWindow(*this);
}
//----------------------------------------------------------------------------
FLinuxWindow* FLinuxWindow::ActiveWindow() {
    return nullptr;
}
//----------------------------------------------------------------------------
void FLinuxWindow::MainWindowDefinition(FWindowDefinition* def) {
    FGenericWindow::MainWindowDefinition(def);
}
//----------------------------------------------------------------------------
void FLinuxWindow::HiddenWindowDefinition(FWindowDefinition* def) {
    FGenericWindow::HiddenWindowDefinition(def);
}
//----------------------------------------------------------------------------
bool FLinuxWindow::CreateWindow(FLinuxWindow* window, FWString&& title, const FWindowDefinition& def) {
    UNUSED(window);
    UNUSED(title);
    UNUSED(def);
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_LINUX
