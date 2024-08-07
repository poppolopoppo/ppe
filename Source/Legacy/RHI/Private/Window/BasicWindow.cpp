﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "BasicWindow.h"

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/LastError.h"
#include "Diagnostic/Logger.h"

#ifdef PLATFORM_WINDOWS
#   include "Misc/Platform_Windows.h"
#else
#   error "no support"
#endif

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FBasicWindowHelper {
    static void SetWindowHandle(FBasicWindow *wnd, void *handle);
    static void SetWindowFocus(FBasicWindow *wnd, void *handle, bool hasFocus);
    static void SetWindowSize(FBasicWindow *wnd, void *handle, size_t width, size_t height);
};
//----------------------------------------------------------------------------
void FBasicWindowHelper::SetWindowHandle(FBasicWindow *wnd, void *handle) {
    Assert(handle);
    wnd->_handle = handle;
}
//----------------------------------------------------------------------------
void FBasicWindowHelper::SetWindowFocus(FBasicWindow *wnd, void *handle, bool hasFocus) {
    UNUSED(handle);
    Assert(wnd);
    Assert(wnd->_handle == handle);
    wnd->_wantFocus = hasFocus;
}
//----------------------------------------------------------------------------
void FBasicWindowHelper::SetWindowSize(FBasicWindow *wnd, void *handle, size_t width, size_t height) {
    UNUSED(handle);
    Assert(wnd);
    Assert(wnd->_handle == handle);
    wnd->_wantedWidth = width;
    wnd->_wantedHeight = height;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define BASICWINDOW_CLASNAME L"PPE::Graphics::BasicWindowClass"
//----------------------------------------------------------------------------
static LRESULT CALLBACK BasicWindowProc_(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) {
    FBasicWindow *wnd = nullptr;

    if (WM_NCCREATE == msg) {
        wnd = reinterpret_cast<FBasicWindow *>(((LPCREATESTRUCT)lparam)->lpCreateParams);
        ::SetWindowLongPtr(handle, GWLP_USERDATA, checked_cast<LONG_PTR>(wnd));
        FBasicWindowHelper::SetWindowHandle(wnd, handle);

        const size_t appIcon = FCurrentProcess::Get().AppIcon();
        if (appIcon) { // set window icon from current process resources
            HMODULE module = ::GetModuleHandle(0);

            const HANDLE hbicon = ::LoadImage(module, MAKEINTRESOURCE(appIcon), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), 0);
            if (hbicon)
                ::SendMessage(handle, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hbicon));

            const HANDLE hsicon = ::LoadImage(module, MAKEINTRESOURCE(appIcon), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
            if (hsicon)
                ::SendMessage(handle, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hsicon));
        }
    }
    else {
        wnd = reinterpret_cast<FBasicWindow *>(::GetWindowLongPtr(handle, GWLP_USERDATA));
    }

    HDC hdc;
    PAINTSTRUCT ps;

    switch (msg)
    {
    case WM_PAINT:
        hdc = BeginPaint(handle, &ps);
        EndPaint(handle, &ps);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SETFOCUS:
        FBasicWindowHelper::SetWindowFocus(wnd, handle, true);
        return 0;

    case WM_KILLFOCUS:
        FBasicWindowHelper::SetWindowFocus(wnd, handle, false);
        return 0;

    case WM_SIZE:
        FBasicWindowHelper::SetWindowSize(wnd, handle, LOWORD(lparam), HIWORD(lparam));
        return 0;
    }

    MessageResult result;
    if (wnd && wnd->DispatchMessageIFP(static_cast<EWindowMessage>(msg), lparam, wparam, &result))
        return result;
    else
        return DefWindowProc(handle, msg, wparam, lparam);
}
//----------------------------------------------------------------------------
static void CreateBasicWindowClass_() {
    WNDCLASSEX wc;

    SecureZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = BasicWindowProc_;
    wc.hInstance = reinterpret_cast<HINSTANCE>(FCurrentProcess::Get().ApplicationHandle());
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = BASICWINDOW_CLASNAME;

    if (!RegisterClassEx(&wc))
        PPE_THROW_IT(FLastErrorException());
}
//----------------------------------------------------------------------------
static void DestroyBasicWindowClass_() {
    if (!UnregisterClass(BASICWINDOW_CLASNAME, reinterpret_cast<HINSTANCE>(FCurrentProcess::Get().ApplicationHandle())))
        PPE_THROW_IT(FLastErrorException());
}
//----------------------------------------------------------------------------
static HWND CreateBasicWindowHandle_(
    FBasicWindow *wnd,
    const FWStringView& title,
    int left, int top,
    size_t width, size_t height,
    FBasicWindow *parent) {
    const DWORD windowStyle = WS_OVERLAPPEDWINDOW;

    RECT size = { 0, 0, checked_cast<LONG>(width), checked_cast<LONG>(height) };
    if (!::AdjustWindowRect(&size, windowStyle, FALSE))
        PPE_THROW_IT(FLastErrorException());

    wchar_t titleCstr[128];
    title.ToNullTerminatedCStr(titleCstr);

    HWND handle = ::CreateWindowEx(
        NULL,
        BASICWINDOW_CLASNAME,
        titleCstr,
        windowStyle,
        left - size.left,
        top - size.top,
        size.right - size.left,
        size.bottom - size.top,
        reinterpret_cast<HWND>((parent) ? parent->Handle() : NULL),
        NULL,
        reinterpret_cast<HINSTANCE>(FCurrentProcess::Get().ApplicationHandle()),
        checked_cast<LPVOID>(wnd));

    if (!handle)
        PPE_THROW_IT(FLastErrorException());

    return handle;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBasicWindow::FBasicWindow(
    const FWStringView& title,
    int left, int top,
    size_t width, size_t height,
    FBasicWindow *parent/* = nullptr */)
:   _handle(nullptr)
,   _title(title)
,   _width(width)
,   _height(height)
,   _parent(parent)
,   _hasFocus(false)
,   _wantFocus(false)
,   _wantedWidth(width)
,   _wantedHeight(height) {
    Assert(not title.empty());

    _handle = CreateBasicWindowHandle_(this, title, left, top, width, height, _parent);

    LOG(Info, L"[Window] Create window \"{0}\", RECT[{1}, {2}, {3}, {4}] = {5}",
        title, left, top, width, height, _handle);
}
//----------------------------------------------------------------------------
FBasicWindow::~FBasicWindow() {
    Assert(_handle);

    LOG(Info, L"[Window] Destroy window {0}", _handle);

    ::DestroyWindow(reinterpret_cast<HWND>(_handle));
}
//----------------------------------------------------------------------------
void FBasicWindow::RegisterMessageHandler(IWindowMessageHandler *handler) {
    Assert(handler);
    Assert(nullptr == handler->Window());

    Add_AssertUnique(_handlers, handler);

    handler->SetWindow(this);
    handler->RegisterMessageDelegates(this);
}
//----------------------------------------------------------------------------
void FBasicWindow::UnregisterMessageHandler(IWindowMessageHandler *handler) {
    Assert(handler);
    Assert(this == handler->Window());

    handler->UnregisterMessageDelegates(this);
    handler->SetWindow(nullptr);

    Remove_AssertExists(_handlers, handler);
}
//----------------------------------------------------------------------------
void FBasicWindow::Update_BeforeDispatch() {
    UpdateMessageHandlers_BeforeDispatch_();
}
//----------------------------------------------------------------------------
void FBasicWindow::Update_AfterDispatch() {
    UpdateMessageHandlers_AfterDispatch_();

    if (_wantFocus != _hasFocus) {
        if (_wantFocus)
            OnSetFocus();
        else
            OnLoseFocus();
    }

    if (_wantedWidth != _width ||
        _wantedHeight != _height)
        OnResize(_wantedWidth, _wantedHeight);
}
//----------------------------------------------------------------------------
bool FBasicWindow::DispatchMessageIFP(EWindowMessage msg, MessageLParam lparam, MessageWParam wparam, MessageResult *result) {
    Assert(result);

    WindowMessageHandlerDelegate handlerDelegate;
    if (_dispatch.Find(msg, &handlerDelegate)) {
        /*LOG(Info, L"[Window] Dispatch message <{1}> for handle {0} (lparam = {2}, wparam = {3})",
            _handle, msg, lparam, wparam);*/

        *result = (*handlerDelegate.second)(handlerDelegate.first, this, msg, lparam, wparam);
        return true;
    }
    else {
        *result = 0;
        return false;
    }
}
//----------------------------------------------------------------------------
void FBasicWindow::Show() {
    Assert(_handle);

    LOG(Info, L"[Window] Show window {0}", _handle);

    ::ShowWindow(reinterpret_cast<HWND>(_handle), FCurrentProcess::Get().nShowCmd());
}
//----------------------------------------------------------------------------
void FBasicWindow::Close() {
    Assert(_handle);

    LOG(Info, L"[Window] Close window {0}", _handle);

    ::CloseWindow(reinterpret_cast<HWND>(_handle));
}
//----------------------------------------------------------------------------
bool FBasicWindow::PumpMessage(EWindowMessage& msg, MessageLParam& lparam, MessageWParam& wparam) {
    Assert(_handle);

    MSG windowMsg;
    if (::PeekMessage(&windowMsg, NULL, 0, 0, PM_REMOVE))
    {
        ::TranslateMessage(&windowMsg);
        ::DispatchMessage(&windowMsg);

        msg = static_cast<EWindowMessage>(windowMsg.message);
        lparam = windowMsg.lParam;
        wparam = windowMsg.wParam;

        /*
        LOG(Info, L"[FBasicWindow] Received window message #{0} <{1}>",
            size_t(msg), WindowMessageToCStr(msg));
            */

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
bool FBasicWindow::PumpAllMessages_ReturnIfQuit() {
    bool quit = false;

    EWindowMessage msg;
    MessageLParam lparam;
    MessageWParam wparam;
    while (PumpMessage(msg, lparam, wparam))
        quit |= (msg == EWindowMessage::Quit);

    return quit;
}
//----------------------------------------------------------------------------
void FBasicWindow::Start() {
    CreateBasicWindowClass_();
}
//----------------------------------------------------------------------------
void FBasicWindow::Shutdown() {
    DestroyBasicWindowClass_();
}
//----------------------------------------------------------------------------
void FBasicWindow::OnSetFocus() {
    Assert(true == _wantFocus);
    LOG(Info, L"[Window] Window {0} set focus", _handle);

    _hasFocus = true;
}
//----------------------------------------------------------------------------
void FBasicWindow::OnLoseFocus() {
    Assert(false == _wantFocus);
    LOG(Info, L"[Window] Window {0} lose focus", _handle);

    _hasFocus = false;
}
//----------------------------------------------------------------------------
void FBasicWindow::OnResize(size_t width, size_t height) {
    LOG(Info, L"[Window] Window {0} resize from {1}x{2} -> {3}x{4}", _handle, _width, _height, width, height);

    // overwrites _wantedWidth/Height to enable child impl to pass a different size
    _width = _wantedWidth = width;
    _height = _wantedHeight = height;
}
//----------------------------------------------------------------------------
void FBasicWindow::RegisterMessageDelegate_(EWindowMessage msg, IWindowMessageHandler *handler, IWindowMessageHandler::TDelegate member) {
    Assert(handler);
    Assert(member);
    Assert(Contains(_handlers, handler));
    Assert(this == handler->Window());

    LOG(Info, L"[Window] Register {1} message handler for window '{0}'", _title, msg);

    _dispatch.Insert_AssertUnique(std::forward<EWindowMessage>(msg), MakePair(handler, member));
}
//----------------------------------------------------------------------------
void FBasicWindow::UnregisterMessageDelegate_(EWindowMessage msg, IWindowMessageHandler *handler, IWindowMessageHandler::TDelegate member) {
    UNUSED(handler);
    UNUSED(member);
    Assert(handler);
    Assert(member);
    Assert(Contains(_handlers, handler));
    Assert(this == handler->Window());

    LOG(Info, L"[Window] Unregister {1} message handler for window '{0}'", _title, msg);

    const auto it = _dispatch.Find(msg);
    AssertRelease(it != _dispatch.end());
    Assert(it->second.first == handler);
    Assert(it->second.second == member);

    _dispatch.Erase(it);
}
//----------------------------------------------------------------------------
void FBasicWindow::UpdateMessageHandlers_BeforeDispatch_() {
    for (IWindowMessageHandler *handler : _handlers)
        handler->UpdateBeforeDispatch(this);
}
//----------------------------------------------------------------------------
void FBasicWindow::UpdateMessageHandlers_AfterDispatch_() {
    for (IWindowMessageHandler *handler : _handlers)
        handler->UpdateAfterDispatch(this);
}
//----------------------------------------------------------------------------
void FBasicWindow::ScreenToClient(int *x, int *y) const {
    Assert(x);
    Assert(y);

    ::POINT pt = {checked_cast<long>(*x), checked_cast<long>(*y)};
    ::ScreenToClient(reinterpret_cast<HWND>(_handle), &pt);

    *x = checked_cast<int>(pt.x);
    *y = checked_cast<int>(pt.y);
}
//----------------------------------------------------------------------------
void FBasicWindow::ClientToScreen(int *x, int *y) const {
    Assert(x);
    Assert(y);

    ::POINT pt = {checked_cast<long>(*x), checked_cast<long>(*y)};
    ::ClientToScreen(reinterpret_cast<HWND>(_handle), &pt);

    *x = checked_cast<int>(pt.x);
    *y = checked_cast<int>(pt.y);
}
//----------------------------------------------------------------------------
void FBasicWindow::SetCursorCapture(bool enabled) const {
    if (enabled)
        ::SetCapture(reinterpret_cast<HWND>(_handle));
    else
        ::ReleaseCapture();

    ::ShowCursor(!enabled);
}
//----------------------------------------------------------------------------
void FBasicWindow::SetCursorPositionOnScreenCenter() const {
    ::POINT pt = {checked_cast<LONG>(_width/2), checked_cast<LONG>(_height/2)};
    ::ClientToScreen(reinterpret_cast<HWND>(_handle), &pt);
    ::SetCursorPos(pt.x, pt.y);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
