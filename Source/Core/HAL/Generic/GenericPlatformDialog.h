#pragma once

#include "Core/HAL/TargetPlatform.h"
#include "Core/IO/String_fwd.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct CORE_API FGenericPlatformDialog {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasDialogBox, false);

    enum EIcon {
        Hand                    = 0,
        Question,
        Exclamation,
        Aterisk,
        Error,
        Warning,
        Information,
    };

    enum EResult {
        None                    = 0,
        Ok                      = 1<<0,
        Cancel                  = 1<<1,
        Abort                   = 1<<2,
        Retry                   = 1<<3,
        Ignore                  = 1<<4,
        Yes                     = 1<<5,
        No                      = 1<<6,
        TryAgain                = 1<<7,
        Continue                = 1<<8,
        IgnoreAlways            = 1<<9,
    };
    ENUM_FLAGS_FRIEND(EResult);

    enum EType {
        kNotify                 = size_t(EResult::Ok),
        kOkCancel               = size_t(EResult::Ok) | size_t(EResult::Cancel),
        kAbortRetryIgnore       = size_t(EResult::Abort) | size_t(EResult::Retry) | size_t(EResult::Ignore),
        kYesNoCancel            = size_t(EResult::Yes) | size_t(EResult::No) | size_t(EResult::Cancel),
        kYesNo                  = size_t(EResult::Yes) | size_t(EResult::No),
        kRetryCancel            = size_t(EResult::Retry) | size_t(EResult::Cancel),
        kCancelTryContinue      = size_t(EResult::Cancel) | size_t(EResult::TryAgain) | size_t(EResult::Continue),
        kIgnoreOnceAlwaysAbort  = size_t(EResult::Ignore) | size_t(EResult::IgnoreAlways) | size_t(EResult::Abort),
    };

    static EResult Show(const FWStringView& text, const FWStringView& caption, EType dialogType, EIcon iconType) = delete;

public: // generic helpers

    static EResult Notify(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Information);
    static EResult OkCancel(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Question);
    static EResult AbortRetryIgnore(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Error);
    static EResult YesNoCancel(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Question);
    static EResult YesNo(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Question);
    static EResult RetryCancel(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Exclamation);
    static EResult CancelTryContinue(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Exclamation);
    static EResult IgnoreOnceAlwaysAbort(const FWStringView& text, const FWStringView& caption, EIcon iconType = EIcon::Error);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
