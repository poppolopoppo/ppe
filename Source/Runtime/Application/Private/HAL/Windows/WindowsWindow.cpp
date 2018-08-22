#include "stdafx.h"

#include "HAL/Windows/WindowsWindow.h"

#ifdef PLATFORM_WINDOWS

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformIncludes.h"
#include "HAL/PlatformMessageHandler.h"
#include "HAL/PlatformMouse.h"
#include "HAL/Windows/LastError.h"
#include "Input/MouseButton.h"

#include <shellapi.h>
#include <windowsx.h>

namespace PPE {
namespace Application {
LOG_CATEGORY(, Window)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static ::HINSTANCE AppHandleWin32_() {
    return reinterpret_cast<::HINSTANCE>(FCurrentProcess::Get().ApplicationHandle());
}
//----------------------------------------------------------------------------
static const wchar_t GAppWindowClassName_[] = L"PPE::MainWindow";
//----------------------------------------------------------------------------
static void WindowsScreenSize_(::HWND hWnd, ::LONG* screenW, ::LONG* screenH) {
    ::MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof(monitor_info);

    ::GetMonitorInfo(
        ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST),
        &monitor_info);

    *screenW = checked_cast<::LONG>(monitor_info.rcMonitor.right - monitor_info.rcMonitor.left);
    *screenH = checked_cast<::LONG>(monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top);

