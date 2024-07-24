// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
    PPE_LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
void FGLFWPlatformNotification::HideSystray() {
    PPE_LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
void FGLFWPlatformNotification::NotifySystray(ENotificationIcon icon, const FWStringView& title, const FWStringView& text) {
    Unused(icon);
    Unused(title);
    Unused(text);
    PPE_LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
size_t FGLFWPlatformNotification::AddSystrayCommand(
    const FWStringView& category,
    const FWStringView& label,
    FSystrayDelegate&& cmd ) {
    Unused(category);
    Unused(label);
    Unused(cmd);
    PPE_LOG_UNSUPPORTED_FUNCTION(HAL);
    return INDEX_NONE;
}
//----------------------------------------------------------------------------
bool FGLFWPlatformNotification::RemoveSystrayCommand(size_t index) {
    Unused(index);
    PPE_LOG_UNSUPPORTED_FUNCTION(HAL);
    return false;
}
//----------------------------------------------------------------------------
void FGLFWPlatformNotification::SetTaskbarState(ETaskbarState state) {
    Unused(state);
    PPE_LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
void FGLFWPlatformNotification::SetTaskbarProgress(size_t completed, size_t total) {
    Unused(completed);
    Unused(total);
    PPE_LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_GLFW
