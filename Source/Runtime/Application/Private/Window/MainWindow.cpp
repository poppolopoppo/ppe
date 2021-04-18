#include "stdafx.h"

#include "Window/MainWindow.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMainWindow::FMainWindow(FWString&& title, const FWindowDefinition& def) {
    VerifyRelease(FPlatformWindow::CreateWindow(this, std::move(title), def));
}
//----------------------------------------------------------------------------
FMainWindow::~FMainWindow() = default;
//----------------------------------------------------------------------------
FWindowDefinition FMainWindow::Definition() NOEXCEPT {
    FWindowDefinition def;
    FPlatformWindow::MainWindowDefinition(&def);
    def.Maximized = true;
    return def;
}
//----------------------------------------------------------------------------
FWindowDefinition FMainWindow::Definition(size_t width, size_t height) NOEXCEPT {
    FWindowDefinition def;
    FPlatformWindow::MainWindowDefinition(&def);
    def.Width = width;
    def.Height = height;
    def.Centered = true;
    return def;
}
//----------------------------------------------------------------------------
FWindowDefinition FMainWindow::Definition(int left, int top, size_t width, size_t height) NOEXCEPT {
    FWindowDefinition def;
    FPlatformWindow::MainWindowDefinition(&def);
    def.Left = left;
    def.Top = top;
    def.Width = width;
    def.Height = height;
    return def;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
