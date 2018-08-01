#include "stdafx.h"

#include "GenericPlatformDialog.h"

#include "HAL/PlatformDialog.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
auto FGenericPlatformDialog::Notify(const FWStringView& text, const FWStringView& caption, EIcon iconType/* = EIcon::Information */) -> EResult {
    return FPlatformDialog::Show(text, caption, kNotify, iconType);
}
//----------------------------------------------------------------------------
auto FGenericPlatformDialog::OkCancel(const FWStringView& text, const FWStringView& caption, EIcon iconType/* = EIcon::Question */) -> EResult {
    return FPlatformDialog::Show(text, caption, kOkCancel, iconType);
}
//----------------------------------------------------------------------------
auto FGenericPlatformDialog::AbortRetryIgnore(const FWStringView& text, const FWStringView& caption, EIcon iconType/* = EIcon::Error */) -> EResult {
    return FPlatformDialog::Show(text, caption, kAbortRetryIgnore, iconType);
}
//----------------------------------------------------------------------------
auto FGenericPlatformDialog::YesNoCancel(const FWStringView& text, const FWStringView& caption, EIcon iconType/* = EIcon::Question */) -> EResult {
    return FPlatformDialog::Show(text, caption, kYesNoCancel, iconType);
}
//----------------------------------------------------------------------------
auto FGenericPlatformDialog::YesNo(const FWStringView& text, const FWStringView& caption, EIcon iconType/* = EIcon::Question */) -> EResult {
    return FPlatformDialog::Show(text, caption, kYesNo, iconType);
}
//----------------------------------------------------------------------------
auto FGenericPlatformDialog::RetryCancel(const FWStringView& text, const FWStringView& caption, EIcon iconType/* = EIcon::Exclamation */) -> EResult {
    return FPlatformDialog::Show(text, caption, kRetryCancel, iconType);
}
//----------------------------------------------------------------------------
auto FGenericPlatformDialog::CancelTryContinue(const FWStringView& text, const FWStringView& caption, EIcon iconType/* = EIcon::Exclamation */) -> EResult {
    return FPlatformDialog::Show(text, caption, kCancelTryContinue, iconType);
}
//----------------------------------------------------------------------------
auto FGenericPlatformDialog::IgnoreOnceAlwaysAbort(const FWStringView& text, const FWStringView& caption, EIcon iconType/* = EIcon::Error */) -> EResult {
    return FPlatformDialog::Show(text, caption, kIgnoreOnceAlwaysAbort, iconType);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
