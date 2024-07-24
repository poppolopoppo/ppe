// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Linux/LinuxPlatformDialog.h"

#ifdef PLATFORM_LINUX

#include "Diagnostic/Callstack.h"
#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/DecodedCallstack.h"
#include "Diagnostic/Logger.h"

#include "Container/Vector.h"
#include "IO/FileSystem.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "HAL/PlatformCrash.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformMisc.h"
#include "Memory/MemoryProvider.h"
#include "Meta/Utility.h"

#include "HAL/Linux/Errno.h"
#include "HAL/Linux/LinuxPlatformConsole.h"

// #TODO: XMotif ?
// would require to link to some graphical libs :/
// https://stackoverflow.com/questions/1384125/c-messagebox-for-linux-like-in-ms-windows

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FLinuxPlatformDialog::EResult ConsoleMessageBox_(
    const FWStringView& text,
    const FWStringView& caption,
    FLinuxPlatformDialog::EType dialogType,
    FLinuxPlatformDialog::EIcon iconType ) {

    FWStringView iconLogo;
    FLinuxPlatformConsole::EAttribute attr;
    switch (iconType) {
    case FLinuxPlatformDialog::Hand:
        iconLogo = L"@_"_view;
        attr = FLinuxPlatformConsole::INFO;
        break;
    case FLinuxPlatformDialog::Question:
        iconLogo = L"??"_view;
        attr = FLinuxPlatformConsole::HALT;
        break;
    case FLinuxPlatformDialog::Exclamation:
        iconLogo = L"!%"_view;
        attr = FLinuxPlatformConsole::EMPHASIS;
        break;
    case FLinuxPlatformDialog::Aterisk:
        iconLogo = L"**"_view;
        attr = FLinuxPlatformConsole::ASTERISK;
        break;
    case FLinuxPlatformDialog::Error:
        iconLogo = L"!!"_view;
        attr = FLinuxPlatformConsole::ERROR;
        break;
    case FLinuxPlatformDialog::Warning:
        iconLogo = L"!?"_view;
        attr = FLinuxPlatformConsole::WARNING;
        break;
    case FLinuxPlatformDialog::Information:
        iconLogo = L"##"_view;
        attr = FLinuxPlatformConsole::VERBOSE;
        break;
    default:
        AssertNotImplemented();
    }

    FLinuxPlatformConsole::Open();
    ON_SCOPE_EXIT([](){ FLinuxPlatformConsole::Close(); });

    FLinuxPlatformConsole::Write(iconLogo, attr);
    FLinuxPlatformConsole::Write(L"\t"_view);
    FLinuxPlatformConsole::Write(caption, attr);
    FLinuxPlatformConsole::Write(L"\n"_view);
    FLinuxPlatformConsole::Write(text);
    FLinuxPlatformConsole::Write(L"\n"_view);

    if (dialogType & FLinuxPlatformDialog::Ok)
        FLinuxPlatformConsole::Write(L" Ok(O)"_view, FLinuxPlatformConsole::SUCCESS);
    if (dialogType & FLinuxPlatformDialog::Cancel)
        FLinuxPlatformConsole::Write(L" Cancel(C)"_view, FLinuxPlatformConsole::WARNING);
    if (dialogType & FLinuxPlatformDialog::Abort)
        FLinuxPlatformConsole::Write(L" Abort(A)"_view, FLinuxPlatformConsole::ERROR);
    if (dialogType & FLinuxPlatformDialog::Retry)
        FLinuxPlatformConsole::Write(L" Retry(R)"_view, FLinuxPlatformConsole::EMPHASIS);
    if (dialogType & FLinuxPlatformDialog::Ignore)
        FLinuxPlatformConsole::Write(L" Ignore(I)"_view, FLinuxPlatformConsole::WARNING);
    if (dialogType & FLinuxPlatformDialog::Yes)
        FLinuxPlatformConsole::Write(L" Yes(Y)"_view, FLinuxPlatformConsole::SUCCESS);
    if (dialogType & FLinuxPlatformDialog::No)
        FLinuxPlatformConsole::Write(L" No(N)"_view, FLinuxPlatformConsole::ERROR);
    if (dialogType & FLinuxPlatformDialog::TryAgain)
        FLinuxPlatformConsole::Write(L" TryAgain(T)"_view, FLinuxPlatformConsole::EMPHASIS);
    if (dialogType & FLinuxPlatformDialog::Continue)
        FLinuxPlatformConsole::Write(L" Continue(U)"_view, FLinuxPlatformConsole::ASTERISK);
    if (dialogType & FLinuxPlatformDialog::IgnoreAlways)
        FLinuxPlatformConsole::Write(L" IgnoreAlways(W)"_view, FLinuxPlatformConsole::VERBOSE);

    for (;;) {
        FLinuxPlatformConsole::Write(L" ?\nResponse: "_view);

        wchar_t buf[256];
        VerifyRelease(FLinuxPlatformConsole::Read(buf));

        if ((ToLower(*buf) == 'o') and (dialogType & FLinuxPlatformDialog::Ok))
            return FLinuxPlatformDialog::Ok;
        if ((ToLower(*buf) == 'c') and (dialogType & FLinuxPlatformDialog::Cancel))
            return FLinuxPlatformDialog::Cancel;
        if ((ToLower(*buf) == 'a') and (dialogType & FLinuxPlatformDialog::Abort))
            return FLinuxPlatformDialog::Abort;
        if ((ToLower(*buf) == 'r') and (dialogType & FLinuxPlatformDialog::Retry))
            return FLinuxPlatformDialog::Retry;
        if ((ToLower(*buf) == 'i') and (dialogType & FLinuxPlatformDialog::Ignore))
            return FLinuxPlatformDialog::Ignore;
        if ((ToLower(*buf) == 'y') and (dialogType & FLinuxPlatformDialog::Yes))
            return FLinuxPlatformDialog::Yes;
        if ((ToLower(*buf) == 'n') and (dialogType & FLinuxPlatformDialog::No))
            return FLinuxPlatformDialog::No;
        if ((ToLower(*buf) == 't') and (dialogType & FLinuxPlatformDialog::TryAgain))
            return FLinuxPlatformDialog::TryAgain;
        if ((ToLower(*buf) == 'u') and (dialogType & FLinuxPlatformDialog::Continue))
            return FLinuxPlatformDialog::Continue;
        if ((ToLower(*buf) == 'w') and (dialogType & FLinuxPlatformDialog::IgnoreAlways))
            return FLinuxPlatformDialog::IgnoreAlways;
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
auto FLinuxPlatformDialog::Show(const FWStringView& text, const FWStringView& caption, EType dialogType, EIcon iconType) -> EResult {
    Assert(not text.empty());
    Assert(not caption.empty());

    // #TODO: override in Application HAL when we finally get a window ?
    return ConsoleMessageBox_(text, caption, dialogType, iconType);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
