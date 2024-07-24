#pragma once

#include "HAL/TargetPlatform.h"

#include "IO/String_fwd.h"

namespace PPE
{
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformDialog
{
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasDialogBox, false);

    enum EIcon
    {
        Hand = 0,
        Question,
        Exclamation,
        Aterisk,
        Error,
        Warning,
        Information,
    };

    enum EResult : u32
    {
        None = 0,
        Ok = 1 << 0,
        Cancel = 1 << 1,
        Abort = 1 << 2,
        Retry = 1 << 3,
        Ignore = 1 << 4,
        Yes = 1 << 5,
        No = 1 << 6,
        TryAgain = 1 << 7,
        Continue = 1 << 8,
        IgnoreAlways = 1 << 9,
    };
    ENUM_FLAGS_FRIEND(EResult);

    enum EType : u32
    {
        kNotify = u32(EResult::Ok),
        kOkCancel = u32(EResult::Ok) | u32(EResult::Cancel),
        kAbortRetryIgnore = u32(EResult::Abort) | u32(EResult::Retry) | u32(EResult::Ignore),
        kYesNoCancel = u32(EResult::Yes) | u32(EResult::No) | u32(EResult::Cancel),
        kYesNo = u32(EResult::Yes) | u32(EResult::No),
        kRetryCancel = u32(EResult::Retry) | u32(EResult::Cancel),
        kCancelTryContinue = u32(EResult::Cancel) | u32(EResult::TryAgain) | u32(EResult::Continue),
        kIgnoreOnceAlwaysAbort = u32(EResult::Ignore) | u32(EResult::IgnoreAlways) | u32(EResult::Abort),
        kIgnoreOnceAlwaysAbortRetry = u32(kIgnoreOnceAlwaysAbort) | u32(EResult::Retry),
    };

    using FDialogHandle = uintptr_t;

    static EResult Show(const FWStringView& text, const FWStringView& caption, EType dialogType, EIcon iconType) = delete;
    static EResult FileDialog(const FConstWChar& sourceFile, u32 sourceLine, const FWStringView& text, const FWStringView& caption, EType dialogType, EIcon iconType) = delete;

    static bool PushSplashScreen_ReturnIfOpened() = delete;
    static bool PopSplashScreen_ReturnIfOpened() = delete;

    static FDialogHandle BeginProgress(const FWStringView& text, size_t total = 0) = delete;
    static void IncProgressPos(FDialogHandle progress) = delete;
    static void SetProgressPos(FDialogHandle progress, size_t amount) = delete;
    static void SetProgressText(FDialogHandle progress, const FWStringView& message) = delete;
    static void EndProgress(FDialogHandle progress) = delete;

public: // generic helpers
    static EResult Notify(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Information);
    static EResult OkCancel(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Question);
    static EResult AbortRetryIgnore(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Error);
    static EResult YesNoCancel(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Question);
    static EResult YesNo(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Question);
    static EResult RetryCancel(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Exclamation);
    static EResult CancelTryContinue(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Exclamation);
    static EResult IgnoreOnceAlwaysAbort(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Error);
    static EResult IgnoreOnceAlwaysAbortRetry(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Error);
};
//----------------------------------------------------------------------------
NODISCARD inline CONSTEXPR bool operator &(FGenericPlatformDialog::EType type, FGenericPlatformDialog::EResult result) {
    return Meta::EnumHas(type, u32(result));
}
//----------------------------------------------------------------------------
NODISCARD inline CONSTEXPR bool operator &(FGenericPlatformDialog::EResult result, FGenericPlatformDialog::EType type) {
    return Meta::EnumHas(result, u32(type));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} // namespace PPE
