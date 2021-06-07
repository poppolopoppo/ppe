#include "stdafx.h"

#include "HAL/Generic/GenericWindow.h"

#include "Maths/ScalarVector.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGenericWindow::FGenericWindow()
:   _handle(nullptr)
,   _left(0)
,   _top(0)
,   _width(0)
,   _height(0)
,   _allowDragDrop(false)
,   _hasCloseButton(true)
,   _hasResizeButton(true)
,   _hasSystemMenu(true)
,   _fullscreen(false)
,   _hasFocus(false)
,   _visible(false)
,   _type(EWindowType::Main)
{}
//----------------------------------------------------------------------------
FGenericWindow::~FGenericWindow() = default;
//----------------------------------------------------------------------------
uint2 FGenericWindow::Dimensions() const noexcept {
    return { _width, _height };
}
//----------------------------------------------------------------------------
bool FGenericWindow::Show() {
    Assert(not _visible);

    _visible = true;
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::Close() {
    Assert(_visible);

    _visible = false;
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::SetFocus() {
    if (not _visible)
        Show();

    _hasFocus = true;
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::PumpMessages() {
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::Center() {
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::Maximize() {
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::Minimize() {
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::Move(int x, int y) {
    _left = x;
    _top = y;
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::Resize(size_t w, size_t h) {
    Assert(w > 0);
    Assert(h > 0);

    _width = checked_cast<u32>(w);
    _height = checked_cast<u32>(h);
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::SetFullscreen(bool value) {
    Assert(_visible);

    _fullscreen = value;
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::SetTitle(FWString&& title) {
    Assert(not title.empty());

    _title.assign(std::move(title));
    return true;
}
//----------------------------------------------------------------------------
// stubs for derived classes :
void FGenericWindow::OnDragDropFile(const FWStringView& ) {}
void FGenericWindow::OnFocusSet() {
    Assert(not _hasFocus);
    _hasFocus = true;
}
void FGenericWindow::OnFocusLose() {
    Assert(_hasFocus);
    _hasFocus = false;
}
void FGenericWindow::OnMouseEnter() {}
void FGenericWindow::OnMouseLeave() {}
void FGenericWindow::OnMouseMove(int , int ) {}
void FGenericWindow::OnMouseClick(int , int , EMouseButton ) {}
void FGenericWindow::OnMouseDoubleClick(int , int , EMouseButton ) {}
void FGenericWindow::OnMouseWheel(int , int , int ) {}
void FGenericWindow::OnWindowShow(bool visible) {
    _visible = visible;
}
void FGenericWindow::OnWindowClose() {
    _visible = false;
}
void FGenericWindow::OnWindowMove(int x, int y) {
    _left = x;
    _top = y;
}
void FGenericWindow::OnWindowResize(size_t w, size_t h) {
    _width = w;
    _height = h;
}
void FGenericWindow::OnWindowPaint() {}
//----------------------------------------------------------------------------
void FGenericWindow::MainWindowDefinition(FWindowDefinition* def) {
    Assert(def);

    def->Type = EGenericWindowType::Main;

    def->Left = 0; // overridden by Centered
    def->Top = 0;

    def->Width = 0; // overridden by Maximized
    def->Height = 0;

    def->AutoSize = true;
    def->AllowDragDrop = true;
    def->Centered = true;
    def->Maximized = true;
    def->Fullscreen = false;
    def->Invisible = false;
    def->HasCloseButton = true;
    def->HasResizeButton = false;
    def->HasSystemMenu = false;

    def->Parent = nullptr;
}
//----------------------------------------------------------------------------
void FGenericWindow::HiddenWindowDefinition(FWindowDefinition* def) {
    Assert(def);

    def->Type = EGenericWindowType::BorderLess;

    def->Left = 0; // overridden by Centered
    def->Top = 0;

    def->Width = 0; // overridden by Maximized
    def->Height = 0;

    def->AutoSize = false;
    def->AllowDragDrop = false;
    def->Centered = false;
    def->Maximized = false;
    def->Fullscreen = false;
    def->Invisible = true;
    def->HasCloseButton = false;
    def->HasResizeButton = false;
    def->HasSystemMenu = false;

    def->Parent = nullptr;
}
//----------------------------------------------------------------------------
bool FGenericWindow::CreateWindow(FGenericWindow* window, FWString&& title, const FWindowDefinition& def) {
    Assert(window);
    Assert(not window->NativeHandle());
    Assert(not title.empty());

    window->_title = std::move(title);
    window->_handle = nullptr;
    window->_left = def.Left;
    window->_top = def.Top;
    window->_width = def.Width;
    window->_height = def.Height;
    window->_allowDragDrop = def.AllowDragDrop;
    window->_hasCloseButton = def.HasCloseButton;
    window->_hasResizeButton = def.HasResizeButton;
    window->_hasSystemMenu = def.HasSystemMenu;
    window->_fullscreen = def.Fullscreen;
    window->_hasFocus = false;
    window->_visible = false;
    window->_type = def.Type;

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
