#include "stdafx.h"

#ifdef PLATFORM_GLFW

#include "HAL/GLFW/GLFWPlatformNotification.h"

#include "HAL/TargetPlatform.h"

#include "Diagnostic/Logger.h"

// #TODO: GLFW won't handle popups or message-boxes, it should be implemented separately

namespace PPE {
namespace Application {
LOG_CATEGORY(, Notification)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FGLFWPlatformNotification::ShowSystray() {
    LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
void FGLFWPlatformNotification::HideSystray() {
    LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
void FGLFWPlatformNotification::NotifySystray(ENotificationIcon icon, const FWStringView& title, const FWStringView& text) {
    UNUSED(icon);
    UNUSED(title);
    UNUSED(text);
    LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
size_t FGLFWPlatformNotification::AddSystrayCommand(
    const FWStringView& category,
    const FWStringView& label,
    FSystrayDelegate&& cmd ) {
    UNUSED(category);
    UNUSED(label);
    UNUSED(cmd);
    LOG_UNSUPPORTED_FUNCTION(HAL);
    return INDEX_NONE;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformNotification::RemoveSystrayCommand(size_t index) {
    UNUSED(index);
    LOG_UNSUPPORTED_FUNCTION(HAL);
    return false;
}
//----------------------------------------------------------------------------
void FGLFWPlatformNotification::SetTaskbarState(ETaskbarState state) {
    UNUSED(state);
    LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
void FGLFWPlatformNotification::SetTaskbarProgress(size_t completed, size_t total) {
    UNUSED(completed);
    UNUSED(total);
    LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_GLFW
