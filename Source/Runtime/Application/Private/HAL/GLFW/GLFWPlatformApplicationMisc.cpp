#include "stdafx.h"

#ifdef PLATFORM_GLFW

#include "HAL/GLFW/GLFWPlatformApplicationMisc.h"

#include "Color/Color.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformIncludes.h"
#include "HAL/PlatformProcess.h"
#include "HAL/GLFW/GLFWPlatformNotification.h"
#include "HAL/GLFW/GLFWWindow.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATION_API, Application)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FGLFWPlatformApplicationMisc::Start() {

}
//----------------------------------------------------------------------------
void FGLFWPlatformApplicationMisc::Shutdown() {

}
//----------------------------------------------------------------------------
bool FGLFWPlatformApplicationMisc::PickScreenColorAt(int x, int y, FColor* color) {
    Unused(x);
    Unused(y);
    Unused(color);
    return false;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformApplicationMisc::PickScreenColorUnderMouse(FColor* color) {
    Unused(color);
    return false;
}
//----------------------------------------------------------------------------
void FGLFWPlatformApplicationMisc::PreventScreenSaver() {

}
//----------------------------------------------------------------------------
bool FGLFWPlatformApplicationMisc::SetHighDPIAwareness() {
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_GLFW
