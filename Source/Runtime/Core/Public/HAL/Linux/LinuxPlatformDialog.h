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

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
