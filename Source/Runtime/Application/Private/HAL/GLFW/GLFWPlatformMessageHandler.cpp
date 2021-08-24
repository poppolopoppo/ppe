#include "stdafx.h"

#ifdef PLATFORM_GLFW

#include "HAL/GLFW/GLFWPlatformMessageHandler.h"

#include "HAL/GLFW/GLFWPlatformIncludes.h"
#include "HAL/GLFW/GLFWWindow.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FGLFWPlatformMessageHandler::PumpGlobalMessages() {
    glfwPollEvents();
    return true;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformMessageHandler::PumpMessages(FGLFWWindow& window) {
    if (GLFWwindow* const glfwWindow = static_cast<GLFWwindow*>(window.NativeHandle())) {
        if (glfwWindowShouldClose(glfwWindow))
            return false;

        glfwPollEvents();
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_GLFW
