﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsWindow.h"

#include "HAL/PlatformIncludes.h"
#include "HAL/PlatformMessageHandler.h"
#include "HAL/PlatformMouse.h"
#include "HAL/PlatformNotification.h"
#include "HAL/PlatformSurvey.h"
#include "HAL/Windows/LastError.h"

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "Input/MouseButton.h"
#include "Misc/Function.h"

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
    return static_cast<::HINSTANCE>(FCurrentProcess::Get().AppHandle());
}
//----------------------------------------------------------------------------
static const FWStringLiteral GAppWindowClassName_{ L"PPE::MainWindow" };
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
        PPE_LOG_LASTERROR(Window, "GetClassNameW");
        return nullptr;
    }

    return (Equals(FWStringView(className, classNameSz), GAppWindowClassName_.MakeView())
        ? reinterpret_cast<FWindowsWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA))
        : nullptr/* unkown window class */ );
}
//----------------------------------------------------------------------------
static ::LRESULT CALLBACK WindowsWindowProc_(::HWND hWnd, ::UINT msg, ::WPARAM wParam, ::LPARAM lParam) {
    Assert(hWnd);
    FWindowsWindow* window = nullptr;

    switch (msg) {
    case WM_NCCREATE:
    {
        // backup window in user data from create data:
        window = reinterpret_cast<FWindowsWindow*>(((::LPCREATESTRUCT)lParam)->lpCreateParams);
        window->SetHandleWin32(hWnd);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, checked_cast<LONG_PTR>(window));
        break;
    }
    default:
    {
        // retrieve owner window from user data :
        window = reinterpret_cast<FWindowsWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
        break;
    }}

    if (window) {
        Assert(window->HandleWin32() == hWnd);
        if (window->WindowProcWin32(msg, wParam, lParam))
            return 0;
    }

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
    wc.hIcon = (HICON)LoadImageW(hInstance,
        MAKEINTRESOURCE(appIcon),
        IMAGE_ICON,
        ::GetSystemMetrics(SM_CXICON),
        ::GetSystemMetrics(SM_CYICON),
        LR_DEFAULTCOLOR | LR_SHARED);
    wc.hIconSm = (::HICON)::LoadImageW(hInstance,
        MAKEINTRESOURCEW(appIcon),
        IMAGE_ICON,
        ::GetSystemMetrics(SM_CXSMICON),
        ::GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR | LR_SHARED);
    wc.hbrBackground = (::HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = GAppWindowClassName_.c_str();

    if (!::RegisterClassExW(&wc))
        PPE_THROW_IT(FLastErrorException("RegisterClassExW"));
}
//----------------------------------------------------------------------------
static void AppDestroyWindowClass_() {
    if (!::UnregisterClassW(GAppWindowClassName_.c_str(), AppHandleWin32_()))
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
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    ::HWND const hWnd = HandleWin32();

    if (hWnd && not ::DestroyWindow(hWnd))
        PPE_LOG_LASTERROR(Window, "DestroyWindow");

    SetNativeHandle(nullptr);
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Show() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    ::HWND const hWnd = HandleWin32();
    Assert(hWnd);

    if (::ShowWindow(hWnd, FCurrentProcess::Get().nShowCmd())) {
        PPE_LOG_LASTERROR(Window, "ShowWindow");
        return false;
    }

    if (not ::UpdateWindow(hWnd)) {
        PPE_LOG_LASTERROR(Window, "UpdateWindow");
        return false;
    }

    return parent_type::Show();
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Close() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    if (not parent_type::Close())
        return false;

    if (not ::CloseWindow(HandleWin32())) {
        PPE_LOG_LASTERROR(Window, "CloseWindow");
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FWindowsWindow::PumpMessages() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    return (parent_type::PumpMessages() &&
        FPlatformMessageHandler::PumpMessages(*this));
}
//----------------------------------------------------------------------------
bool FWindowsWindow::BringToFront() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    Assert(NativeHandle());

    if (::SetForegroundWindow(HandleWin32())) {
        return parent_type::BringToFront();
    }
    else {
        PPE_LOG_LASTERROR(Window, "SetForegroundWindow");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Center() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    ::HWND const hWnd = HandleWin32();
    Assert(hWnd);

    ::LONG screenW, screenH;
    WindowsScreenSize_(hWnd, &screenW, &screenH);

    ::RECT wRect;
    PPE_CLOG_LASTERROR(not ::GetClientRect(hWnd, &wRect), Window, "GetClientRect");

    const ::LONG clientW = (wRect.right - wRect.left);
    const ::LONG clientH = (wRect.bottom - wRect.top);

    const int windowX = checked_cast<int>(screenW - clientW) / 2;
    const int windowY = checked_cast<int>(screenH - clientH) / 2;

    if (::SetWindowPos(hWnd, NULL, windowX, windowY, clientW, clientH, SWP_NOZORDER | SWP_NOACTIVATE)) {
        UpdateClientRect();
        return parent_type::Center();
    }
    else {
        PPE_LOG_LASTERROR(Window, "SetWindowPos");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Maximize() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    Assert(NativeHandle());

    if (::ShowWindow(HandleWin32(), SW_MAXIMIZE)) {
        return parent_type::Maximize();
    }
    else {
        PPE_LOG_LASTERROR(Window, "ShowWindow");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Minimize() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    Assert(NativeHandle());

    if (::ShowWindow(HandleWin32(), SW_MINIMIZE)) {
        return parent_type::Maximize();
    }
    else {
        PPE_LOG_LASTERROR(Window, "ShowWindow");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Move(int x, int y) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    ::HWND const hWnd = HandleWin32();
    Assert(hWnd);

    const int cx = checked_cast<int>(Width());
    const int cy = checked_cast<int>(Height());

    if (::SetWindowPos(hWnd, NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE)) {
        UpdateClientRect();
        return parent_type::Move(x, y);
    }
    else {
        PPE_LOG_LASTERROR(Window, "SetWindowPos");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::Resize(size_t w, size_t h) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    ::HWND const hWnd = HandleWin32();
    Assert(hWnd);

    const int cx = checked_cast<int>(w);
    const int cy = checked_cast<int>(h);

    if (::SetWindowPos(hWnd, NULL, Left(), Top(), cx, cy, SWP_NOZORDER | SWP_NOACTIVATE)) {
        UpdateClientRect();
        return parent_type::Resize(w, h);
    }
    else {
        PPE_LOG_LASTERROR(Window, "SetWindowPos");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::SetFocus() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    Assert(NativeHandle());

    if (::SetActiveWindow(HandleWin32())) {
        return parent_type::SetFocus();
    }
    else {
        PPE_LOG_LASTERROR(Window, "SetActiveWindow");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::SetFullscreen(bool value) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
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
            PPE_LOG_LASTERROR(Window, "SetWindowPos");
        }

    }
    else {
        AssertNotImplemented(); // #TODO save previous state and restore here ?
    }

    return false;
}
//----------------------------------------------------------------------------
bool FWindowsWindow::SetTitle(FWString&& title) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    Assert(NativeHandle());

    if (::SetWindowTextW(HandleWin32(), title.data())) {
        return parent_type::SetTitle(std::move(title));
    }
    else {
        PPE_LOG_LASTERROR(Window, "SetWindowTextW");
        return false;
    }
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
void FWindowsWindow::HiddenWindowDefinition(FWindowDefinition* def) {
    parent_type::HiddenWindowDefinition(def);
}
//----------------------------------------------------------------------------
bool FWindowsWindow::CreateWindow(FWindowsWindow* window, FWString&& title, const FWindowDefinition& def) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(window);
    Assert(window);
    Assert(not window->NativeHandle());

    Verify(parent_type::CreateWindow(window, FWString(title), def));

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
    if (def.Invisible) {
        dwStyle &= ~(WS_VISIBLE);    // this works - window become invisible
        dwStyle |= WS_EX_TOOLWINDOW;   // flags don't work - windows remains in taskbar
        dwStyle &= ~(WS_EX_APPWINDOW);
    }

    FWindowDefinition corrected = def;

    if (def.AutoSize) {
        corrected.Left = CW_USEDEFAULT;
        corrected.Top = CW_USEDEFAULT;
        corrected.Width = size_t(CW_USEDEFAULT);
        corrected.Height = size_t(CW_USEDEFAULT);
    }
    else {
        ::RECT wRect = {
            checked_cast<::LONG>(def.Left),
            checked_cast<::LONG>(def.Top),
            checked_cast<::LONG>(def.Left + def.Width),
            checked_cast<::LONG>(def.Top + def.Height) };

        if (not ::AdjustWindowRect(&wRect, dwStyle, FALSE))
            PPE_THROW_IT(FLastErrorException("AdjustWindowRect"));

        corrected.Left = checked_cast<int>(wRect.left);
        corrected.Top = checked_cast<int>(wRect.top);
        corrected.Width = checked_cast<size_t>(wRect.right - wRect.left);
        corrected.Height = checked_cast<size_t>(wRect.bottom - wRect.top);
    }

    ::HWND const hParent = (def.Parent
        ? checked_cast<FWindowsWindow*>(def.Parent)->HandleWin32()
        : NULL );
    Assert(not def.Parent || hParent);

    ::HWND const hWnd = ::CreateWindowExW(
        dwStyleEx,
        GAppWindowClassName_.c_str(),
        title.data(),
        dwStyle,
        checked_cast<int>(corrected.Left),
        checked_cast<int>(corrected.Top),
        int(corrected.Width),
        int(corrected.Height),
        hParent,
        NULL,
        AppHandleWin32_(),
        window );

    if (not hWnd)
        PPE_THROW_IT(FLastErrorException("CreateWindowExW"));

    window->SetHandleWin32(hWnd);
    window->UpdateMonitorDPIWin32();

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
    /*case WM_PAINT:
        PaintProcWin32();
        break;*/

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
    case WM_DPICHANGED:
        OnWindowDPI(HIWORD(wParam));
        break;
    case WM_MOVE:
        OnWindowMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;
    case WM_SIZE:
        OnWindowResize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;
    case WM_SHOWWINDOW:
        OnWindowShow(TRUE == wParam);
        break;
    case WM_CLOSE:
        OnWindowClose();
        break;
    case WM_DESTROY:
        if (Type() == EWindowType::Main) {
            PPE_LOG(Window, Info, "post quit message from <{0}> window", Title());
            ::PostQuitMessage(0);
        }
        break;

    case WM_NCDESTROY:
        SetHandleWin32(nullptr);
        return true;

    case FWindowsPlatformNotification::WM_SYSTRAY:
        switch (LOWORD(lParam)) {
        case WM_RBUTTONDOWN:
            FWindowsPlatformNotification::SummonSystrayPopupMenuWin32(HandleWin32());
            return true;
        }
        break;

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
void FWindowsWindow::UpdateMonitorDPIWin32() {
    FPlatformSurvey::FMonitorInfo monitorInfo;
    if (FPlatformSurvey::MonitorFromWindow(*this, &monitorInfo)) {
        OnWindowDPI(monitorInfo.DPIScale);
    }
}
//----------------------------------------------------------------------------
bool FWindowsWindow::DispatchEventWin32(::UINT msg, ::WPARAM wParam, ::LPARAM lParam) {
    FWindowsMessage broadcast{};
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
