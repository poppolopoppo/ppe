// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#ifdef PLATFORM_GLFW

#include "HAL/GLFW/GLFWPlatformSurvey.h"

#include "HAL/GLFW/GLFWPlatformIncludes.h"
#include "HAL/GLFW/GLFWWindow.h"
#include "HAL/TargetPlatform.h"

#include "Diagnostic/Logger.h"
#include "IO/StaticString.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void MonitorInfo_(FGLFWPlatformSurvey::FMonitorInfo* monitor, GLFWmonitor* nativeMonitor) {
    const GLFWvidmode* const mode = glfwGetVideoMode(nativeMonitor);
    monitor->CurrentResolution.Width = checked_cast<u32>(mode->width);
    monitor->CurrentResolution.Height = checked_cast<u32>(mode->height);
    monitor->CurrentResolution.BitsPerPixel = checked_cast<u32>(
        mode->redBits + mode->greenBits + mode->blueBits );
    monitor->CurrentResolution.RefreshRate = checked_cast<u32>(mode->refreshRate);

    monitor->MonitorName = UTF_8_TO_WCHAR(glfwGetMonitorName(nativeMonitor));
    glfwGetMonitorPos(nativeMonitor, &monitor->ScreenX, &monitor->ScreenY);

    monitor->SafeWidth = monitor->CurrentResolution.Width;
    monitor->SafeHeight = monitor->CurrentResolution.Height;
    monitor->SafeX = 0;
    monitor->SafeY = 0;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FGLFWPlatformSurvey::PrimaryDisplayAdapter(FDisplayAdapter* displayAdapter) {
    Unused(displayAdapter);
    LOG_UNSUPPORTED_FUNCTION(HAL);
    return false;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformSurvey::DisplayAdapters(FDisplayAdapters* displayAdapters) {
    Unused(displayAdapters);
    LOG_UNSUPPORTED_FUNCTION(HAL);
    return false;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformSurvey::PrimaryMonitor(FMonitorInfo* monitor) {
    if (GLFWmonitor* const nativeMonitor = glfwGetPrimaryMonitor()) {
        MonitorInfo_(monitor, nativeMonitor);
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformSurvey::MonitorFromPoint(int x, int y, FMonitorInfo* monitor) {
    Unused(x);
    Unused(y);
    Unused(monitor);
    LOG_UNSUPPORTED_FUNCTION(HAL);
    return false;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformSurvey::MonitorFromCursor(FMonitorInfo* monitor) {
    Unused(monitor);
    LOG_UNSUPPORTED_FUNCTION(HAL);
    return false;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformSurvey::MonitorFromWindow(const FGLFWWindow& window, FMonitorInfo* monitor) {
    if (auto* const nativeWindow = static_cast<GLFWwindow*>(window.NativeHandle())) {
        MonitorInfo_(monitor, glfwGetWindowMonitor(nativeWindow));
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformSurvey::MonitorInfos(FMonitorInfos* monitors) {
    int count;
    if (GLFWmonitor** const nativeMonitors = glfwGetMonitors(&count)) {
        forrange(i, 0, count)
            MonitorInfo_(monitors->push_back_Uninitialized(), nativeMonitors[i]);
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformSurvey::ApplicationStorage(FStorageInfo* storage) {
    Unused(storage);
    LOG_UNSUPPORTED_FUNCTION(HAL);
    return false;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformSurvey::SystemStorage(FStorageInfo* storage) {
    Unused(storage);
    LOG_UNSUPPORTED_FUNCTION(HAL);
    return false;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformSurvey::StorageFromPath(const FWStringView& path, FStorageInfo* storage) {
    Unused(path);
    Unused(storage);
    LOG_UNSUPPORTED_FUNCTION(HAL);
    return false;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformSurvey::StorageInfos(FStorageInfos* storages) {
    Unused(storages);
    LOG_UNSUPPORTED_FUNCTION(HAL);
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_GLFW
