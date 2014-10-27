#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace DialogBox {
    enum class Type         : u32 {
        Ok                  = 0x00000000L,
        OkCancel            = 0x00000001L,
        AbortRetryIgnore    = 0x00000002L,
        YesNoCancel         = 0x00000003L,
        YesNo               = 0x00000004L,
        RetryCancel         = 0x00000005L,
        CancelTryContinue   = 0x00000006L
    };

    enum class Icon         : u32 {
        Hand                = 0x00000010L,
        Question            = 0x00000020L,
        Exclamation         = 0x00000030L,
        Aterisk             = 0x00000040L,
    };

    enum class Result       : u32 {
        Ok                  = 1,
        Cancel              = 2,
        Abort               = 3,
        Retry               = 4,
        Ignore              = 5,
        Yes                 = 6,
        No                  = 7,
        TryAgain            = 10,
        Continue            = 11,
    };

    Result Show(const wchar_t *text, const wchar_t *caption, Type dialogType, Icon iconType);

    inline Result Ok(const wchar_t *text, const wchar_t *caption, Icon iconType) { return Show(text, caption, Type::Ok, iconType); }
    inline Result OkCancel(const wchar_t *text, const wchar_t *caption, Icon iconType) { return Show(text, caption, Type::OkCancel, iconType); }
    inline Result AbortRetryIgnore(const wchar_t *text, const wchar_t *caption, Icon iconType) { return Show(text, caption, Type::AbortRetryIgnore, iconType); }
    inline Result YesNoCancel(const wchar_t *text, const wchar_t *caption, Icon iconType) { return Show(text, caption, Type::YesNoCancel, iconType); }
    inline Result YesNo(const wchar_t *text, const wchar_t *caption, Icon iconType) { return Show(text, caption, Type::YesNo, iconType); }
    inline Result RetryCancel(const wchar_t *text, const wchar_t *caption, Icon iconType) { return Show(text, caption, Type::RetryCancel, iconType); }
    inline Result CancelTryContinue(const wchar_t *text, const wchar_t *caption, Icon iconType) { return Show(text, caption, Type::CancelTryContinue, iconType); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
