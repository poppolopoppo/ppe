#pragma once

#include "Application_fwd.h"

#include "Misc/Function.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericNotificationIcon {
    None,
    Info,
    Warning,
    Error,
};
//----------------------------------------------------------------------------
enum class EGenericTaskbarState {
    Normal,
    Progress,
    NoProgress,
    Paused,
    Error,
    Indeterminate,
};
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericPlatformNotification {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasSystray, false);
    STATIC_CONST_INTEGRAL(bool, HasTaskbar, false);

    static void ShowSystray() = delete;
    static void HideSystray() = delete;

    using ENotificationIcon = EGenericNotificationIcon;

    static void NotifySystray(ENotificationIcon icon, const FWStringView& title, const FWStringView& text) = delete;

    using FSystrayDelegate = TFunction<void()>;

    static size_t AddSystrayCommand(
        const FWStringView& category,
        const FWStringView& label,
        FSystrayDelegate&& cmd ) = delete;
    static bool RemoveSystrayCommand(size_t index) = delete;

    using ETaskbarState = EGenericTaskbarState;

    static void SetTaskbarState(ETaskbarState state) = delete;
    static void SetTaskbarProgress(size_t completed, size_t total) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE