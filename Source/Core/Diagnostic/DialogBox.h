#pragma once

#include "Core/Core.h"

#include "Core/IO/Format.h"
#include "Core/IO/StringBuilder.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Dialog {
    enum class Icon {
        Hand                = 0,
        Question            ,
        Exclamation         ,
        Aterisk             ,
        Error               ,
        Warning             ,
        Information         ,
    };

    enum class EResult {
        None                = 0,
        Ok                  ,
        Cancel              ,
        Abort               ,
        Retry               ,
        Ignore              ,
        Yes                 ,
        No                  ,
        TryAgain            ,
        Continue            ,
        IgnoreAlways        ,
    };

    enum class EType {
        Ok                      = (1<<(size_t)EResult::Ok),
        OkCancel                = (1<<(size_t)EResult::Ok)|(1<<(size_t)EResult::Cancel),
        AbortRetryIgnore        = (1<<(size_t)EResult::Abort)|(1<<(size_t)EResult::Retry)|(1<<(size_t)EResult::Ignore),
        YesNoCancel             = (1<<(size_t)EResult::Yes)|(1<<(size_t)EResult::No)|(1<<(size_t)EResult::Cancel),
        YesNo                   = (1<<(size_t)EResult::Yes)|(1<<(size_t)EResult::No),
        RetryCancel             = (1<<(size_t)EResult::Retry)|(1<<(size_t)EResult::Cancel),
        CancelTryContinue       = (1<<(size_t)EResult::Cancel)|(1<<(size_t)EResult::TryAgain)|(1<<(size_t)EResult::Continue),
        IgnoreOnceAlwaysAbort   = (1<<(size_t)EResult::Ignore)|(1<<(size_t)EResult::IgnoreAlways)|(1<<(size_t)EResult::Abort),
    };

    EResult Show(const FWStringView& text, const FWStringView& caption, EType dialogType, Icon iconType);

    inline EResult Ok(const FWStringView& text, const FWStringView& caption, Icon iconType = Icon::Information) { return Show(text, caption, EType::Ok, iconType); }
    inline EResult OkCancel(const FWStringView& text, const FWStringView& caption, Icon iconType = Icon::Question) { return Show(text, caption, EType::OkCancel, iconType); }
    inline EResult AbortRetryIgnore(const FWStringView& text, const FWStringView& caption, Icon iconType = Icon::Error) { return Show(text, caption, EType::AbortRetryIgnore, iconType); }
    inline EResult YesNoCancel(const FWStringView& text, const FWStringView& caption, Icon iconType = Icon::Question) { return Show(text, caption, EType::YesNoCancel, iconType); }
    inline EResult YesNo(const FWStringView& text, const FWStringView& caption, Icon iconType = Icon::Question) { return Show(text, caption, EType::YesNo, iconType); }
    inline EResult RetryCancel(const FWStringView& text, const FWStringView& caption, Icon iconType = Icon::Exclamation) { return Show(text, caption, EType::RetryCancel, iconType); }
    inline EResult CancelTryContinue(const FWStringView& text, const FWStringView& caption, Icon iconType = Icon::Exclamation) { return Show(text, caption, EType::CancelTryContinue, iconType); }
    inline EResult IgnoreOnceAlwaysAbort(const FWStringView& text, const FWStringView& caption, Icon iconType = Icon::Error) { return Show(text, caption, EType::IgnoreOnceAlwaysAbort, iconType); }

    template <typename _Arg0, typename... _Args>
    EResult Show(const FWStringView& caption, EType dialogType, Icon iconType, const wchar_t *fmt, _Arg0&& arg0, _Args&&... args) {
        FWStringBuilder oss;
        Format(oss, fmt, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
        return Show(oss.ToString(), caption, dialogType, iconType);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
