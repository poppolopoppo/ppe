#include "stdafx.h"

#include "Window/WindowBase.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWindowBase::FWindowBase(FWString&& title, const FWindowDefinition& def) {
    VerifyRelease(FPlatformWindow::CreateWindow(this, std::move(title), def));
}
//----------------------------------------------------------------------------
FWindowBase::~FWindowBase() = default;
//----------------------------------------------------------------------------
FWindowDefinition FWindowBase::MainWindowDefinition() NOEXCEPT {
    FWindowDefinition def;
    FPlatformWindow::MainWindowDefinition(&def);
    def.Maximized = true;
    return def;
}
//----------------------------------------------------------------------------
FWindowDefinition FWindowBase::MainWindowDefinition(size_t width, size_t height) NOEXCEPT {
    FWindowDefinition def;
    FPlatformWindow::MainWindowDefinition(&def);
    def.Width = width;
    def.Height = height;
    def.Centered = true;
    return def;
}
//----------------------------------------------------------------------------
FWindowDefinition FWindowBase::MainWindowDefinition(int left, int top, size_t width, size_t height) NOEXCEPT {
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
FWindowBare::FWindowBare(FWString&& title, const FWindowDefinition& def)
:   FWindowBase(std::move(title), def)
{}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
