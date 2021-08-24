#pragma once

#include "HAL/Generic/GenericPlatformNotification.h"

#ifndef PLATFORM_GLFW
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGLFWPlatformNotification : FGenericPlatformNotification {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasSystray, true);
    STATIC_CONST_INTEGRAL(bool, HasTaskbar, true);

    static void ShowSystray();
    static void HideSystray();

    using ENotificationIcon = FGenericPlatformNotification::ENotificationIcon;

    static void NotifySystray(ENotificationIcon icon, const FWStringView& title, const FWStringView& text);

    using FSystrayDelegate = FGenericPlatformNotification::FSystrayDelegate;

    static size_t AddSystrayCommand(
        const FWStringView& category,
        const FWStringView& label,
        FSystrayDelegate&& cmd );
    static bool RemoveSystrayCommand(size_t index);

    using ETaskbarState = FGenericPlatformNotification::ETaskbarState;

    static void SetTaskbarState(ETaskbarState state);
    static void SetTaskbarProgress(size_t completed, size_t total);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE