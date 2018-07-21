#pragma once

#include "Core/HAL/Generic/GenericPlatformDialog.h"

#ifdef PLATFORM_WINDOWS

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct CORE_API FWindowsPlatformDialog : FGenericPlatformDialog {
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
} //!namespace Core

#endif //!PLATFORM_WINDOWS