    Assert(*screenW > 0);
    Assert(*screenH > 0);
}
//----------------------------------------------------------------------------
static FWindowsWindow* WindowsWindowFromHWnd_(::HWND hWnd) {
    Assert(hWnd);

    wchar_t className[MAX_PATH + 1];
    const size_t classNameSz = size_t(::GetClassNameW(hWnd, className, MAX_PATH));

    if (0 == classNameSz) {
        LOG_LASTERROR(Window, L"GetClassNameW");
        return nullptr;
    }

    return (Equals(FWStringView(className, classNameSz), GAppWindowClassName_)
        ? reinterpret_cast<FWindowsWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA))
        : nullptr/* unkown window class */ );
}
//----------------------------------------------------------------------------
static ::LRESULT CALLBACK WindowsWindowProc_(::HWND hWnd, ::UINT msg, ::WPARAM wParam, ::LPARAM lParam) {
    FWindowsWindow* window;
    if (WM_NCCREATE == msg) {
        // backup window in user data from create data :
        window = reinterpret_cast<FWindowsWindow*>(((::LPCREATESTRUCT)lParam)->lpCreateParams);
        window->SetHandleWin32(hWnd);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, checked_cast<LONG_PTR>(window));
    }
    else {
        // retrieve owner window from user data :
        window = reinterpret_cast<FWindowsWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    Assert(window->HandleWin32() == hWnd);

    if (window->WindowProcWin32(msg, wParam, lParam))
        return 0;

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
//----------------------------------------------------------------------------
static void AppCreateWindowClass_() {
    ::WNDCLASSEXW wc;
    ::SecureZeroMemory(&wc, sizeof(::WNDCLASSEXW));

    ::HINSTANCE const hInstance = AppHandleWin32_();
    const size_t appIcon = FCurrentProcess::Get().AppIcon();

    wc.cbSize = sizeof(::WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowsWindowProc_;
    wc.hInstance = hInstance;
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = ::LoadIconW(NULL, MAKEINTRESOURCEW(appIcon));
    wc.hIconSm = (::HICON)::LoadImageW(hInstance,
        MAKEINTRESOURCEW(appIcon),
        IMAGE_ICON,
        ::GetSystemMetrics(SM_CXSMICON),
        ::GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);
    wc.hbrBackground = (::HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = GAppWindowClassName_;

    if (!::RegisterClassExW(&wc))
        PPE_THROW_IT(FLastErrorException("RegisterClassExW"));
}
//----------------------------------------------------------------------------
static void AppDestroyWindowClass_() {
    if (!::UnregisterClassW(GAppWindowClassName_, AppHandleWin32_()))
        PPE_THROW_IT(FLastErrorException("UnregisterClassW"));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWindowsWindow::FWindowsWindow()
{}
//----------------------------------------------------------------------------
FWindowsWindow::~FWindowsWindow() {
    ::HWND const hWnd = HandleWin32();

    if (hWnd && not ::DestroyWindow(hWnd))
        LOG_LASTERROR(Window, L"DestroyWindow");
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Show() {
    ::HWND const hWnd = HandleWin32();
    Assert(hWnd);

    ::BOOL succeed;

    succeed = ::ShowWindow(hWnd, FCurrentProcess::Get().nShowCmd());
    CLOG_LASTERROR(not succeed, Window, L"ShowWindow");
    if (not succeed)
        return false;

    succeed = ::UpdateWindow(hWnd);
    CLOG_LASTERROR(not succeed, Window, L"UpdateWindow");
    if (not succeed)
        return false;

    return parent_type::Show();
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Close() {
    const ::BOOL succeed = ::CloseWindow(HandleWin32());
    CLOG_LASTERROR(not succeed, Window, L"CloseWindow");

    return (succeed && parent_type::Close());
}
//----------------------------------------------------------------------------
bool FWindowsWindow::PumpMessages() {
    return (parent_type::PumpMessages() &&
        FPlatformMessageHandler::PumpMessages(this));
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Center() {
    ::HWND const hWnd = HandleWin32();
    Assert(hWnd);

    ::LONG screenW, screenH;
    WindowsScreenSize_(hWnd, &screenW, &screenH);

    ::RECT wRect;
    CLOG_LASTERROR(not ::GetClientRect(hWnd, &wRect), Window, L"GetClientRect");

    const ::LONG clientW = (wRect.right - wRect.left);
    const ::LONG clientH = (wRect.bottom - wRect.top);

    const int windowX = checked_cast<int>(screenW - clientW) / 2;
    const int windowY = checked_cast<int>(screenH - clientH) / 2;

    if (::SetWindowPos(hWnd, NULL, windowX, windowY, clientW, clientH, SWP_NOZORDER | SWP_NOACTIVATE)) {
        UpdateClientRect();
        return parent_type::Center();
    }
    else {
        LOG_LASTERROR(Window, L"SetWindowPos");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Maximize() {
    Assert(Handle());

    if (::ShowWindow(HandleWin32(), SW_MAXIMIZE)) {
        return parent_type::Maximize();
    }
    else {
        LOG_LASTERROR(Window, L"ShowWindow");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Minimize() {
    Assert(Handle());

    if (::ShowWindow(HandleWin32(), SW_MINIMIZE)) {
        return parent_type::Maximize();
    }
    else {
        LOG_LASTERROR(Window, L"ShowWindow");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Move(int x, int y) {
    ::HWND const hWnd = HandleWin32();
    Assert(hWnd);

    const int cx = checked_cast<int>(Width());
    const int cy = checked_cast<int>(Height());

    if (::SetWindowPos(hWnd, NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE)) {
        UpdateClientRect();
        return parent_type::Move(x, y);
    }
    else {
        LOG_LASTERROR(Window, L"SetWindowPos");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Resize(size_t w, size_t h) {
    ::HWND const hWnd = HandleWin32();
    Assert(hWnd);

    const int cx = checked_cast<int>(w);
    const int cy = checked_cast<int>(h);

    if (::SetWindowPos(hWnd, NULL, Left(), Top(), cx, cy, SWP_NOZORDER | SWP_NOACTIVATE)) {
        UpdateClientRect();
        return parent_type::Resize(w, h);
    }
    else {
        LOG_LASTERROR(Window, L"SetWindowPos");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::SetFocus() {
    Assert(Handle());

    if (::SetActiveWindow(HandleWin32())) {
        return parent_type::SetFocus();
    }
    else {
        LOG_LASTERROR(Window, L"SetActiveWindow");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::SetFullscreen(bool value) {
    if (value) {
        ::HWND const hWnd = HandleWin32();
        Assert(hWnd);

        // Set new window style and size.
        ::SetWindowLong(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        ::SetWindowLong(hWnd, GWL_EXSTYLE, 0);

        ::LONG screenW, screenH;
        WindowsScreenSize_(hWnd, &screenW, &screenH);

        ::RECT wRect;
        Verify(::GetClientRect(hWnd, &wRect));

        const ::LONG clientW = (wRect.right - wRect.left);
        const ::LONG clientH = (wRect.bottom - wRect.top);

        const int windowX = checked_cast<int>(screenW - clientW) / 2;
        const int windowY = checked_cast<int>(screenH - clientH) / 2;

        if (::SetWindowPos(hWnd, NULL, windowX, windowY, clientW, clientH, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED)) {
            UpdateClientRect();
            return parent_type::SetFullscreen(true);
        }
        else {
            LOG_LASTERROR(Window, L"SetWindowPos");
        }

    }
    else {
        AssertNotImplemented(); // #TODO save previous state and restore here ?
    }

    return false;
}
//----------------------------------------------------------------------------
bool FWindowsWindow::SetTitle(FWString&& title) {
    Assert(Handle());

    if (::SetWindowTextW(HandleWin32(), title.data())) {
        return parent_type::SetTitle(std::move(title));
    }
    else {
        LOG_LASTERROR(Window, L"SetWindowTextW");
        return false;
    }
}
//----------------------------------------------------------------------------
void FWindowsWindow::ScreenToClient(int* screenX, int* screenY) const {
    Verify(FPlatformMouse::ScreenToClient(*this, screenX, screenY));
}
//----------------------------------------------------------------------------
void FWindowsWindow::ClientToScreen(int* clientX, int* clientY) const {
    Verify(FPlatformMouse::ClientToScreen(*this, clientX, clientY));
}
//----------------------------------------------------------------------------
void FWindowsWindow::SetCursorCapture(bool enabled) const {
    if (enabled)
        FPlatformMouse::SetCapture(*this);
    else
        FPlatformMouse::ResetCapture();
}
//----------------------------------------------------------------------------
void FWindowsWindow::SetCursorOnWindowCenter() const {
    FPlatformMouse::CenterCursorOnWindow(*this);
}
//----------------------------------------------------------------------------
FWindowsWindow* FWindowsWindow::ActiveWindow() {
    ::HWND const hWnd = ::GetActiveWindow();
    return (hWnd ? WindowsWindowFromHWnd_(hWnd) : nullptr);
}
//----------------------------------------------------------------------------
void FWindowsWindow::MainWindowDefinition(FWindowDefinition* def) {
    parent_type::MainWindowDefinition(def);
}
//----------------------------------------------------------------------------
bool FWindowsWindow::CreateWindow(FWindowsWindow* window, FWString&& title, const FWindowDefinition& def) {
    Assert(window);
    Assert(not window->Handle());

    ::DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    ::DWORD dwStyleEx = 0;

    switch (def.Type) {
    case EWindowType::Main:
        dwStyleEx = WS_EX_APPWINDOW;
        break;
    case EWindowType::Modal:
        dwStyle = WS_DLGFRAME | WS_POPUPWINDOW;
        break;
    case EWindowType::Tool:
        dwStyle = WS_POPUPWINDOW;
        dwStyleEx = WS_EX_TOOLWINDOW;
        break;
    case EWindowType::Child:
        dwStyle = WS_CHILDWINDOW;
        dwStyleEx = WS_EX_MDICHILD;
        break;
    case EWindowType::BorderLess:
        dwStyle = WS_POPUP | WS_VISIBLE;
        break;
    }

    // order matters here :
    if (def.Fullscreen)
        dwStyle = WS_POPUP | WS_VISIBLE;
    if (def.Maximized || def.Fullscreen)
        dwStyle |= WS_MAXIMIZE;
    if (def.AllowDragDrop)
        dwStyleEx |= WS_EX_ACCEPTFILES;
    if (not def.HasCloseButton)
        dwStyle &= ~WS_SYSMENU;
    if (not def.HasResizeButton)
        dwStyle &= ~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX);

    ::RECT wRect = {
        checked_cast<::LONG>(def.Left),
        checked_cast<::LONG>(def.Top),
        checked_cast<::LONG>(def.Left + def.Width),
        checked_cast<::LONG>(def.Top + def.Height) };

    if (not ::AdjustWindowRect(&wRect, dwStyle, FALSE))
        PPE_THROW_IT(FLastErrorException("AdjustWindowRect"));

    ::HWND const hParent = (def.Parent
        ? checked_cast<FWindowsWindow*>(def.Parent)->HandleWin32()
        : NULL );
    Assert(not def.Parent || hParent);

    FWindowDefinition corrected = def;
    corrected.Left = checked_cast<int>(wRect.left);
    corrected.Top = checked_cast<int>(wRect.top);
    corrected.Width = checked_cast<size_t>(wRect.right - wRect.left);
    corrected.Height = checked_cast<size_t>(wRect.bottom - wRect.top);

    Verify(parent_type::CreateWindow(window, std::move(title), def));

    ::HWND const hWnd = ::CreateWindowExW(
        dwStyleEx,
        GAppWindowClassName_,
        title.data(),
        dwStyle,
        checked_cast<int>(corrected.Left),
        checked_cast<int>(corrected.Top),
        checked_cast<int>(corrected.Width),
        checked_cast<int>(corrected.Height),
        hParent,
        NULL,
        AppHandleWin32_(),
        window );

    if (not hWnd)
        PPE_THROW_IT(FLastErrorException("CreateWindowExW"));

    window->SetHandleWin32(hWnd);

    if (def.Maximized)
        window->UpdateClientRect();
    if (def.Centered)
        Verify(window->Center());
    if (def.Fullscreen)
        Verify(window->SetFullscreen(true));
    if (def.AllowDragDrop)
        ::DragAcceptFiles(hWnd, true);

    return true;
}
//----------------------------------------------------------------------------
void FWindowsWindow::Start() {
    AppCreateWindowClass_();
}
//----------------------------------------------------------------------------
void FWindowsWindow::Shutdown() {
    AppDestroyWindowClass_();
}
//----------------------------------------------------------------------------
bool FWindowsWindow::WindowProcWin32(::UINT msg, ::WPARAM wParam, ::LPARAM lParam) {
    switch (msg) {
    // Paint :
    case WM_PAINT:
        PaintProcWin32();
        return true;

    // Focus :
    case WM_SETFOCUS:
        OnFocusSet();
        return true;
    case WM_KILLFOCUS:
        OnFocusLose();
        return true;

    // Mouse events :
    case WM_MOUSEHOVER:
        OnMouseEnter();
        break;
    case WM_MOUSELEAVE:
        OnMouseLeave();
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_LBUTTONUP:
        OnMouseClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EMouseButton::Button0);
        break;
    case WM_RBUTTONUP:
        OnMouseClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EMouseButton::Button1);
        break;
    case WM_MBUTTONUP:
        OnMouseClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EMouseButton::Button2);
        break;

    case WM_LBUTTONDBLCLK:
        OnMouseDoubleClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EMouseButton::Button0);
        return true;
    case WM_RBUTTONDBLCLK:
        OnMouseDoubleClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EMouseButton::Button1);
        return true;
    case WM_MBUTTONDBLCLK:
        OnMouseDoubleClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), EMouseButton::Button2);
        return true;

    case WM_MOUSEWHEEL:
        OnMouseWheel(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), GET_WHEEL_DELTA_WPARAM(wParam));
        break;

    // Window events :
    case WM_SHOWWINDOW:
        OnWindowShow();
        return true;
    case WM_CLOSE:
        OnWindowClose();
        return true;
    case WM_MOVE:
        OnWindowMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return true;
    case WM_SIZE:
        OnWindowResize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return true;
    case WM_DESTROY:
        OnWindowClose();
        if (Type() == EWindowType::Main)
            ::PostQuitMessage(0);
        return true;

    default:
        break;
    }

    // Drag drop events :
    if (msg == WM_DROPFILES && AllowDragDrop()) {
        DragDropProcWin32(wParam);
        return true;
    }

    return DispatchEventWin32(msg, wParam, lParam);
}
//----------------------------------------------------------------------------
void FWindowsWindow::UpdateClientRect() {
    ::HWND const hWnd = HandleWin32();
    Assert(hWnd);

    ::RECT wRect;
    Verify(::GetClientRect(hWnd, &wRect));

    const size_t clientW = checked_cast<size_t>(wRect.right - wRect.left);
    const size_t clientH = checked_cast<size_t>(wRect.bottom - wRect.top);

    // only updates _left,_top,_width,_height using parent_type :
    parent_type::Move(wRect.left, wRect.top);
    parent_type::Resize(clientW, clientH);
}
//----------------------------------------------------------------------------
bool FWindowsWindow::DispatchEventWin32(::UINT msg, ::WPARAM wParam, ::LPARAM lParam) {
    FWindowsMessage broadcast;
    broadcast.Type = EWindowsMessageType(msg);
    broadcast.LParam = lParam;
    broadcast.WParam = wParam;
    broadcast.Handled = false;

    _OnMessageWin32(*this, &broadcast);

    return broadcast.Handled;
}
//----------------------------------------------------------------------------
// https://www.codeproject.com/Articles/840/How-to-Implement-Drag-and-Drop-Between-Your-Progra
void FWindowsWindow::DragDropProcWin32(::WPARAM wParam) {
    Assert(wParam);

    ::HDROP const hDrop = (::HDROP)wParam;

    // get the # of files being dropped.
    const ::UINT uNumFiles = ::DragQueryFileW(hDrop, ::UINT(-1), NULL, 0);

    ::TCHAR szNextFile[MAX_PATH + 1];
    forrange(uFile, 0, uNumFiles) {
        // get the next filename from the HDROP info.
        if (::DragQueryFileW(hDrop, uFile, szNextFile, MAX_PATH) > 0) {
            OnDragDropFile(MakeCStringView(szNextFile));
        }
    }

    // Free up memory.
    ::DragFinish(hDrop);
}
//----------------------------------------------------------------------------
void FWindowsWindow::PaintProcWin32() {
    ::HWND const hWnd = HandleWin32();
    Assert(hWnd);

    ::PAINTSTRUCT ps;
    ::HDC const hdc = ::BeginPaint(hWnd, &ps);
    NOOP(hdc);
    Assert(hdc);

    OnWindowPaint();

    Verify(::EndPaint(hWnd, &ps));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
