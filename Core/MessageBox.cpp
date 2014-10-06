#include "stdafx.h"

#include "MessageBox.h"

#ifdef OS_WINDOWS
#   include <windows.h>
#   ifdef MessageBox
#       undef MessageBox
#   endif
#else
#   error "no support"
#endif

namespace Core {
namespace MessageBox {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(MB_OK == (u32)MessageBox::Type::Ok);
STATIC_ASSERT(MB_OKCANCEL == (u32)MessageBox::Type::OkCancel);
STATIC_ASSERT(MB_ABORTRETRYIGNORE == (u32)MessageBox::Type::AbortRetryIgnore);
STATIC_ASSERT(MB_YESNOCANCEL == (u32)MessageBox::Type::YesNoCancel);
STATIC_ASSERT(MB_YESNO == (u32)MessageBox::Type::YesNo);
STATIC_ASSERT(MB_RETRYCANCEL == (u32)MessageBox::Type::RetryCancel);
STATIC_ASSERT(MB_CANCELTRYCONTINUE == (u32)MessageBox::Type::CancelTryContinue);
//----------------------------------------------------------------------------
STATIC_ASSERT(MB_ICONHAND == (u32)MessageBox::Icon::Hand);
STATIC_ASSERT(MB_ICONQUESTION == (u32)MessageBox::Icon::Question);
STATIC_ASSERT(MB_ICONEXCLAMATION == (u32)MessageBox::Icon::Exclamation);
STATIC_ASSERT(MB_ICONASTERISK == (u32)MessageBox::Icon::Aterisk);
//----------------------------------------------------------------------------
STATIC_ASSERT(IDOK == (u32)MessageBox::Result::Ok);
STATIC_ASSERT(IDCANCEL == (u32)MessageBox::Result::Cancel);
STATIC_ASSERT(IDABORT == (u32)MessageBox::Result::Abort);
STATIC_ASSERT(IDRETRY == (u32)MessageBox::Result::Retry);
STATIC_ASSERT(IDIGNORE == (u32)MessageBox::Result::Ignore);
STATIC_ASSERT(IDYES == (u32)MessageBox::Result::Yes);
STATIC_ASSERT(IDNO == (u32)MessageBox::Result::No);
STATIC_ASSERT(IDTRYAGAIN == (u32)MessageBox::Result::TryAgain);
STATIC_ASSERT(IDCONTINUE == (u32)MessageBox::Result::Continue);
//----------------------------------------------------------------------------
Result Show(const wchar_t *text, const wchar_t *caption, Type dialogType, Icon iconType)
{
    AssertRelease(text);
    AssertRelease(caption);
    return (Result)MessageBoxW(NULL, text, caption, (u32)dialogType|(u32)iconType|MB_DEFBUTTON1|MB_TASKMODAL|MB_TOPMOST);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace MessageBox
} //!namespace Core
