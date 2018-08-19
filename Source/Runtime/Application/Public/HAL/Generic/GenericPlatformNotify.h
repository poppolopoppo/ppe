#pragma once

#include "Application_fwd.h"

#include "Misc/Function.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericTaskbarState {
    Normal,
    Progress,
    Paused,
    Error,
    Indeterminate,
};
//----------------------------------------------------------------------------
// https://docs.microsoft.com/fr-fr/windows/desktop/shell/taskbar %NOCOMMIT%
// https://github.com/pauldotknopf/WindowsSDK7-Samples/blob/master/winui/shell/appshellintegration/TaskbarThumbnailToolbar/ThumbnailToolbar.cpp %NOCOMMIT%
struct PPE_APPLICATION_API FGenericPlatformNotify {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasSystray, false);
    STATIC_CONST_INTEGRAL(bool, HasTaskbar, false);

    static void ShowSystray() = delete;
    static void HideSystray() = delete;

    static void SystrayNotify(const FWStringView& text) = delete;

    using FSystrayDelegate = TFunction<void()>;

    static size_t SystrayAddCommand(const FWStringView& label, FSystrayDelegate&& cmd) = 0;
    static bool SystrayRemoveCommand(size_t index) = 0;

    using ETaskbarState = EGenericTaskbarStatus;

    static void SetTaskbarState(ETaskbarState state) = delete;
    static void SetTaskbarProgress(size_t completed, size_t total) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE