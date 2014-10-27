#include "stdafx.h"

#include "DialogBox.h"

#ifdef OS_WINDOWS
#   include <windows.h>
#else
#   error "no support"
#endif

namespace Core {
namespace DialogBox {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(MB_OK == (u32)Core::DialogBox::Type::Ok);
STATIC_ASSERT(MB_OKCANCEL == (u32)Core::DialogBox::Type::OkCancel);
STATIC_ASSERT(MB_ABORTRETRYIGNORE == (u32)Core::DialogBox::Type::AbortRetryIgnore);
STATIC_ASSERT(MB_YESNOCANCEL == (u32)Core::DialogBox::Type::YesNoCancel);
STATIC_ASSERT(MB_YESNO == (u32)Core::DialogBox::Type::YesNo);
STATIC_ASSERT(MB_RETRYCANCEL == (u32)Core::DialogBox::Type::RetryCancel);
STATIC_ASSERT(MB_CANCELTRYCONTINUE == (u32)Core::DialogBox::Type::CancelTryContinue);
//----------------------------------------------------------------------------
STATIC_ASSERT(MB_ICONHAND == (u32)Core::DialogBox::Icon::Hand);
STATIC_ASSERT(MB_ICONQUESTION == (u32)Core::DialogBox::Icon::Question);
STATIC_ASSERT(MB_ICONEXCLAMATION == (u32)Core::DialogBox::Icon::Exclamation);
STATIC_ASSERT(MB_ICONASTERISK == (u32)Core::DialogBox::Icon::Aterisk);
//----------------------------------------------------------------------------
STATIC_ASSERT(IDOK == (u32)Core::DialogBox::Result::Ok);
STATIC_ASSERT(IDCANCEL == (u32)Core::DialogBox::Result::Cancel);
STATIC_ASSERT(IDABORT == (u32)Core::DialogBox::Result::Abort);
STATIC_ASSERT(IDRETRY == (u32)Core::DialogBox::Result::Retry);
STATIC_ASSERT(IDIGNORE == (u32)Core::DialogBox::Result::Ignore);
STATIC_ASSERT(IDYES == (u32)Core::DialogBox::Result::Yes);
STATIC_ASSERT(IDNO == (u32)Core::DialogBox::Result::No);
STATIC_ASSERT(IDTRYAGAIN == (u32)Core::DialogBox::Result::TryAgain);
STATIC_ASSERT(IDCONTINUE == (u32)Core::DialogBox::Result::Continue);
//----------------------------------------------------------------------------
DialogBox::Result Show(const wchar_t *text, const wchar_t *caption, DialogBox::Type dialogType, DialogBox::Icon iconType)
{
    AssertRelease(text);
    AssertRelease(caption);
    return static_cast<DialogBox::Result>(::MessageBoxW(
        NULL, text, caption,
        (u32)dialogType|(u32)iconType|MB_DEFBUTTON1|MB_TASKMODAL|MB_TOPMOST));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DialogBox
} //!namespace Core
