// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Windows/WindowsPlatformDialog.h"

#include "Diagnostic/BuildVersion.h"

#include "HAL/PlatformLowLevelIO.h"
#include "IO/BufferedStream.h"
#include "IO/FileStream.h"
#include "IO/TextReader.h"
#include "IO/TextReader_fwd.h"

#include "Modular/ModularDomain.h"

#include "Runtime/VFS/Public/VirtualFileSystem_fwd.h"

#ifdef PLATFORM_WINDOWS

#include "Diagnostic/Callstack.h"
#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/DecodedCallstack.h"
#include "Diagnostic/Logger.h"

#include "Container/BitMask.h"
#include "Container/SparseArray.h"
#include "Container/Vector.h"
#include "IO/FileSystem.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "HAL/PlatformCrash.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformMisc.h"
#include "Memory/MemoryProvider.h"
#include "Memory/RefPtr.h"
#include "Meta/Utility.h"
#include "Misc/Function.h"
#include "Thread/ThreadContext.h"
#include "Thread/ThreadSafe.h"

#include "HAL/Windows/LastError.h"

#include <CommCtrl.h> // PROGRESS_CLASS
#include <Windowsx.h> // Edit_GetSel()

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FWStringLiteral MonospaceFontName_() {
    return L"Consolas";
}
//----------------------------------------------------------------------------
static LPCWSTR SystemIcon_(FWindowsPlatformDialog::EIcon iconType) {
    switch (iconType)
    {
    case PPE::FWindowsPlatformDialog::EIcon::Hand:
        return IDI_HAND;
    case PPE::FWindowsPlatformDialog::EIcon::Question:
        return IDI_QUESTION;
    case PPE::FWindowsPlatformDialog::EIcon::Exclamation:
        return IDI_EXCLAMATION;
    case PPE::FWindowsPlatformDialog::EIcon::Aterisk:
        return IDI_ASTERISK;
    case PPE::FWindowsPlatformDialog::EIcon::Error:
        return IDI_ERROR;
    case PPE::FWindowsPlatformDialog::EIcon::Warning:
        return IDI_WARNING;
    case PPE::FWindowsPlatformDialog::EIcon::Information:
        return IDI_INFORMATION;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
static FWStringLiteral ResultCaption_(FWindowsPlatformDialog::EResult result) {
    switch (result)
    {
    case PPE::FWindowsPlatformDialog::EResult::Ok:
        return L"Ok";
    case PPE::FWindowsPlatformDialog::EResult::Cancel:
        return L"Cancel";
    case PPE::FWindowsPlatformDialog::EResult::Abort:
        return L"Abort";
    case PPE::FWindowsPlatformDialog::EResult::Retry:
        return L"Retry";
    case PPE::FWindowsPlatformDialog::EResult::Ignore:
        return L"Ignore";
    case PPE::FWindowsPlatformDialog::EResult::Yes:
        return L"Yes";
    case PPE::FWindowsPlatformDialog::EResult::No:
        return L"No";
    case PPE::FWindowsPlatformDialog::EResult::TryAgain:
        return L"Try Again";
    case PPE::FWindowsPlatformDialog::EResult::Continue:
        return L"Continue";
    case PPE::FWindowsPlatformDialog::EResult::IgnoreAlways:
        return L"Ignore Always";
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
enum class EAtomClass_ : ::WORD {
    Predefined  = 0,
    Button      = 0x0080,
    Edit        = 0x0081,
    Static      = 0x0082,
    ListBox     = 0x0083,
    ScrollBar   = 0x0084,
    ComboBox    = 0x0085,
};
//----------------------------------------------------------------------------
static constexpr size_t DIALOG_ID_TEXT          = 150;
static constexpr size_t DIALOG_ID_STACK         = 151;
static constexpr size_t DIALOG_ID_MINIDUMP      = 152;
static constexpr size_t DIALOG_ID_COPY          = 153;
static constexpr size_t DIALOG_ID_BREAK         = 154;
static constexpr size_t DIALOG_ID_ICON          = 155;
static constexpr size_t DIALOG_ID_EDIT          = 156;
static constexpr size_t DIALOG_ID_TITLE         = 157;
static constexpr size_t DIALOG_ID_BACKGROUND    = 158;
static constexpr size_t ResultToID_(FWindowsPlatformDialog::EResult button) { return 200+size_t(button); }
static constexpr FWindowsPlatformDialog::EResult IDToResult_(size_t id) { return FWindowsPlatformDialog::EResult(id-200); }
//----------------------------------------------------------------------------
static void Template_AddItem_(
    FMemoryViewWriter& writer,
    size_t x, size_t y,
    size_t cx, size_t cy,
    size_t id, size_t style,
    EAtomClass_ klass ) {
    const auto eaten = writer.EatAligned(sizeof(::DLGITEMTEMPLATE), sizeof(DWORD));
    Assert(Meta::IsAlignedPow2(sizeof(DWORD), eaten.Pointer()));
    ::LPDLGITEMTEMPLATE const tpl = reinterpret_cast<::LPDLGITEMTEMPLATE>(eaten.Pointer());
    ::ZeroMemory(tpl, sizeof(::DLGITEMTEMPLATE));

    tpl->x = checked_cast<short>(x);
    tpl->y = checked_cast<short>(y);
    tpl->cx = checked_cast<short>(cx);
    tpl->cy = checked_cast<short>(cy);
    tpl->id = checked_cast<WORD>(id);
    tpl->style = checked_cast<DWORD>(style);

    if (klass == EAtomClass_::Predefined) {
        writer.WritePOD(u32(0));
    }
    else {
        writer.WritePOD(u16(0xFFFF));
        writer.WritePOD(u16(klass));
    }
}
//----------------------------------------------------------------------------
static void Template_AddCaption_(FMemoryViewWriter& writer, const FWStringView& caption ) {
    writer.Write(caption.Pointer(), caption.SizeInBytes());
    if (caption.back() != L'\0')
        writer.WritePOD(L'\0');

    //// next element must be aligned on word boundary
    //// https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-dlgitemtemplate#:~:text=The%20creation%20data%20array%20begins%20at%20the%20next%20WORD%20boundary%20after%20the%20title%20array.
    //while (Meta::IsAligned(sizeof(WORD), checked_cast<uintptr_t>(writer.TellO())))
    //    writer.WritePOD('\0');
}
//----------------------------------------------------------------------------
static void Template_AddButton_(
    FMemoryViewWriter& writer,
    size_t x, size_t y,
    size_t cx, size_t cy,
    size_t id,
    const FWStringLiteral& caption ) {
    Template_AddItem_(writer, x, y, cx, cy,
        id,
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, EAtomClass_::Button);
    Template_AddCaption_(writer, caption.MakeView());
    writer.WritePOD(WORD(0)); // no creation data
}
//----------------------------------------------------------------------------
struct FTemplate_DialogContext_ {
    FWStringView Text;
    FWStringView Caption;
    FConstWChar SourceFile;
    u32 SourceLine{ 0 };
    FWindowsPlatformDialog::EType Buttons{ Default };
    ::LPCWSTR IconId{ NULL };
    ::HICON hIconResource{ NULL };
    FDecodedCallstack DecodedCallstack;
    TMemoryView<const FWString> ListItems;
};
//----------------------------------------------------------------------------
static LRESULT CALLBACK Template_TextProc_(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message)
    {
        case WM_RBUTTONUP:
            {
                const DWORD selection = Edit_GetSel(hwndDlg);
                const DWORD sel0 = LOWORD(selection);
                const DWORD sel1 = HIWORD(selection);

                wchar_t buffer[2048];
                DWORD length = ::GetWindowTextW(hwndDlg, buffer, lengthof(buffer));

                FWStringView text(buffer, length);
                if (sel0 < sel1 && sel1 < length)
                    text = text.SubRange(sel0, sel1 - sel0);

                FPlatformMisc::ClipboardCopy(text.data(), text.size());
            }
            return TRUE;

        default:
            {
                if (::WNDPROC prevProc = bit_cast<::WNDPROC>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA)))
                    return ::CallWindowProc(prevProc, hwndDlg, message, wParam, lParam);
            }
    }
    return ::DefWindowProc(hwndDlg, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static LRESULT CALLBACK Template_StackProc_(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    Unused(wParam);
    Unused(lParam);
    switch (message)
    {
        case WM_LBUTTONDBLCLK:
            {
                HWND window = ::GetParent(hwndDlg);
                LRESULT index = ::SendDlgItemMessage(window, DIALOG_ID_STACK, LB_GETCURSEL, 0, 0);
                if (index != LB_ERR) {
                    const FTemplate_DialogContext_* ctx = bit_cast<const FTemplate_DialogContext_*>(::GetWindowLongPtr(window, GWLP_USERDATA));
                    Assert(ctx);

                    if (ctx->SourceFile) {
                        // assume source file content
                        FPlatformMisc::ExternalTextEditor(ctx->SourceFile, index + 1);
                    }
                    else {
                        // assume callstack
                        FPlatformMisc::ExternalTextEditor(
                            ctx->DecodedCallstack.Frames()[index].Filename().data(),
                            ctx->DecodedCallstack.Frames()[index].Line() );
                    }
                }
            }
            break;

        default:
            {
                if (::WNDPROC prevProc = bit_cast<::WNDPROC>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA)) )
                    return ::CallWindowProc(prevProc, hwndDlg, message, wParam, lParam);
            }
    }
    return ::DefWindowProc(hwndDlg, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static LRESULT CALLBACK Template_DialogProc_(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message)
    {
    case WM_INITDIALOG:
        {
            Assert(lParam);
            const FTemplate_DialogContext_* ctx = bit_cast<const FTemplate_DialogContext_*>(lParam);

            ::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)ctx);

            ::SendMessage(hwndDlg, WM_SETICON, 0, (LPARAM)ctx->hIconResource);

            if (::HWND const hIcon = ::GetDlgItem(hwndDlg, DIALOG_ID_ICON))
                ::SendMessage(hIcon, STM_SETICON, (WPARAM)ctx->hIconResource, 0);

            if (::HWND const hText = ::GetDlgItem(hwndDlg, DIALOG_ID_TEXT))
                ::SetWindowLongPtr(hText, GWLP_USERDATA, ::SetWindowLongPtr(hText, GWLP_WNDPROC, (LPARAM)Template_TextProc_));

            if (::HWND const hStack = ::GetDlgItem(hwndDlg, DIALOG_ID_STACK)) {
                ::SetWindowLongPtr(hStack, GWLP_USERDATA, ::SetWindowLongPtr(hStack, GWLP_WNDPROC, (LPARAM)Template_StackProc_));
                ::SendMessage(hStack, LB_SETHORIZONTALEXTENT, 4096, 0);

                for (const FWString& it : ctx->ListItems)
                    ::SendMessageW(hStack, LB_ADDSTRING, 0, (LPARAM)it.c_str());

                if (ctx->SourceFile && ctx->SourceLine > 0)
                    ::SendMessageW(hStack, LB_SETCURSEL, ctx->SourceLine - 1, 0);
            }

            ::RECT rect;
            ::GetWindowRect(hwndDlg, &rect);
            ::SetWindowPos(hwndDlg, NULL,
                (::GetSystemMetrics(SM_CXSCREEN)-rect.right)/2,
                (::GetSystemMetrics(SM_CYSCREEN)-rect.bottom)/2,
                rect.right - rect.left,
                rect.bottom - rect.top,
                0 );
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ResultToID_(FWindowsPlatformDialog::EResult::Ok):
        case ResultToID_(FWindowsPlatformDialog::EResult::Cancel):
        case ResultToID_(FWindowsPlatformDialog::EResult::Abort):
        case ResultToID_(FWindowsPlatformDialog::EResult::Retry):
        case ResultToID_(FWindowsPlatformDialog::EResult::Ignore):
        case ResultToID_(FWindowsPlatformDialog::EResult::Yes):
        case ResultToID_(FWindowsPlatformDialog::EResult::No):
        case ResultToID_(FWindowsPlatformDialog::EResult::TryAgain):
        case ResultToID_(FWindowsPlatformDialog::EResult::Continue):
        case ResultToID_(FWindowsPlatformDialog::EResult::IgnoreAlways):
            ::EndDialog(hwndDlg, LOWORD(wParam));
            return TRUE;

        case DIALOG_ID_COPY:
            {
                const FTemplate_DialogContext_* ctx = bit_cast<const FTemplate_DialogContext_*>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
                Assert(ctx);

                FWStringBuilder oss;

                if (ctx->SourceFile) {
                    oss << ctx->Text << Crlf
                    << Crlf
                    << L"----------------------------------------------------------------" << Crlf
                    << L"Source: " << Fmt::Quoted(ctx->SourceFile, '"') << Crlf
                    << L"----------------------------------------------------------------" << Crlf;
                }
                else {
                    oss << ctx->Text << Crlf
                    << Crlf
                    << L"----------------------------------------------------------------" << Crlf
                    << L"FCallstack:" << Crlf
                    << L"----------------------------------------------------------------" << Crlf;
                }

                for (const FWString& frame : ctx->ListItems)
                    oss << frame << Crlf;

                const FWStringView clipboard = oss.Written();
                FWindowsPlatformMisc::ClipboardCopy(clipboard.data(), clipboard.size());
            }
            return TRUE;

        case DIALOG_ID_BREAK:
            {
                ::DebugBreak();
            }
            return TRUE;

        case DIALOG_ID_EDIT:
            {
                const FTemplate_DialogContext_* ctx = bit_cast<const FTemplate_DialogContext_*>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
                Assert(ctx);

                if (ctx and ctx->SourceFile)
                    FPlatformMisc::ExternalTextEditor(ctx->SourceFile);
            }
            return TRUE;

        case DIALOG_ID_MINIDUMP:
            {
                const FFilename process = FCurrentProcess::Get().FileName();
                const FWString path = FPlatformFile::MakeTemporaryFile(process.BasenameNoExt().c_str(), L".dmp");

                ::LPCWSTR caption = nullptr;
                ::UINT dialogType = MB_OK;
                switch (FPlatformCrash::WriteMiniDump(path, FPlatformCrash::Large)) {
                // Success:
                case FPlatformCrash::Success:
                    caption = L"Coredump succeeded";
                    dialogType |= MB_ICONASTERISK;
                    FWindowsPlatformMisc::ClipboardCopy(path.data(), path.size());
                    break;

                // Failure:
                case FPlatformCrash::NoDbgHelpDLL:
                    caption = L"Coredump failed: no dbghelp";
                    dialogType |= MB_ICONEXCLAMATION;
                    break;
                case FPlatformCrash::InvalidFilename:
                    caption = L"Coredump failed: invalid filename";
                    dialogType |= MB_ICONEXCLAMATION;
                    break;
                case FPlatformCrash::CantCreateFile:
                    caption = L"Coredump failed: can't create file";
                    dialogType |= MB_ICONEXCLAMATION;
                    break;
                case FPlatformCrash::DumpFailed:
                    caption = L"Coredump failed: unknown error";
                    dialogType |= MB_ICONEXCLAMATION;
                    break;
                case FPlatformCrash::FailedToCloseHandle:
                    caption = L"Coredump failed: failed to close file handle";
                    dialogType |= MB_ICONEXCLAMATION;
                    break;
                case FPlatformCrash::NotAvailable:
                    caption = L"Coredump failed: feature not available";
                    dialogType |= MB_ICONEXCLAMATION;
                    break;
                case FPlatformCrash::Reentrancy:
                    caption = L"Coredump failed: reentrancy";
                    dialogType |= MB_ICONEXCLAMATION;
                    break;
                }

                ::MessageBoxExW(hwndDlg, path.c_str(), caption, dialogType, 0);
            }
            return TRUE;
        }
    }
    return ::DefWindowProc(hwndDlg, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static constexpr FWindowsPlatformDialog::EResult GTemplate_AllButtons[] = {
    FWindowsPlatformDialog::EResult::Ok,
    FWindowsPlatformDialog::EResult::Retry,
    FWindowsPlatformDialog::EResult::Ignore,
    FWindowsPlatformDialog::EResult::Yes,
    FWindowsPlatformDialog::EResult::TryAgain,
    FWindowsPlatformDialog::EResult::Continue,
    FWindowsPlatformDialog::EResult::IgnoreAlways,
    FWindowsPlatformDialog::EResult::No,
    FWindowsPlatformDialog::EResult::Cancel,
    FWindowsPlatformDialog::EResult::Abort,
};
//----------------------------------------------------------------------------
static ::HINSTANCE AppHandleWin32_() {
    return static_cast<::HINSTANCE>(FCurrentProcess::Get().AppHandle());
}
//----------------------------------------------------------------------------
static FWindowsPlatformDialog::EResult Template_CreateDialogBox_(
    FWindowsPlatformDialog::EIcon icon,
    FWindowsPlatformDialog::EType buttons,
    const FWStringView& text,
    const FWStringView& caption,
    FConstWChar sourceFile = nullptr,
    u32 sourceLine = 0 ) {
    STATIC_ASSERT(sizeof(u16) == sizeof(::WORD));
    STATIC_ASSERT(sizeof(u32) == sizeof(::DWORD));

    FTemplate_DialogContext_ ctx;
    ctx.Text = text;
    ctx.Caption = caption;
    ctx.SourceFile = sourceFile;
    ctx.SourceLine = sourceLine;
    ctx.Buttons = buttons;
    ctx.IconId = SystemIcon_(icon);
    ctx.hIconResource = ::LoadIconW(nullptr, ctx.IconId);

    VECTORINSITU(Diagnostic, FWString, FCallstack::MaxDepth) listItems;

    if (not ctx.SourceFile) {
        // no source file provided -> decode current frame callstack
        FCallstack callstack;
        FCallstack::Capture(&callstack, 5, FCallstack::MaxDepth);

        ctx.DecodedCallstack = FDecodedCallstack(callstack);
        listItems.reserve(ctx.DecodedCallstack.Frames().size());

        FWStringBuilder sb;
        for (const FDecodedCallstack::FFrame& frame : ctx.DecodedCallstack.Frames()) {
            sb  << L' ' << frame.Filename()
                << L'(' << frame.Line() << L"): " << frame.Symbol();
            listItems.emplace_back(sb.ToString());
        }
    }
    else {
        // source file provided -> read content and display it assuming text
        FFileStreamReader reader = FFileStreamReader::OpenRead(sourceFile, EAccessPolicy::Binary);
        if (reader.Good()) {
            UsingBufferedStream(&reader, [&](IBufferedStreamReader* buffered) {
                FTextReader text{ buffered };

                FString line;
                for (u32 index = 0; text.ReadLine(&line); index++) {
                    listItems.push_back(StringFormat(L"{:#4}| {}", (index + 1), line));
                    line.clear();
                }
            });
        }
        else {
            listItems.push_back(StringFormat(L"failed to open source file: {:q}", sourceFile));
        }
    }
    ctx.ListItems = listItems.MakeConstView();

    CONSTEXPR const size_t GAllocSize = 8192;
    ::HGLOBAL const hgbl = ::GlobalAlloc(GMEM_ZEROINIT, GAllocSize);
    Assert(nullptr != hgbl);
    DEFERRED{ ::GlobalFree(hgbl); };

    {
        FMemoryViewWriter writer(TMemoryView<u8>((u8*)::GlobalLock(hgbl), GAllocSize));
        DEFERRED{ ::GlobalUnlock(hgbl); };

        const auto eaten = writer.Eat(sizeof(::DLGTEMPLATE));
        Assert(Meta::IsAlignedPow2(sizeof(::DWORD), eaten.Pointer()));
        ::LPDLGTEMPLATE const tpl = bit_cast<::LPDLGTEMPLATE>(eaten.Pointer());

        tpl->x = 10;
        tpl->y = 10;
        tpl->cx = 600;
        tpl->cy = 300;
        tpl->style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION | DS_SETFONT;
        tpl->cdit = 0;

        writer.WritePOD(::WORD(0)); // No menu
        writer.WritePOD(::WORD(EAtomClass_::Predefined)); // default class

        Template_AddCaption_(writer, caption);

        writer.WritePOD(::WORD(8)); // Font size
        Template_AddCaption_(writer, MonospaceFontName_().MakeView()); // Font name

        // modal buttons
        constexpr size_t buttonWidthPerChar = 8;
        constexpr size_t buttonWidthPadding = 3;
        constexpr size_t buttonHeight = 15;

        const size_t buttonTop = size_t(tpl->cy) - 5 - buttonHeight;
        size_t buttonRight = size_t(tpl->cx) - 2;

        for (FWindowsPlatformDialog::EResult button : GTemplate_AllButtons) {
            if ((size_t)button & (size_t)buttons) {
                const FWStringLiteral buttonCaption = ResultCaption_(button);
                const size_t w = buttonWidthPadding * 2 + buttonWidthPerChar * buttonCaption.size();
                buttonRight -= buttonWidthPadding + w;

                Template_AddButton_(writer, buttonRight, buttonTop, w, buttonHeight, ResultToID_(button), buttonCaption);
                tpl->cdit++;
            }
        }

        Template_AddButton_(writer, 5, buttonTop, 32, buttonHeight, DIALOG_ID_COPY, L"Copy");
        tpl->cdit++;

        Template_AddButton_(writer, 40, buttonTop, 32, buttonHeight, DIALOG_ID_BREAK, L"Break");
        tpl->cdit++;

        Template_AddButton_(writer, 75, buttonTop, 55, buttonHeight, DIALOG_ID_MINIDUMP, L"Minidump");
        tpl->cdit++;

        if (sourceFile) {
            Template_AddButton_(writer, 75+3+55, buttonTop, 32, buttonHeight, DIALOG_ID_EDIT, L"Edit");
            tpl->cdit++;
        }

        Template_AddItem_(writer, 15, 10, 32, 32,
            DIALOG_ID_ICON,
            WS_CHILD | WS_VISIBLE | SS_ICON | SS_LEFT, EAtomClass_::Static);
        writer.WritePOD(::WORD(0)); // no caption text
        writer.WritePOD(::WORD(0)); // no creation data
        tpl->cdit++;

        Template_AddItem_(writer, 45, 8, 550, 67,
            DIALOG_ID_TEXT,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_READONLY, EAtomClass_::Edit);
        Template_AddCaption_(writer, text);
        writer.WritePOD(::WORD(0)); // no creation data
        tpl->cdit++;

        Template_AddItem_(writer, 5, 5+70+5, 590, 200,
            DIALOG_ID_STACK,
            WS_BORDER | WS_HSCROLL | WS_VSCROLL | WS_CHILD | WS_VISIBLE, EAtomClass_::ListBox);
        writer.WritePOD(::WORD(0));
        tpl->cdit++;
    }

    const ::INT_PTR ret = ::DialogBoxIndirectParamW(NULL, (::LPDLGTEMPLATE)hgbl, NULL, (::DLGPROC)Template_DialogProc_, (LPARAM)&ctx);
    const ::DWORD lastError = ::GetLastError();
    if (-1 == ret)
        PPE_THROW_IT(FLastErrorException("DialogBoxIndirectParamW", lastError));

    return (FWindowsPlatformDialog::EResult)IDToResult_(ret);
}
//----------------------------------------------------------------------------
struct FProgressState {
    FWString Text;

    ::HWND hProgressBar{ NULL };
    ::HWND hStaticLabel{ NULL };

    size_t Total{ 0 };
    size_t Amount{ 0 };

    u32 YLevel{ 0 };
};
//----------------------------------------------------------------------------
class FSplashScreenDialog_ : public Meta::FNonCopyableNorMovable {
    static TThreadSafe<TPtrRef<FSplashScreenDialog_>, EThreadBarrier::RWLock> GInstance_;
public:
    static TPtrRef<FSplashScreenDialog_> GetIFP() NOEXCEPT {
        return GInstance_.LockShared().Value();
    }
    static bool PushOpen() {
        const auto exclusiveInstance = GInstance_.LockExclusive();
        if (not exclusiveInstance.Value())
            exclusiveInstance.Value() = new FSplashScreenDialog_();
        return (exclusiveInstance->RefCount++ == 0);
    }
    static bool PopClose() NOEXCEPT {
        const auto exclusiveInstance = GInstance_.LockExclusive();
        if (--exclusiveInstance->RefCount == 0) {
            exclusiveInstance->Close();
            delete(exclusiveInstance.Get());
            exclusiveInstance.Value().reset();
            return true;
        }
        return false;
    }

    NODISCARD FSparseDataId BeginProgress(const FWStringView& text, size_t total, size_t amount) {
        Assert_NoAssume(not text.empty());
        Assert_NoAssume(amount <= total);

        const FSparseDataId id = Data.LockExclusive()->ProgressBars.Emplace(FProgressState {
            .Text = FWString(text),
            .Total = total,
            .Amount = amount,
        });

        QueueCommand_([id](FInternal& data) {
            if (FProgressState* const pPbar = data.ProgressBars.Find(id); Ensure(pPbar)) {
                pPbar->YLevel = checked_cast<u32>(data.cyYLevelStack.PopFront_AssumeNotEmpty());

                CreateProgressBar_(pPbar, data.hDialog, data.cyProgressBar);

                ::SendMessage(pPbar->hStaticLabel, WM_SETFONT, (WPARAM)data.hTextFont, TRUE);
                ::RedrawWindow(data.hDialog, NULL, NULL, RDW_INVALIDATE);
            }
        });
        return id;
    }

    void IncProgressPos(FSparseDataId id) {
        QueueCommand_([id](FInternal& data) {
            if (FProgressState* const pPbar = data.ProgressBars.Find(id); Ensure(pPbar)) {
                pPbar->Amount += 1;
                ::SendMessage(pPbar->hProgressBar, PBM_SETPOS, pPbar->Amount, 0);
            }
        });
    }

    void SetProgressPos(FSparseDataId id, size_t position) {
        QueueCommand_([id, position](FInternal& data) {
            if (FProgressState* const pPbar = data.ProgressBars.Find(id); Ensure(pPbar)) {
                pPbar->Amount = position;
                ::SendMessage(pPbar->hProgressBar, PBM_SETPOS, pPbar->Amount, 0);
            }
        });
    }

    void SetProgressText(FSparseDataId id, FWString&& rtext) {
        Assert_NoAssume(not rtext.empty());

        QueueCommand_([id, text(std::move(rtext))](FInternal& data) {
            if (FProgressState* const pPbar = data.ProgressBars.Find(id); Ensure(pPbar)) {
                // const_cast since TFunction<> does not support mutable lambda, sorry :(
                pPbar->Text.assign(std::move(*const_cast<FWString*>(&text)));
                Assert_NoAssume(not pPbar->Text.empty());

                ::SetWindowTextW(pPbar->hStaticLabel, *pPbar->Text);
            }
        });
    }

    void SetProgressText(FSparseDataId id, const FWStringView& text) {
        SetProgressText(id, FWString(text));
    }

    void EndProgress(FSparseDataId id) {
        QueueCommand_([id](FInternal& data) {
            if (FProgressState* const pPbar = data.ProgressBars.Find(id); Ensure(pPbar)) {
                ::DestroyWindow(pPbar->hProgressBar);
                ::DestroyWindow(pPbar->hStaticLabel);

                data.cyYLevelStack.SetTrue(pPbar->YLevel);
                data.ProgressBars.Remove(*pPbar);
            }
        });
    }

    void Close() {
        QueueCommand_([](FInternal& data) {
            if (data.hDialog != NULL)
                ::EndDialog(data.hDialog, 0);
        });
    }

private:
    STATIC_CONST_INTEGRAL(size_t, GAllocSize, 4096);

    // https://learn.microsoft.com/fr-fr/windows/win32/winmsg/wm-user
    STATIC_CONST_INTEGRAL(::DWORD, GNotifyCommandMessage, WM_USER + 4321);
    STATIC_ASSERT(GNotifyCommandMessage >= WM_USER && GNotifyCommandMessage < WM_USER + 0x7FFF);

    const ::HGLOBAL hGlobalAlloc{ NULL };
    std::thread DedicatedThread; // need to allocate hGlobalAlloc *BEFORE* DedicatedThread

    struct FInternal {
        SPARSEARRAY_INSITU(HAL, TFunction<void(FInternal&)>) Commands;
        SPARSEARRAY_INSITU(HAL, FProgressState) ProgressBars;

        ::HWND hDialog{ NULL };
        ::HBITMAP hBackgroundBitmap{ NULL };
        ::HICON hIcon{ NULL };
        ::HFONT hTextFont{ NULL };
        ::HFONT hTitleFont{ NULL };

        int cyProgressBar{ 15 };

        FBitMask cyYLevelStack{ FBitMask::AllMask };
    };

    using FThreadSafeData = TThreadSafe<FInternal, EThreadBarrier::RWLock>;
    FThreadSafeData Data;

    i32 RefCount{ 0 };

    FSplashScreenDialog_()
    :   hGlobalAlloc(::GlobalAlloc(GMEM_ZEROINIT, GAllocSize))
    ,   DedicatedThread(&EntryPoint_, MakePtrRef(this))
    {}

    ~FSplashScreenDialog_() {
        DedicatedThread.join();

        const FThreadSafeData::FExclusiveLock exclusive = Data.LockExclusive();
        Unused(exclusive);

        ::GlobalFree(hGlobalAlloc);
    }

    template <typename _Cmd, decltype(std::declval<_Cmd&&>()(std::declval<FInternal&>()))* = nullptr>
    void QueueCommand_(_Cmd&& rcmd) {
        ::HWND hDialog = NULL; {
            const FThreadSafeData::FExclusiveLock exclusiveData = Data.LockExclusive();
            exclusiveData->Commands.EmplaceIt(std::move(rcmd));
            hDialog = exclusiveData->hDialog;
        }

        if (hDialog != NULL)
            ::SendMessage(hDialog, GNotifyCommandMessage, NULL, (::LPARAM)this);
    }

    static void EntryPoint_(FSplashScreenDialog_& dialog) {
        const FThreadContextStartup threadStartup("WindowsSplashScreen", PPE_THREADTAG_OTHER);

        threadStartup.Context().SetAffinityMask(FPlatformThread::SecondaryThreadAffinity());
        threadStartup.Context().SetPriority(EThreadPriority::Idle);

        ::LPDLGTEMPLATE const pDialogTemplate = CreateDialogTemplate_(dialog, L"PPE"_view);

        const ::INT_PTR ret = ::DialogBoxIndirectParamW(AppHandleWin32_(), pDialogTemplate, NULL, &DialogProc_, LPARAM(&dialog));
        {
            const FThreadSafeData::FExclusiveLock exclusiveData = dialog.Data.LockExclusive();

            if (exclusiveData->hBackgroundBitmap != NULL) {
                ::DeleteObject(exclusiveData->hBackgroundBitmap);
                exclusiveData->hBackgroundBitmap = NULL;
            }
            if (exclusiveData->hIcon != NULL) {
                ::DestroyIcon(exclusiveData->hIcon);
                exclusiveData->hIcon = NULL;
            }
            if (exclusiveData->hTextFont != NULL) {
                ::DeleteObject(exclusiveData->hTextFont);
                exclusiveData->hTextFont = NULL;
            }
            if (exclusiveData->hTitleFont != NULL) {
                ::DeleteObject(exclusiveData->hTitleFont);
                exclusiveData->hTitleFont = NULL;
            }
        }

        if (-1 == ret)
            PPE_THROW_IT(FLastErrorException("DialogBoxIndirectParamW"));
    }

    static LRESULT CALLBACK DialogProc_(::HWND hDialog, ::UINT dwMessage, ::WPARAM wParam, ::LPARAM lParam) {
        Unused(hDialog, dwMessage, wParam, lParam);

        switch (dwMessage) {
        case GNotifyCommandMessage:
        {
            Assert(lParam);
            FSplashScreenDialog_* const pDialog = bit_cast<FSplashScreenDialog_*>(lParam);

            const FThreadSafeData::FExclusiveLock exclusiveData = pDialog->Data.LockExclusive();

            if (exclusiveData->hDialog != NULL) {
                for (const TFunction<void(FInternal&)>& cmd : exclusiveData->Commands) {
                    cmd(*exclusiveData);
                }

                exclusiveData->Commands.Clear();
                return TRUE;
            }

            return FALSE;
        }
        case WM_INITDIALOG:
        {
            Assert(lParam);
            FSplashScreenDialog_* const pDialog = bit_cast<FSplashScreenDialog_*>(lParam);

            const FThreadSafeData::FExclusiveLock exclusiveData = pDialog->Data.LockExclusive();
            exclusiveData->hDialog = hDialog;

            // embed splash screen dialog pointer for later
            ::SetWindowLongPtr(hDialog, GWLP_USERDATA, ::LONG_PTR(pDialog));

            // load application icon
            const size_t appIcon = FCurrentProcess::Get().AppIcon();
            exclusiveData->hIcon = ::LoadIconW(AppHandleWin32_(), MAKEINTRESOURCEW(appIcon));

            // set application icon as window icon
            ::SendMessage(hDialog, WM_SETICON, 0, (LPARAM)exclusiveData->hIcon);

            // set application icon as splash screen logo
            if (::HWND const hStaticIcon = ::GetDlgItem(hDialog, DIALOG_ID_ICON))
                ::SendMessage(hStaticIcon, STM_SETICON, (WPARAM)exclusiveData->hIcon, 0);

            // change dialog font
            exclusiveData->hTextFont = ::CreateFont(
                -::MulDiv(9, ::GetDeviceCaps(::GetDC(hDialog), LOGPIXELSY), 72), // height of font
                0,              // average character width
                0,              // angle of escapement
                0,              // base-line orientation angle
                FW_LIGHT,       // font weight
                FALSE,          // italic attribute option
                FALSE,          // underline attribute option
                FALSE,          // strikeout attribute option
                ANSI_CHARSET,   // character set identifier
                OUT_TT_PRECIS,  // output precision
                CLIP_DEFAULT_PRECIS, // clipping precision
                DEFAULT_QUALITY,     // output quality
                DEFAULT_PITCH | FF_DONTCARE, // pitch and family
                TEXT("MS Shell DLG")); // typeface name

            if (::HWND const hSubText = ::GetDlgItem(hDialog, DIALOG_ID_TEXT)) {
                ::SendMessage(hSubText, WM_SETFONT, (WPARAM)exclusiveData->hTextFont, TRUE);
            }

            // change title text font
            if (::HWND const hTitleText = ::GetDlgItem(hDialog, DIALOG_ID_TITLE)) {
                exclusiveData->hTitleFont = ::CreateFont(
                    -::MulDiv(24, ::GetDeviceCaps(::GetDC(hTitleText), LOGPIXELSY), 72), // height of font
                    0,              // average character width
                    0,              // angle of escapement
                    0,              // base-line orientation angle
                    FW_BOLD,        // font weight
                    FALSE,          // italic attribute option
                    FALSE,          // underline attribute option
                    FALSE,          // strikeout attribute option
                    ANSI_CHARSET,   // character set identifier
                    OUT_TT_PRECIS,  // output precision
                    CLIP_DEFAULT_PRECIS, // clipping precision
                    DEFAULT_QUALITY,     // output quality
                    DEFAULT_PITCH | FF_DONTCARE, // pitch and family
                    TEXT("Tahoma")); // typeface name

                ::SendMessage(hTitleText, WM_SETFONT, (WPARAM)exclusiveData->hTitleFont, TRUE);
            }

            // use scrollbar arrow height as progress bar height
            exclusiveData->cyProgressBar = ::GetSystemMetrics(SM_CYVSCROLL);

            // retrieve dialog window client rect
            ::RECT rcClient;
            ::GetWindowRect(hDialog, &rcClient);

            // load and set splash-screen background image
            if (::HWND const hBackgroundImageView = ::GetDlgItem(hDialog, DIALOG_ID_BACKGROUND)) {
                const FWString splashScreenPath{ FCurrentProcess::Get().DataPath() + L"\\Icons\\splash_screen.bmp" };
                exclusiveData->hBackgroundBitmap = (::HBITMAP)::LoadImageW(NULL, *splashScreenPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
                ::SendMessage(hBackgroundImageView, STM_SETIMAGE, IMAGE_BITMAP, (::LPARAM)exclusiveData->hBackgroundBitmap);
            }

            // center window on screen
            ::SetWindowPos(hDialog,
                HWND_TOPMOST, // always on top
                (::GetSystemMetrics(SM_CXSCREEN) - rcClient.right) / 2,
                (::GetSystemMetrics(SM_CYSCREEN) - rcClient.bottom) / 2,
                std::abs(rcClient.left - rcClient.right),
                std::abs(rcClient.top - rcClient.bottom),
                0 );

            return TRUE;
        }
        case WM_CTLCOLORSTATIC:
        {
            const ::HWND hwndStatic = (HWND)lParam;
            const int controlID = GetDlgCtrlID(hwndStatic);
            /*if (controlID == DIALOG_ID_ICON)
                return FALSE;*/

            const ::HDC hdcStatic = (HDC)wParam;
            ::SetBkMode(hdcStatic, TRANSPARENT); // Make background transparent

            if (controlID != DIALOG_ID_TITLE)
                ::SetTextColor(hdcStatic, RGB(129, 103, 151)); // Set text color
            else if (controlID != DIALOG_ID_ICON)
                ::SetTextColor(hdcStatic, RGB(254, 243, 238)); // Set title color

            return (INT_PTR)::GetStockObject(NULL_BRUSH);
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam)) {
            case ResultToID_(FWindowsPlatformDialog::EResult::Ok):
            case ResultToID_(FWindowsPlatformDialog::EResult::Cancel):
            case ResultToID_(FWindowsPlatformDialog::EResult::Abort):
            case ResultToID_(FWindowsPlatformDialog::EResult::Retry):
            case ResultToID_(FWindowsPlatformDialog::EResult::Ignore):
            case ResultToID_(FWindowsPlatformDialog::EResult::Yes):
            case ResultToID_(FWindowsPlatformDialog::EResult::No):
            case ResultToID_(FWindowsPlatformDialog::EResult::TryAgain):
            case ResultToID_(FWindowsPlatformDialog::EResult::Continue):
            case ResultToID_(FWindowsPlatformDialog::EResult::IgnoreAlways):
                ::EndDialog(hDialog, LOWORD(wParam));
                return TRUE;
            }

            return FALSE;
        }}

        return ::DefWindowProc(hDialog, dwMessage, wParam, lParam);
    }

    STATIC_CONST_INTEGRAL(u32, cyFrameBorder, 5);

    static void CreateProgressBar_(FProgressState* pPbar, ::HWND hDialog, int cyProgressBar) {
        ::HINSTANCE const hAppInstance = AppHandleWin32_();

        // retrieve dialog window client rect
        ::RECT rcClient;
        ::GetWindowRect(hDialog, &rcClient);

        // stack the progress bar above existing ones IFN
        rcClient.bottom -= checked_cast<::LONG>(pPbar->YLevel * (cyProgressBar * 2) + cyFrameBorder - 2);

        const int width = std::abs(rcClient.right - rcClient.left);
        const int height = std::abs(rcClient.top - rcClient.bottom);

        // detect if progress bar knows before handle total amount of work to do
        const bool bMarquee = (pPbar->Total == 0);

        // create progress text above the progress bar
        pPbar->hStaticLabel = ::CreateWindowExW(0, L"Static", *pPbar->Text,
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            cyFrameBorder + 2,
            height - cyProgressBar * 2,
            width - cyFrameBorder * 2 - 4,
            cyProgressBar,
            hDialog, NULL, hAppInstance, NULL);

        // create progress bar at bottom of dialog window
        pPbar->hProgressBar = ::CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR) NULL,
            WS_CHILD | WS_VISIBLE | PBS_SMOOTH | (bMarquee ? PBS_MARQUEE : 0),
            cyFrameBorder,
            height - cyProgressBar,
            width - cyFrameBorder * 2 - 2/* progress bar border is `outset` */,
            cyProgressBar - cyFrameBorder,
            hDialog, NULL, hAppInstance, NULL);

        // initialize progress bar range
        ::SendMessage(pPbar->hProgressBar, PBM_SETPOS, 0, 0);
        if (bMarquee)
            ::SendMessage(pPbar->hProgressBar, PBM_SETMARQUEE, TRUE, 0);
        else
            ::SendMessage(pPbar->hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, pPbar->Total));
    }

    NODISCARD static ::LPDLGTEMPLATE CreateDialogTemplate_(FSplashScreenDialog_& dialog, const FWStringView& title) {
        const FThreadSafeData::FExclusiveLock exclusive = dialog.Data.LockExclusive();
        AssertRelease(NULL != dialog.hGlobalAlloc);

        FMemoryViewWriter writer(MakeRawView(::GlobalLock(dialog.hGlobalAlloc), GAllocSize));
        DEFERRED { ::GlobalUnlock(dialog.hGlobalAlloc); };

        ::LPDLGTEMPLATE const tpl = bit_cast<::LPDLGTEMPLATE>(writer.Eat(sizeof(::DLGTEMPLATE)).Pointer());
        tpl->x = 10;
        tpl->y = 10;
        tpl->cx = 400;
        tpl->cy = 100;
        tpl->style = WS_POPUP | WS_BORDER;
        tpl->cdit = 0;

        writer.WritePOD(::WORD(0)); // No menu
        writer.WritePOD(::WORD(EAtomClass_::Predefined)); // default class
        Template_AddCaption_(writer, L"PPE - Splash Screen"_view); // window name

        //writer.WritePOD(::WORD(9)); // Font size
        //Template_AddCaption_(writer, L"Arial"_view); // Font name

        // background bitmap
        Template_AddItem_(writer, 1, 1, tpl->cx - 2, tpl->cy -2,
            DIALOG_ID_BACKGROUND,
            WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_REALSIZECONTROL,
            EAtomClass_::Static);

        writer.WritePOD(::WORD(0)); // no caption data
        writer.WritePOD(::WORD(0)); // no creation data
        tpl->cdit++;

        // splash screen header icon
        Template_AddItem_(writer, 0, 0, SM_CXSMICON, SM_CYSMICON,
            DIALOG_ID_ICON,
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER | SS_ICON | SS_REALSIZECONTROL,
            EAtomClass_::Static);

        writer.WritePOD(::WORD(0)); // no caption data
        writer.WritePOD(::WORD(0)); // no creation data
        tpl->cdit++;

        wchar_t titleBuf[512];
        const FModularDomain& domain = FModularDomain::Get();
        constexpr FBuildVersion ver = CurrentBuildVersion();

        // splash screen header title
        Template_AddItem_(writer, SM_CXSMICON, cyFrameBorder, tpl->cx - SM_CXSMICON - cyFrameBorder * 2, SM_CYSMICON / 2 - cyFrameBorder,
            DIALOG_ID_TITLE,
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            EAtomClass_::Static);

        Format(titleBuf, L"{} - {}", title, domain.Name());

        Template_AddCaption_(writer, MakeCStringView(titleBuf));
        writer.WritePOD(::WORD(0)); // no creation data
        tpl->cdit++;

        // splash screen header text
        Template_AddItem_(writer, SM_CXSMICON, SM_CYSMICON / 2, tpl->cx - SM_CXSMICON - cyFrameBorder * 2, SM_CYSMICON / 2,
            DIALOG_ID_TEXT,
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            EAtomClass_::Static);

        Format(titleBuf,
            L"{} - {}\r\n"
            L"rev. {}",
            ver.Family, ver.Compiler,
            ver.Revision);

        Template_AddCaption_(writer, MakeCStringView(titleBuf));
        writer.WritePOD(::WORD(0)); // no creation data
        tpl->cdit++;

        return static_cast<::LPDLGTEMPLATE>(dialog.hGlobalAlloc);
    }
};
//----------------------------------------------------------------------------
TThreadSafe<TPtrRef<FSplashScreenDialog_>, EThreadBarrier::RWLock> FSplashScreenDialog_::GInstance_{};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
auto FWindowsPlatformDialog::Show(const FWStringView& text, const FWStringView& caption, EType dialogType, EIcon iconType) -> EResult {
    Assert(not text.empty());
    Assert(not caption.empty());

    return Template_CreateDialogBox_(iconType, dialogType, text, caption);
}
//----------------------------------------------------------------------------
auto FWindowsPlatformDialog::FileDialog(const FConstWChar& sourceFile, u32 sourceLine, const FWStringView& text, const FWStringView& caption, EType dialogType, EIcon iconType) -> EResult {
    Assert(sourceFile.Data);
    Assert(not text.empty());
    Assert(not caption.empty());

    return Template_CreateDialogBox_(iconType, dialogType, text, caption, sourceFile, sourceLine);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformDialog::PushSplashScreen_ReturnIfOpened() {
    return FSplashScreenDialog_::PushOpen();
}
//----------------------------------------------------------------------------
bool FWindowsPlatformDialog::PopSplashScreen_ReturnIfOpened() {
    return FSplashScreenDialog_::PopClose();
}
//----------------------------------------------------------------------------
auto FWindowsPlatformDialog::BeginProgress(const FWStringView& text, size_t total/* = 0.f */) -> FDialogHandle {
    if (const TPtrRef<FSplashScreenDialog_> dialog = FSplashScreenDialog_::GetIFP())
        return static_cast<size_t>(dialog->BeginProgress(text, total, 0));
    return Zero;
}
//----------------------------------------------------------------------------
void FWindowsPlatformDialog::IncProgressPos(FDialogHandle progress) {
    if (const TPtrRef<FSplashScreenDialog_> dialog = FSplashScreenDialog_::GetIFP())
        dialog->IncProgressPos(FSparseDataId{progress});
}
//----------------------------------------------------------------------------
void FWindowsPlatformDialog::SetProgressPos(FDialogHandle progress, size_t amount) {
    if (const TPtrRef<FSplashScreenDialog_> dialog = FSplashScreenDialog_::GetIFP())
        dialog->SetProgressPos(FSparseDataId{progress}, amount);
}
//----------------------------------------------------------------------------
void FWindowsPlatformDialog::SetProgressText(FDialogHandle progress, const FWStringView& message) {
    if (const TPtrRef<FSplashScreenDialog_> dialog = FSplashScreenDialog_::GetIFP())
        dialog->SetProgressText(FSparseDataId{progress}, message);
}
//----------------------------------------------------------------------------
void FWindowsPlatformDialog::EndProgress(FDialogHandle progress) {
    if (const TPtrRef<FSplashScreenDialog_> dialog = FSplashScreenDialog_::GetIFP())
        dialog->EndProgress(FSparseDataId{progress});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
