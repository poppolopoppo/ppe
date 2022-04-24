#include "stdafx.h"

#ifdef PLATFORM_GLFW

#include "HAL/GLFW/GLFWPlatformKeyboard.h"

#include "Input/KeyboardKey.h"
#include "Input/KeyboardState.h"

#include "HAL/GLFW/GLFWPlatformIncludes.h"
#include "HAL/GLFW/GLFWPlatformMessageHandler.h"
#include "HAL/GLFW/GLFWWindow.h"

#include "Misc/Function.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGLFWWindow::FKeyboardCallbacks {
    static void Keyboard(GLFWwindow* nativeHandle, int key, int scancode, int action, int mods) {
        Assert(nativeHandle);
        Unused(scancode);
        Unused(mods); // #TODO

        auto* const window = static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(nativeHandle));
        Assert(window);

        FGLFWMessageKeyboard keyboard;

        switch (action) {
        case GLFW_PRESS:
            keyboard.Event = FGLFWMessageKeyboard::KeyDown;
            break;
        case GLFW_RELEASE:
            keyboard.Event = FGLFWMessageKeyboard::KeyUp;
            break;
        case GLFW_REPEAT:
            return; // skip repeat events

        default: AssertNotImplemented();
        }

        switch (key) {
    #define PPE_GLFW_KEYBOARDKEY_DEF(GLFW, EKey) \
        case GLFW: \
            keyboard.Key = EKeyboardKey::EKey; \
            break;
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_0, _0)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_1, _1)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_2, _2)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_3, _3)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_4, _4)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_5, _5)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_6, _6)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_7, _7)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_8, _8)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_9, _9)

        // Alpha

        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_A, A)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_B, B)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_C, C)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_D, D)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_E, E)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_F, F)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_G, G)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_H, H)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_I, I)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_J, J)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_K, K)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_L, L)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_M, M)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_N, N)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_O, O)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_P, P)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_Q, Q)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_R, R)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_S, S)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_T, T)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_U, U)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_V, V)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_W, W)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_X, X)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_Y, Y)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_Z, Z)

        // Numpad

        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_0, Numpad0)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_1, Numpad1)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_2, Numpad2)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_3, Numpad3)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_4, Numpad4)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_5, Numpad5)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_6, Numpad6)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_7, Numpad7)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_8, Numpad8)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_9, Numpad9)

        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_ADD, Add)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_SUBTRACT, Subtract)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_MULTIPLY, Multiply)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_KP_DIVIDE, Divide)

        // Function

        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_F1, F1)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_F2, F2)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_F3, F3)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_F4, F4)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_F5, F5)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_F6, F6)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_F7, F7)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_F8, F8)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_F9, F9)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_F10, F10)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_F11, F11)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_F12, F12)

        // Direction

        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_UP, Up)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_DOWN, Down)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_LEFT, Left)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_RIGHT, Right)

        // Specials

        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_ESCAPE, Escape)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_SPACE, Space)

        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_PAUSE, Pause)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_PRINT_SCREEN, PrintScreen)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_SCROLL_LOCK, ScrollLock)

        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_BACKSPACE, Backspace)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_ENTER, Enter)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_TAB, Tab)

        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_HOME, Home)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_END, End)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_INSERT, Insert)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_DELETE, Delete)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_PAGE_UP, PageUp)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_PAGE_DOWN, PageDown)

        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_LEFT_ALT, Alt)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_RIGHT_ALT, Alt)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_MENU, Menu)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_LEFT_CONTROL, Control)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_RIGHT_CONTROL, Control)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_LEFT_SHIFT, Shift)
        PPE_GLFW_KEYBOARDKEY_DEF(GLFW_KEY_RIGHT_SHIFT, Shift)

    #undef PPE_GLFW_KEYBOARDKEY_DEF

        default: return; // skip unknown keys
        }

        window->SendEvent(keyboard);
    }
    static void OnMessage(FGLFWWindow& , const FGLFWMessageKeyboard& event, FKeyboardState* keyboard) {
        Assert(keyboard);

        switch(event.Event) {
        case FGLFWMessageKeyboard::KeyDown:
            keyboard->SetKeyDown(event.Key);
            break;
        case FGLFWMessageKeyboard::KeyUp:
            keyboard->SetKeyUp(event.Key);
            break;
        }
    }
};
//----------------------------------------------------------------------------
FEventHandle FGLFWPlatformKeyboard::SetupMessageHandler(FGLFWWindow& window, FKeyboardState* keyboard) {
    Assert(window.NativeHandle());
    Assert(keyboard);

    if (window.OnKeyboardEvent().empty()) {
        Verify(not glfwSetKeyCallback(
            static_cast<GLFWwindow*>(window.NativeHandle()),
            &FGLFWWindow::FKeyboardCallbacks::Keyboard ));
    }

    return window.OnKeyboardEvent().Add(FGLFWMessageKeyboardEvent::Bind<&FGLFWWindow::FKeyboardCallbacks::OnMessage>(keyboard));
}
//----------------------------------------------------------------------------
void FGLFWPlatformKeyboard::RemoveMessageHandler(FGLFWWindow& window, FEventHandle& handle) {
    Assert(window.NativeHandle());
    Assert(handle);

    window.OnKeyboardEvent().Remove(handle);

    if (window.OnKeyboardEvent().empty()) {
        Verify(&FGLFWWindow::FKeyboardCallbacks::Keyboard == glfwSetKeyCallback(
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
