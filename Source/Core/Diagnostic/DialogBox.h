#pragma once

#include "Core/Core.h"

#include "Core/IO/Format.h"
#include "Core/IO/Stream.h"
#include "Core/IO/StringSlice.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace DialogBox {
    enum class Icon {
        Hand                = 0,
        Question            ,
        Exclamation         ,
        Aterisk             ,
        Error               ,
        Warning             ,
        Information         ,
    };

    enum class Result {
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

    enum class Type {
        Ok                      = (1<<(size_t)Result::Ok),
        OkCancel                = (1<<(size_t)Result::Ok)|(1<<(size_t)Result::Cancel),
        AbortRetryIgnore        = (1<<(size_t)Result::Abort)|(1<<(size_t)Result::Retry)|(1<<(size_t)Result::Ignore),
        YesNoCancel             = (1<<(size_t)Result::Yes)|(1<<(size_t)Result::No)|(1<<(size_t)Result::Cancel),
        YesNo                   = (1<<(size_t)Result::Yes)|(1<<(size_t)Result::No),
        RetryCancel             = (1<<(size_t)Result::Retry)|(1<<(size_t)Result::Cancel),
        CancelTryContinue       = (1<<(size_t)Result::Cancel)|(1<<(size_t)Result::TryAgain)|(1<<(size_t)Result::Continue),
        IgnoreOnceAlwaysAbort   = (1<<(size_t)Result::Ignore)|(1<<(size_t)Result::IgnoreAlways)|(1<<(size_t)Result::Abort),
    };

    Result Show(const WStringSlice& text, const WStringSlice& caption, Type dialogType, Icon iconType);

    inline Result Ok(const WStringSlice& text, const WStringSlice& caption, Icon iconType = Icon::Information) { return Show(text, caption, Type::Ok, iconType); }
    inline Result OkCancel(const WStringSlice& text, const WStringSlice& caption, Icon iconType = Icon::Question) { return Show(text, caption, Type::OkCancel, iconType); }
    inline Result AbortRetryIgnore(const WStringSlice& text, const WStringSlice& caption, Icon iconType = Icon::Error) { return Show(text, caption, Type::AbortRetryIgnore, iconType); }
    inline Result YesNoCancel(const WStringSlice& text, const WStringSlice& caption, Icon iconType = Icon::Question) { return Show(text, caption, Type::YesNoCancel, iconType); }
    inline Result YesNo(const WStringSlice& text, const WStringSlice& caption, Icon iconType = Icon::Question) { return Show(text, caption, Type::YesNo, iconType); }
    inline Result RetryCancel(const WStringSlice& text, const WStringSlice& caption, Icon iconType = Icon::Exclamation) { return Show(text, caption, Type::RetryCancel, iconType); }
    inline Result CancelTryContinue(const WStringSlice& text, const WStringSlice& caption, Icon iconType = Icon::Exclamation) { return Show(text, caption, Type::CancelTryContinue, iconType); }
    inline Result IgnoreOnceAlwaysAbort(const WStringSlice& text, const WStringSlice& caption, Icon iconType = Icon::Error) { return Show(text, caption, Type::IgnoreOnceAlwaysAbort, iconType); }

    template <typename _Arg0, typename... _Args>
    Result Show(const WStringSlice& caption, Type dialogType, Icon iconType, const wchar_t *fmt, _Arg0&& arg0, _Args&&... args) {
        ThreadLocalWOStringStream oss;
        Format(oss, fmt, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
        return Show(MakeStringSlice(oss.str()), caption, dialogType, iconType);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
