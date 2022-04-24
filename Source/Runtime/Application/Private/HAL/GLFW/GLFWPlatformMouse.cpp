#include "stdafx.h"

#ifdef PLATFORM_GLFW

#include "HAL/GLFW/GLFWPlatformMouse.h"

#include "HAL/GLFW/GLFWPlatformIncludes.h"
#include "HAL/GLFW/GLFWWindow.h"
#include "HAL/TargetPlatform.h"

#include "Container/AssociativeVector.h"
#include "Diagnostic/Logger.h"
#include "Input/MouseState.h"
#include "Maths/MathHelpers.h"
#include "Misc/Function.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static GLFWcursor* StandardCursor_(EGLFWCursorType shape) {
    struct FGLFWCustorHolder_ {
        GLFWcursor* Cursor;
        FGLFWCustorHolder_(GLFWcursor* cursor)
        :   Cursor(cursor) {
            Assert(Cursor);
        }
        ~FGLFWCustorHolder_() {
            glfwDestroyCursor(Cursor);
        }
    };
    using FCustorCache = TThreadSafe<
        ASSOCIATIVE_VECTORINSITU(Window, EGLFWCursorType, FGLFWCustorHolder_, 8),
        EThreadBarrier::CriticalSection >;
    ONE_TIME_DEFAULT_INITIALIZE(FCustorCache, GCursorCache);

    const auto exclusive = GCursorCache.LockExclusive();
    auto it = exclusive->find(shape);

    if (exclusive->end() != it)
        return it->second.Cursor;

    GLFWcursor* const cursor = glfwCreateStandardCursor(static_cast<int>(shape));
    exclusive->insert({ shape, cursor });
    return cursor;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGLFWWindow::FMouseCallbacks {
    static void Button(GLFWwindow* nativeHandle, int button, int action, int mods) {
        Assert(nativeHandle);
        Unused(mods); // #TODO

        auto* const window = static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(nativeHandle));
        Assert(window);

        FGLFWMessageMouse mouse;

        switch (action) {
        case GLFW_PRESS:
            mouse.Event = FGLFWMessageMouse::ButtonDown;
            break;
        case GLFW_RELEASE:
            mouse.Event = FGLFWMessageMouse::ButtonUp;
            break;

        default: AssertNotImplemented();
        }

        switch (button) {
        case GLFW_MOUSE_BUTTON_1:
            mouse.Button = EMouseButton::Button0;
            break;
        case GLFW_MOUSE_BUTTON_2:
            mouse.Button = EMouseButton::Button1;
            break;
        case GLFW_MOUSE_BUTTON_3:
            mouse.Button = EMouseButton::Button2;
            break;
        case GLFW_MOUSE_BUTTON_4:
            mouse.Button = EMouseButton::Thumb0;
            break;
        case GLFW_MOUSE_BUTTON_5:
            mouse.Button = EMouseButton::Thumb1;
            break;

        default: return; // skip unknown buttons
        }

        mouse.Coords = double2::Zero;
        return window->SendEvent(mouse);
    }
    static void CursorPos(GLFWwindow* nativeHandle, double xpos, double ypos) {
        Assert(nativeHandle);

        auto* const window = static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(nativeHandle));
        Assert(window);

        FGLFWMessageMouse mouse;
        mouse.Event = FGLFWMessageMouse::CursorMove;
        mouse.Button = Default;
        mouse.Coords = { xpos, ypos };

        window->SendEvent(mouse);
    }
    static void Scroll(GLFWwindow* nativeHandle, double xoffset, double yoffset) {
        Assert(nativeHandle);

        auto* const window = static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(nativeHandle));
        Assert(window);

        FGLFWMessageMouse mouse;
        mouse.Event = FGLFWMessageMouse::Scroll;
        mouse.Button = Default;
        mouse.Coords = { xoffset, yoffset };

        window->SendEvent(mouse);
    }
    static void OnMessage(FGLFWWindow& window, const FGLFWMessageMouse& event, FMouseState* mouse) {
        Assert(mouse);

        switch(event.Event) {
        case FGLFWMessageMouse::CursorMove:
            mouse->SetPosition(
                FPlatformMaths::RoundToInt(window.Left() + event.Coords.x),
                FPlatformMaths::RoundToInt(window.Top() + event.Coords.y),
                FPlatformMaths::RoundToInt(event.Coords.x),
                FPlatformMaths::RoundToInt(event.Coords.y),
                static_cast<float>(LinearStep(event.Coords[0], 0, static_cast<double>(window.Width()))),
                static_cast<float>(LinearStep(event.Coords[1], 0, static_cast<double>(window.Height()))) );
            window.OnMouseMove(mouse->ClientX(), mouse->ClientY());
            break;
        case FGLFWMessageMouse::ButtonDown:
            mouse->SetButtonDown(event.Button);
            break;
        case FGLFWMessageMouse::ButtonUp:
            mouse->SetButtonUp(event.Button);
            window.OnMouseClick(mouse->ClientX(), mouse->ClientY(), event.Button);
            break;
        case FGLFWMessageMouse::Scroll:
            mouse->SetWheelDelta(FPlatformMaths::RoundToInt(static_cast<float>(event.Coords.x)));
            window.OnMouseWheel(mouse->ClientX(), mouse->ClientY(), mouse->WheelDelta().Raw());
            break;
        }
    }
};
//----------------------------------------------------------------------------
auto FGLFWPlatformMouse::CursorType() -> ECursorType {
    LOG_UNSUPPORTED_FUNCTION(HAL);
    return Default; // #TODO: track cursor type inside active FGLFWWindow
}
//----------------------------------------------------------------------------
auto FGLFWPlatformMouse::SetCursorType(ECursorType type) -> ECursorType {
    ECursorType prev = CursorType();
    if (type == ECursorType::_GLFW_DEFAULT_CURSOR)
        ResetCursorType();
    else
        glfwSetCursor(
            static_cast<GLFWwindow*>(FGLFWWindow::ActiveNativeHandle()),
            StandardCursor_(type) );
    return prev;
}
//----------------------------------------------------------------------------
void FGLFWPlatformMouse::ResetCursorType() {
    glfwSetCursor(
        static_cast<GLFWwindow*>(FGLFWWindow::ActiveNativeHandle()),
        nullptr );
}
//----------------------------------------------------------------------------
bool FGLFWPlatformMouse::Visible() {
    return (glfwGetInputMode(
        static_cast<GLFWwindow*>(FGLFWWindow::ActiveNativeHandle()),
        GLFW_CURSOR ) != GLFW_CURSOR_NORMAL );
}
//----------------------------------------------------------------------------
bool FGLFWPlatformMouse::SetVisible(bool value) {
    glfwSetInputMode(
        static_cast<GLFWwindow*>(FGLFWWindow::ActiveNativeHandle()),
        GLFW_CURSOR,
        value ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN );
    return true;
}
//----------------------------------------------------------------------------
void FGLFWPlatformMouse::ResetCapture() {
    LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
void FGLFWPlatformMouse::SetCapture(const FGLFWWindow& window) {
    Unused(window);
    LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
bool FGLFWPlatformMouse::ClientToScreen(const FGLFWWindow& window, int* x, int *y) {
    *x += window.Left();
    *y += window.Height();
    return true;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformMouse::ScreenToClient(const FGLFWWindow& window, int* x, int *y) {
    *x -= window.Left();
    *y -= window.Height();
    return true;
}
//----------------------------------------------------------------------------
void FGLFWPlatformMouse::CenterCursorOnWindow(const FGLFWWindow& window) {
    auto* const nativeHandle = static_cast<GLFWwindow*>(window.NativeHandle());

    const double clientX = (window.Width() / 2.0);
    const double clientY = (window.Height() / 2.0);
    glfwSetCursorPos(nativeHandle, clientX, clientY);
}
//----------------------------------------------------------------------------
FEventHandle FGLFWPlatformMouse::SetupMessageHandler(FGLFWWindow& window, FMouseState* mouse) {
    Assert(window.NativeHandle());
    Assert(mouse);

    if (window.OnMouseEvent().empty()) {
        Verify(not glfwSetMouseButtonCallback(
            static_cast<GLFWwindow*>(window.NativeHandle()),
            &FGLFWWindow::FMouseCallbacks::Button ));

        Verify(not glfwSetCursorPosCallback(
            static_cast<GLFWwindow*>(window.NativeHandle()),
            &FGLFWWindow::FMouseCallbacks::CursorPos ));

        Verify(not glfwSetScrollCallback(
            static_cast<GLFWwindow*>(window.NativeHandle()),
            &FGLFWWindow::FMouseCallbacks::Scroll ));
    }

    return window.OnMouseEvent().Add(FGLFWMessageMouseEvent::Bind<&FGLFWWindow::FMouseCallbacks::OnMessage>(mouse));
}
//----------------------------------------------------------------------------
void FGLFWPlatformMouse::RemoveMessageHandler(FGLFWWindow& window, FEventHandle& handle) {
    Assert(window.NativeHandle());
    Assert(handle);

    window.OnMouseEvent().Remove(handle);

    if (window.OnMouseEvent().empty()) {
        Verify(&FGLFWWindow::FMouseCallbacks::Button == glfwSetMouseButtonCallback(
            static_cast<GLFWwindow*>(window.NativeHandle()),
            nullptr ));

        Verify(&FGLFWWindow::FMouseCallbacks::CursorPos == glfwSetCursorPosCallback(
            static_cast<GLFWwindow*>(window.NativeHandle()),
            nullptr ));

        Verify(&FGLFWWindow::FMouseCallbacks::Scroll == glfwSetScrollCallback(
            static_cast<GLFWwindow*>(window.NativeHandle()),
            nullptr ));
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_GLFW
