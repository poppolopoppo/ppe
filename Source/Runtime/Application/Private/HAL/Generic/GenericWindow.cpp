#include "stdafx.h"

#include "HAL/Generic/GenericWindow.h"

#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformMouse.h"
#include "HAL/PlatformWindow.h"

#include "Maths/ScalarVector.h"
#include "Window/WindowListener.h"

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
,   _dpi(FPlatformApplicationMisc::DefaultScreenDPI)
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
FGenericWindow::~FGenericWindow() {
    Assert_NoAssume(nullptr == _handle);
}
//----------------------------------------------------------------------------
uint2 FGenericWindow::Dimensions() const noexcept {
    PPE_DATARACE_SHARED_SCOPE(this);
    return { _width, _height };
}
//----------------------------------------------------------------------------
bool FGenericWindow::Show() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    Assert(not _visible);

    _visible = true;
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::Close() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    Assert(_visible);

    _visible = false;
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::SetFocus() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    if (not _visible)
        Show();

    _hasFocus = true;
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::PumpMessages() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::Center() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::Maximize() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::Minimize() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::Move(int x, int y) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    _left = x;
    _top = y;
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::Resize(size_t w, size_t h) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    Assert(w > 0);
    Assert(h > 0);

    _width = checked_cast<u32>(w);
    _height = checked_cast<u32>(h);
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::SetFullscreen(bool value) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    Assert(_visible);

    _fullscreen = value;
    return true;
}
//----------------------------------------------------------------------------
bool FGenericWindow::SetTitle(FWString&& title) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    Assert(not title.empty());

    _title.assign(std::move(title));
    return true;
}
//----------------------------------------------------------------------------
// stubs for derived classes :
void FGenericWindow::OnDragDropFile(const FWStringView& ) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
}
void FGenericWindow::OnFocusSet() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    Assert(not _hasFocus);
    _hasFocus = true;

    for (const TPtrRef<IWindowListener>& listener : *_listeners.LockShared())
        listener->OnWindowFocus(_hasFocus);
}
void FGenericWindow::OnFocusLose() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    Assert(_hasFocus);
    _hasFocus = false;

    for (const TPtrRef<IWindowListener>& listener : *_listeners.LockShared())
        listener->OnWindowFocus(_hasFocus);
}
void FGenericWindow::OnMouseEnter() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
}
void FGenericWindow::OnMouseLeave() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
}
void FGenericWindow::OnMouseMove(int , int ) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
}
void FGenericWindow::OnMouseClick(int , int , EMouseButton ) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
}
void FGenericWindow::OnMouseDoubleClick(int , int , EMouseButton ) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
}
void FGenericWindow::OnMouseWheel(int , int , int ) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
}
void FGenericWindow::OnWindowShow(bool visible) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    _visible = visible;
}
void FGenericWindow::OnWindowClose() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    _visible = false;
}
void FGenericWindow::OnWindowDPI(u32 dpi) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    _dpi = dpi;

    for (const TPtrRef<IWindowListener>& listener : *_listeners.LockShared())
        listener->OnWindowDPI(_dpi);
}
void FGenericWindow::OnWindowMove(int x, int y) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    _left = x;
    _top = y;

    for (const TPtrRef<IWindowListener>& listener : *_listeners.LockShared())
        listener->OnWindowMove({ _left, _top });
}
void FGenericWindow::OnWindowResize(size_t w, size_t h) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    _width =  checked_cast<u32>(w);
    _height = checked_cast<u32>(h);

    for (const TPtrRef<IWindowListener>& listener : *_listeners.LockShared())
        listener->OnWindowResize({ _width, _height });
}
void FGenericWindow::OnWindowPaint() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    for (const TPtrRef<IWindowListener>& listener : *_listeners.LockShared())
        listener->OnWindowPaint();
}
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
void FGenericWindow::ScreenToClient(int* screenX, int* screenY) const {
    PPE_DATARACE_SHARED_SCOPE(this);
    Verify(FPlatformMouse::ScreenToClient(*checked_cast<const FPlatformWindow*>(this), screenX, screenY));
}
//----------------------------------------------------------------------------
void FGenericWindow::ClientToScreen(int* clientX, int* clientY) const {
    PPE_DATARACE_SHARED_SCOPE(this);
    Verify(FPlatformMouse::ClientToScreen(*checked_cast<const FPlatformWindow*>(this), clientX, clientY));
}

ECursorType FGenericWindow::SetCursorType(ECursorType value) {
    const ECursorType previous = _cursor;
    if (previous != value) {
        _cursor = value;
        FPlatformMouse::SetWindowCursor(*checked_cast<const FPlatformWindow*>(this));
    }
    return previous;
}
//----------------------------------------------------------------------------
void FGenericWindow::SetCursorCapture(bool enabled) const {
    PPE_DATARACE_SHARED_SCOPE(this);
    if (enabled)
        FPlatformMouse::SetCapture(*checked_cast<const FPlatformWindow*>(this));
    else
        FPlatformMouse::ResetCapture();
}
//----------------------------------------------------------------------------
void FGenericWindow::SetCursorOnWindowCenter() const {
    PPE_DATARACE_SHARED_SCOPE(this);
    FPlatformMouse::CenterCursorOnWindow(*checked_cast<const FPlatformWindow*>(this));
}
//----------------------------------------------------------------------------
bool FGenericWindow::CreateWindow(FGenericWindow* window, FWString&& title, const FWindowDefinition& def) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(window);
    Assert(window);
    Assert(not window->NativeHandle());
    Assert(not title.empty());

    window->_title = std::move(title);
    window->_handle = nullptr;
    window->_left = def.Left;
    window->_top = def.Top;
    window->_width = checked_cast<u32>(def.Width);
    window->_height = checked_cast<u32>(def.Height);
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
void FGenericWindow::AddListener(TPtrRef<IWindowListener>&& listener) {
    Assert(listener);
    Add_Unique(*_listeners.LockExclusive(), std::move(listener));
}
//----------------------------------------------------------------------------
void FGenericWindow::RemoveListener(const TPtrRef<IWindowListener>& listener) {
    Assert(listener);
    Remove_AssertExists(*_listeners.LockExclusive(), listener);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
