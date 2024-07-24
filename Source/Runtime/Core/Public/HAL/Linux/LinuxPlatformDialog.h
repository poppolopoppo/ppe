#pragma once

#include "HAL/Generic/GenericPlatformDialog.h"

#ifdef PLATFORM_LINUX

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FLinuxPlatformDialog : FGenericPlatformDialog {
public:
    STATIC_CONST_INTEGRAL(bool, HasDialogBox, true);

    using FGenericPlatformDialog::EIcon;
    using FGenericPlatformDialog::EResult;
    using FGenericPlatformDialog::EType;

    static EResult Show(const FWStringView& text, const FWStringView& caption, EType dialogType, EIcon iconType);

    static bool PushSplashScreen_ReturnIfOpened() {
        NOOP();
        return false;
    }
    static bool PopSplashScreen_ReturnIfOpened() {
        NOOP();
        return false;
    }

    static FDialogHandle BeginProgress(const FWStringView& text, size_t total = 0) {
        Unused(text, total);
        return 0;
    }
    static void IncProgressPos(FDialogHandle progress) {
        Unused(progress);
    }
    static void SetProgressPos(FDialogHandle progress, size_t amount) {
        Unused(progress, amount);
    }
    static void SetProgressText(FDialogHandle progress, const FWStringView& message) {
        Unused(progress, message);
    }
    static void EndProgress(FDialogHandle progress) {
        Unused(progress);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
