#pragma once

#include "HAL/Generic/GenericPlatformDialog.h"

#ifdef PLATFORM_WINDOWS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FWindowsPlatformDialog : FGenericPlatformDialog {
public:
    STATIC_CONST_INTEGRAL(bool, HasDialogBox, true);

    using FGenericPlatformDialog::EIcon;
    using FGenericPlatformDialog::EResult;
    using FGenericPlatformDialog::EType;
    using FGenericPlatformDialog::FDialogHandle;

    PPE_CORE_API static EResult Show(const FWStringView& text, const FWStringView& caption, EType dialogType, EIcon iconType);

    PPE_CORE_API static EResult FileDialog(const FConstWChar& sourceFile, u32 sourceLine, const FWStringView& text, const FWStringView& caption, EType dialogType, EIcon iconType);

    PPE_CORE_API static bool PushSplashScreen_ReturnIfOpened();
    PPE_CORE_API static bool PopSplashScreen_ReturnIfOpened();

    PPE_CORE_API static FDialogHandle BeginProgress(const FWStringView& text, size_t total = 0);
    PPE_CORE_API static void IncProgressPos(FDialogHandle progress);
    PPE_CORE_API static void SetProgressPos(FDialogHandle progress, size_t amount);
    PPE_CORE_API static void SetProgressText(FDialogHandle progress, const FWStringView& message);
    PPE_CORE_API static void EndProgress(FDialogHandle progress);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
