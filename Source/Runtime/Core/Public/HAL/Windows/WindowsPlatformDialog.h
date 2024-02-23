#pragma once

#include "HAL/Generic/GenericPlatformDialog.h"

#ifdef PLATFORM_WINDOWS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformDialog : FGenericPlatformDialog {
public:
    STATIC_CONST_INTEGRAL(bool, HasDialogBox, true);

    using FGenericPlatformDialog::EIcon;
    using FGenericPlatformDialog::EResult;
    using FGenericPlatformDialog::EType;

    static EResult Show(const FWStringView& text, const FWStringView& caption, EType dialogType, EIcon iconType);
    static EResult FileDialog(const FConstWChar& sourceFile, u32 sourceLine, const FWStringView& text, const FWStringView& caption, EType dialogType, EIcon iconType);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
