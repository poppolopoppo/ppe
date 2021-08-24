#include "stdafx.h"

#ifdef PLATFORM_GLFW

#include "HAL/GLFW/GLFWWindow.h"

#include "HAL/GLFW/GLFWPlatformIncludes.h"
#include "HAL/GLFW/GLFWPlatformMessageHandler.h"

#include "HAL/PlatformIncludes.h"
#include "HAL/PlatformMouse.h"
#include "HAL/PlatformNotification.h"

#include "Container/Stack.h"
#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "Input/MouseButton.h"
#include "IO/StaticString.h"
#include "IO/String.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static GLFWwindow* NativeHandle_(const FGLFWWindow& window) {
    Assert(window.NativeHandle());
    return static_cast<GLFWwindow*>(window.NativeHandle());
}
//----------------------------------------------------------------------------
static FGLFWWindow& GLFWWindow_(GLFWwindow* nativeHandle) {
    Assert(nativeHandle);
    auto* const window = static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(nativeHandle));
    Assert(window);
    return (*window);
}
//----------------------------------------------------------------------------
// track active window
using FGLFWWindowStackRef_ = TThreadSafe<
    TFixedSizeStack<FGLFWWindow*, 4>,
    EThreadBarrier::AtomicSpinLock >;
static FGLFWWindowStackRef_& ActiveWindow_() {
    ONE_TIME_DEFAULT_INITIALIZE(FGLFWWindowStackRef_, GInstance);
    return GInstance;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGLFWWindow::FWindowCallbacks {
    static void Position(GLFWwindow* nativeHandle, int xpos, int ypos) {
        GLFWWindow_(nativeHandle).OnMouseMove(xpos, ypos);
    }

    static void Size(GLFWwindow* nativeHandle, int width, int height) {
        GLFWWindow_(nativeHandle).OnWindowResize(
            checked_cast<size_t>(width),
            checked_cast<size_t>(height) );
    }

    static void CursorEnter(GLFWwindow* nativeHandle, int entered) {
        FGLFWWindow& window = GLFWWindow_(nativeHandle);

        if (entered)
            window.OnMouseEnter();
        else
            window.OnMouseLeave();
    }

    static void Focus(GLFWwindow* nativeHandle, int focused) {
        FGLFWWindow& window = GLFWWindow_(nativeHandle);

        if (focused)
            window.OnFocusSet();
        else
            window.OnFocusLose();
    }

    static void Refresh(GLFWwindow* nativeHandle) {
        GLFWWindow_(nativeHandle).OnWindowPaint();
    }

    static void Drop(GLFWwindow* nativeHandle, int path_count, const char* paths[]) {
        FGLFWWindow& window = GLFWWindow_(nativeHandle);

        forrange(i, 0, path_count)
            window.OnDragDropFile(UTF_8_TO_WCHAR(paths[i]));
    }
};
//----------------------------------------------------------------------------
FGLFWWindow::FGLFWWindow()
{}
//----------------------------------------------------------------------------
FGLFWWindow::~FGLFWWindow() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    // track active window (tolerant)
    {
        const auto exclusive = ActiveWindow_().LockExclusive();
        const auto it = exclusive->Find(MakePtrRef(this));
        if (exclusive->end() != it)
            exclusive->Erase(it);
    }

    // destroy GLFW window IFN
    if (Likely(NativeHandle())) {
        glfwDestroyWindow(NativeHandle_(*this));
        SetNativeHandle(nullptr);
    }
}
//----------------------------------------------------------------------------
bool FGLFWWindow::Show() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    glfwShowWindow(NativeHandle_(*this));

    return FGenericWindow::Show();
}
//----------------------------------------------------------------------------
bool FGLFWWindow::Close() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    glfwHideWindow(NativeHandle_(*this));

    return FGenericWindow::Close();
}
//----------------------------------------------------------------------------
bool FGLFWWindow::PumpMessages() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);
    return (FGenericWindow::PumpMessages() &&
        FGLFWPlatformMessageHandler::PumpMessages(*this) );
}
//----------------------------------------------------------------------------
bool FGLFWWindow::Center() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    GLFWwindow* const nativeHandle = NativeHandle_(*this);
    if (GLFWmonitor* const monitor = glfwGetWindowMonitor(nativeHandle)) {
        int windowW, windowH;
        glfwGetWindowSize(nativeHandle, &windowW, &windowH);

        int monitorW, monitorH;
        glfwGetMonitorPhysicalSize(monitor, &monitorW, &monitorH);

        const int screenX = (monitorW - windowW) / 2;
        const int screenY = (monitorH - windowH) / 2;
        glfwSetWindowPos(nativeHandle, screenX, screenY);

        return FGenericWindow::Center();
    }

    return false;
}
//----------------------------------------------------------------------------
bool FGLFWWindow::Maximize() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    GLFWwindow* const nativeHandle = NativeHandle_(*this);
    glfwRestoreWindow(nativeHandle);
    glfwMaximizeWindow(nativeHandle);

    return FGenericWindow::Maximize();
}
//----------------------------------------------------------------------------
bool FGLFWWindow::Minimize() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    glfwIconifyWindow(NativeHandle_(*this));

    return FGenericWindow::Minimize();
}
//----------------------------------------------------------------------------
bool FGLFWWindow::Move(int x, int y) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    glfwSetWindowPos(NativeHandle_(*this), x, y);

    return FGenericWindow::Move(x, y);
}
//----------------------------------------------------------------------------
bool FGLFWWindow::Resize(size_t w, size_t h) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    glfwSetWindowSize(NativeHandle_(*this), checked_cast<int>(w), checked_cast<int>(h));

    return FGenericWindow::Resize(w, h);
}
//----------------------------------------------------------------------------
bool FGLFWWindow::SetFocus() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    glfwFocusWindow(NativeHandle_(*this));

    return FGenericWindow::SetFocus();
}
//----------------------------------------------------------------------------
bool FGLFWWindow::SetFullscreen(bool value) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    GLFWwindow* const nativeHandle = NativeHandle_(*this);
    GLFWmonitor* monitor = glfwGetWindowMonitor(nativeHandle);

    if (!!monitor == value)
        return true; // already in wanted state

    if (value) {
        Assert(!monitor);
        monitor = glfwGetPrimaryMonitor();

        const GLFWvidmode* const mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(nativeHandle, monitor, 0, 0, mode->width, mode->height, 0);
    }
    else {
        glfwSetWindowMonitor(nativeHandle, nullptr, Left(), Top(), checked_cast<int>(Width()), checked_cast<int>(Height()), 0);
    }

    return FGenericWindow::SetFullscreen(value);
}
//----------------------------------------------------------------------------
bool FGLFWWindow::SetTitle(FWString&& title) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    glfwSetWindowTitle(NativeHandle_(*this), WCHAR_TO_UTF_8(title.c_str()));

    return FGenericWindow::SetTitle(std::move(title));
}
//----------------------------------------------------------------------------
FGLFWWindow* FGLFWWindow::ActiveWindow() {
    return ActiveWindow_().LockExclusive()->Peek();
}
//----------------------------------------------------------------------------
auto FGLFWWindow::ActiveNativeHandle() -> FNativeHandle {
    FGLFWWindow* const active = ActiveWindow();
    return (active ? active->NativeHandle() : nullptr);
}
//----------------------------------------------------------------------------
void FGLFWWindow::MainWindowDefinition(FWindowDefinition* def) {
    FGenericWindow::MainWindowDefinition(def);
}
//----------------------------------------------------------------------------
void FGLFWWindow::HiddenWindowDefinition(FWindowDefinition* def) {
    FGenericWindow::HiddenWindowDefinition(def);
}
//----------------------------------------------------------------------------
bool FGLFWWindow::CreateWindow(FGLFWWindow* window, FWString&& title, const FWindowDefinition& def) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(window);

    // reset all hints before creating the new window
    glfwDefaultWindowHints();

    // don't initialize an OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* const mode = glfwGetVideoMode(monitor);

    int width = checked_cast<int>(def.Width);
    int height = checked_cast<int>(def.Height);

    if (def.AutoSize) {
        width = mode->width;
        height = mode->height;
    }

    if (def.Fullscreen) {
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

        glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
    }
    else {
        monitor = nullptr; // Disable fullscreen

        glfwWindowHint(GLFW_DECORATED, def.HasSystemMenu);
        glfwWindowHint(GLFW_RESIZABLE, def.HasResizeButton);
        glfwWindowHint(GLFW_MAXIMIZED, def.Maximized);
    }

    glfwWindowHint(GLFW_VISIBLE, not def.Invisible);

    GLFWwindow* const nativeHandle = glfwCreateWindow(
        width, height,
        WCHAR_TO_UTF_8(title.c_str()),
        monitor,
        nullptr );

    if (Unlikely(nativeHandle == nullptr))
        return false;

    window->SetNativeHandle(nativeHandle);
    glfwSetWindowUserPointer(nativeHandle, window);
    Assert_NoAssume(glfwGetWindowUserPointer(nativeHandle) == window);

    glfwSetWindowPosCallback(nativeHandle, &FWindowCallbacks::Position);
    glfwSetWindowSizeCallback(nativeHandle, &FWindowCallbacks::Size);
    glfwSetCursorEnterCallback(nativeHandle, &FWindowCallbacks::CursorEnter);
    glfwSetWindowFocusCallback(nativeHandle, &FWindowCallbacks::Focus);
    glfwSetWindowRefreshCallback(nativeHandle, &FWindowCallbacks::Refresh);

    if (def.AllowDragDrop)
        glfwSetDropCallback(nativeHandle, &FWindowCallbacks::Drop);

    if (def.Centered)
        window->Center();

    return true;
}
//----------------------------------------------------------------------------
void FGLFWWindow::SendEvent(const FGLFWMessageKeyboard& keyboard) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    _OnKeyboardEvent(*this, keyboard);
}
//----------------------------------------------------------------------------
void FGLFWWindow::SendEvent(const FGLFWMessageMouse& mouse) {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    _OnMouseEvent(*this, mouse);
}
//----------------------------------------------------------------------------
void FGLFWWindow::OnFocusSet() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    // track active window (tolerant)
    {
        const auto exclusive = ActiveWindow_().LockExclusive();
        const auto it = exclusive->Find(MakePtrRef(this));
        if (exclusive->end() != it)
            exclusive->Erase(it);

        exclusive->Push(MakePtrRef(this));
    }

    FGenericWindow::OnFocusSet();
}
//----------------------------------------------------------------------------
void FGLFWWindow::OnFocusLose() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    // track active window (tolerant)
    {
        const auto exclusive = ActiveWindow_().LockExclusive();
        const auto it = exclusive->Find(MakePtrRef(this));
        if (exclusive->end() != it)
            exclusive->Erase(it);
    }

    FGenericWindow::OnFocusLose();
}
//----------------------------------------------------------------------------
void FGLFWWindow::UpdateClientRect() {
    PPE_DATARACE_EXCLUSIVE_SCOPE(this);

    GLFWwindow* const nativeHandle = NativeHandle_(*this);

    int x, y;
    glfwGetWindowPos(nativeHandle, &x, &y);
    int w, h;
    glfwGetWindowSize(nativeHandle, &w, &h);

    FGenericWindow::Move(x, y);
    FGenericWindow::Resize(checked_cast<size_t>(w), checked_cast<size_t>(h));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_GLFW
