// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#ifdef PLATFORM_GLFW

#include "HAL/GLFW/GLFWPlatformLaunch.h"

#include "HAL/GLFW/GLFWPlatformIncludes.h"

#include "Diagnostic/Logger.h"
#include "IO/StaticString.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_LOGGER
LOG_CATEGORY(, GLFW);
static void GLFW_error_callback_(int error, const char* description) {
    LOG(GLFW, Error, L"{0} (code: {1})", UTF_8_TO_WCHAR(description), error);
}
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FGLFWPlatformLaunch::OnPlatformLaunch(const wchar_t* filename, size_t argc, const wchar_t* const* argv) {
    FGenericPlatformLaunch::OnPlatformLaunch(nullptr, 0, filename, argc, argv);

    VerifyRelease(glfwInit());

#if USE_PPE_LOGGER
    glfwSetErrorCallback(&GLFW_error_callback_);
#endif
}
//----------------------------------------------------------------------------
void FGLFWPlatformLaunch::OnPlatformShutdown() {
    glfwTerminate();

    FGenericPlatformLaunch::OnPlatformShutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_GLFW
