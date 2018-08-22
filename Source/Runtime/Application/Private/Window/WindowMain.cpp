#include "stdafx.h"

#include "Window/WindowMain.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FWindowDefinition MainWindowDef_() {
    FWindowDefinition def;
    FPlatformWindow::MainWindowDefinition(&def);
    def.Maximized = true;
    return def;
}
//----------------------------------------------------------------------------
static FWindowDefinition MainWindowDef_(size_t width, size_t height) {
    FWindowDefinition def;
    FPlatformWindow::MainWindowDefinition(&def);
    def.Width = width;
    def.Height = height;
    def.Centered = true;
    return def;
}
//----------------------------------------------------------------------------
static FWindowDefinition MainWindowDef_(int left, int top, size_t width, size_t height) {
    FWindowDefinition def;
    FPlatformWindow::MainWindowDefinition(&def);
    def.Left = left;
    def.Top = top;
    def.Width = width;
    def.Height = height;
    return def;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWindowMain::FWindowMain(FWString&& title)
:   FWindowBase(std::move(title), MainWindowDef_())
{}
//----------------------------------------------------------------------------
FWindowMain::FWindowMain(FWString&& title, size_t width, size_t height)
:   FWindowBase(std::move(title), MainWindowDef_(width, height))
{}
//----------------------------------------------------------------------------
FWindowMain::FWindowMain(FWString&& title, int left, int top, size_t width, size_t height)
:   FWindowBase(std::move(title), MainWindowDef_(left, top, width, height))
{}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
