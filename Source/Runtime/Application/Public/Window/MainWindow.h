#pragma once

#include "Application.h"

#include "HAL/PlatformWindow.h"
#include "Memory/RefPtr.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FWindowDefinition = FPlatformWindow::FWindowDefinition;
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FMainWindow
:   public FPlatformWindow
,   public FRefCountable {
public:
    FMainWindow(FWString&& title, const FWindowDefinition& def);
    virtual ~FMainWindow();

    using FPlatformWindow::Title;
    using FPlatformWindow::NativeHandle;
    using FPlatformWindow::Left;
    using FPlatformWindow::Top;
    using FPlatformWindow::Width;
    using FPlatformWindow::Height;
    using FPlatformWindow::AllowDragDrop;
    using FPlatformWindow::HasCloseButton;
    using FPlatformWindow::HasResizeButton;
    using FPlatformWindow::HasSystemMenu;
    using FPlatformWindow::Fullscreen;
    using FPlatformWindow::HasFocus;
    using FPlatformWindow::Visible;
    using FPlatformWindow::Type;

    using FPlatformWindow::Show;
    using FPlatformWindow::Close;
    using FPlatformWindow::PumpMessages;
    using FPlatformWindow::Center;
    using FPlatformWindow::Maximize;
    using FPlatformWindow::Minimize;
    using FPlatformWindow::Move;
    using FPlatformWindow::Resize;
    using FPlatformWindow::SetFocus;
    using FPlatformWindow::SetFullscreen;
    using FPlatformWindow::SetTitle;

    using FPlatformWindow::ScreenToClient;
    using FPlatformWindow::ClientToScreen;
    using FPlatformWindow::SetCursorCapture;
    using FPlatformWindow::SetCursorOnWindowCenter;

    static FWindowDefinition Definition() NOEXCEPT;
    static FWindowDefinition Definition(size_t width, size_t height) NOEXCEPT;
    static FWindowDefinition Definition(int left, int top, size_t width, size_t height) NOEXCEPT;

protected:
    using FPlatformWindow::OnDragDropFile;
    using FPlatformWindow::OnFocusSet;
    using FPlatformWindow::OnFocusLose;
    using FPlatformWindow::OnMouseEnter;
    using FPlatformWindow::OnMouseLeave;
    using FPlatformWindow::OnMouseMove;
    using FPlatformWindow::OnMouseClick;
    using FPlatformWindow::OnMouseDoubleClick;
    using FPlatformWindow::OnMouseWheel;
    using FPlatformWindow::OnWindowShow;
    using FPlatformWindow::OnWindowClose;
    using FPlatformWindow::OnWindowMove;
    using FPlatformWindow::OnWindowResize;
    using FPlatformWindow::OnWindowPaint;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
