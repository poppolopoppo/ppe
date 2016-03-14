#include "stdafx.h"

#include "DialogBox.h"

#include "Callstack.h"
#include "CurrentProcess.h"
#include "DecodedCallstack.h"
#include "LastError.h"
#include "Logger.h"
#include "MiniDump.h"

#include "Container/Vector.h"
#include "IO/FileSystem.h"
#include "IO/StringSlice.h"
#include "IO/VirtualFileSystem.h"
#include "Memory/MemoryProvider.h"

#ifdef OS_WINDOWS
#   include <Windows.h>
#else
#   error "no support"
#endif

namespace Core {
namespace DialogBox {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef OS_WINDOWS
namespace {
//----------------------------------------------------------------------------
static LPCWSTR SystemIcon_(DialogBox::Icon iconType) {
    switch (iconType)
    {
    case Core::DialogBox::Icon::Hand:
        return IDI_HAND;
    case Core::DialogBox::Icon::Question:
        return IDI_QUESTION;
    case Core::DialogBox::Icon::Exclamation:
        return IDI_EXCLAMATION;
    case Core::DialogBox::Icon::Aterisk:
        return IDI_ASTERISK;
    case Core::DialogBox::Icon::Error:
        return IDI_ERROR;
    case Core::DialogBox::Icon::Warning:
        return IDI_WARNING;
    case Core::DialogBox::Icon::Information:
        return IDI_INFORMATION;
    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
static WStringSlice ResultCaption_(DialogBox::Result result) {
    switch (result)
    {
    case Core::DialogBox::Result::Ok:
        return L"Ok";
    case Core::DialogBox::Result::Cancel:
        return L"Cancel";
    case Core::DialogBox::Result::Abort:
        return L"Abort";
    case Core::DialogBox::Result::Retry:
        return L"Retry";
    case Core::DialogBox::Result::Ignore:
        return L"Ignore";
    case Core::DialogBox::Result::Yes:
        return L"Yes";
    case Core::DialogBox::Result::No:
        return L"No";
    case Core::DialogBox::Result::TryAgain:
        return L"Try Again";
    case Core::DialogBox::Result::Continue:
        return L"Continue";
    case Core::DialogBox::Result::IgnoreAlways:
        return L"Ignore Always";
    default:
        AssertNotImplemented();
    }
    return WStringSlice();
}
//----------------------------------------------------------------------------
static void SetClipboard_(HWND hwndDlg, const WStringSlice& content)
{
    HANDLE handle = (HANDLE)::GlobalAlloc(GHND|GMEM_ZEROINIT, (content.size()+2)*sizeof(wchar_t));
    if(handle != NULL)
    {
        wchar_t *pData = (wchar_t *) ::GlobalLock((HGLOBAL)handle);
        wcsncpy(pData, content.Pointer(), content.size());
        ::GlobalUnlock((HGLOBAL)handle);

        ::OpenClipboard(hwndDlg);
        ::EmptyClipboard();
        ::SetClipboardData(CF_UNICODETEXT, handle);
        ::CloseClipboard();
    }
    ::GlobalFree(handle);
}
//----------------------------------------------------------------------------
enum class AtomClass_ {
    Predefined  = 0,
    Button      = 0x0080,
    Edit        = 0x0081,
    Static      = 0x0082,
    ListBox     = 0x0083,
    ScrollBar   = 0x0084,
    ComboBox    = 0x0085,
};
//----------------------------------------------------------------------------
static constexpr size_t ID_TEXT         = 41001;
static constexpr size_t ID_STACK        = 41002;
static constexpr size_t ID_MINIDUMP     = 41003;
static constexpr size_t ID_COPY         = 41005;
static constexpr size_t ID_BREAK        = 41006;
static constexpr size_t ID_ICON         = 41007;
static constexpr size_t ResultToID_(DialogBox::Result button) { return 40000+size_t(button); }
static constexpr DialogBox::Result IDToResult_(size_t id) { return DialogBox::Result(id-40000); }
//----------------------------------------------------------------------------
static void Template_AddItem_(
    MemoryViewWriter& writer,
    size_t x, size_t y,
    size_t cx, size_t cy,
    size_t id, size_t style,
    AtomClass_ klass ) {
    const auto eaten = writer.EatAligned(sizeof(::DLGITEMTEMPLATE), sizeof(DWORD));
    Assert(IS_ALIGNED(sizeof(DWORD), eaten.Pointer()));
    ::LPDLGITEMTEMPLATE const tpl = reinterpret_cast<::LPDLGITEMTEMPLATE>(eaten.Pointer());

    tpl->x = checked_cast<short>(x);
    tpl->y = checked_cast<short>(y);
    tpl->cx = checked_cast<short>(cx);
    tpl->cy = checked_cast<short>(cy);
    tpl->id = checked_cast<WORD>(id);
    tpl->style = checked_cast<DWORD>(style);

    if (klass == AtomClass_::Predefined) {
        writer.WritePOD(u32(0));
    }
    else {
        writer.WritePOD(u16(0xFFFF));
        writer.WritePOD(u16(klass));
    }
}
//----------------------------------------------------------------------------
static void Template_AddCaption_(MemoryViewWriter& writer, const WStringSlice& caption ) {
    writer.Write(caption.Pointer(), caption.SizeInBytes());
    if (caption.back() != L'\0')
        writer.WritePOD(L'\0');
}
//----------------------------------------------------------------------------
struct Template_DialogContext_ {
    WStringSlice Text;
    WStringSlice Caption;
    DialogBox::Type Buttons;
    LPCWSTR IconId;
    HICON IconResource;
    DecodedCallstack DecodedCallstack;
    MemoryView<const WString> CallstackFrames;
};
//----------------------------------------------------------------------------
static LRESULT CALLBACK Template_StackProc_(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNUSED(wParam);
    UNUSED(lParam);
    switch (message)
    {
        case WM_LBUTTONDBLCLK:
            {
                HWND window = ::GetParent(hwndDlg);
                LRESULT index = ::SendDlgItemMessage(window, ID_STACK, LB_GETCURSEL, 0, 0);
                if(index != LB_ERR)
                {
                    const Template_DialogContext_* ctx = reinterpret_cast<const Template_DialogContext_*>(::GetWindowLongPtr(window, GWLP_USERDATA));
                    Assert(ctx);

                    LOG(Info, L"double click on frame : >>> {0} <<<", ctx->CallstackFrames[index]);

                    String cmdLine;
                    Format(cmdLine, "\"{0}\" \"{1}:{2}\"",
                        L"C:\\Program Files\\Sublime Text 3\\sublime_text.exe", // TODO: handle other editors ?
                        ctx->DecodedCallstack.Frames()[index].Filename(),
                        ctx->DecodedCallstack.Frames()[index].Line() );

                    ::WinExec(cmdLine.c_str(), SW_SHOW);
                }
            }
            break;

        default:
            {
                ::WNDPROC prevProc = reinterpret_cast<::WNDPROC>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
                Assert(prevProc);

                return ::CallWindowProc(prevProc, hwndDlg, message, wParam, lParam);
            }
    }
    return FALSE;
}
//----------------------------------------------------------------------------
static LRESULT CALLBACK Template_DialogProc_(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message)
    {
    case WM_INITDIALOG:
        {
            Assert(lParam);
            const Template_DialogContext_* ctx = reinterpret_cast<const Template_DialogContext_*>(lParam);

            ::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)ctx);

            ::SendMessage(hwndDlg, WM_SETICON, 0, (LPARAM)ctx->IconResource);

            ::HWND icon = ::GetDlgItem(hwndDlg, ID_ICON);
            Assert(icon);
            ::SendMessage(icon, STM_SETICON, (WPARAM)ctx->IconResource, 0);

            ::HWND stack = ::GetDlgItem(hwndDlg, ID_STACK);
            Assert(stack);
            for (const WString& str : ctx->CallstackFrames)
                ::SendMessageW(stack, LB_ADDSTRING, 0, (LPARAM)str.c_str());
            ::SendMessage(stack, LB_SETHORIZONTALEXTENT, 4096, 0);
            ::WNDPROC prevStackProc = (WNDPROC)::SetWindowLongPtr(stack, GWLP_WNDPROC, (LPARAM)Template_StackProc_);
            Assert(prevStackProc);
            ::SetWindowLongPtr(stack, GWLP_USERDATA, (LONG_PTR)prevStackProc);

            ::RECT rect;
            ::GetWindowRect(hwndDlg, &rect);
            Assert(rect.left < rect.right);
            Assert(rect.top < rect.bottom);
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
        case ResultToID_(DialogBox::Result::Ok):
        case ResultToID_(DialogBox::Result::Cancel):
        case ResultToID_(DialogBox::Result::Abort):
        case ResultToID_(DialogBox::Result::Retry):
        case ResultToID_(DialogBox::Result::Ignore):
        case ResultToID_(DialogBox::Result::Yes):
        case ResultToID_(DialogBox::Result::No):
        case ResultToID_(DialogBox::Result::TryAgain):
        case ResultToID_(DialogBox::Result::Continue):
        case ResultToID_(DialogBox::Result::IgnoreAlways):
            ::EndDialog(hwndDlg, LOWORD(wParam));
            return TRUE;

        case ID_COPY:
            {
                const Template_DialogContext_* ctx = reinterpret_cast<const Template_DialogContext_*>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
                Assert(ctx);

                ThreadLocalWOStringStream oss;
                oss << ctx->Text << L"\r\n"
                    << L"\r\n"
                    << L"----------------------------------------------------------------\r\n"
                    << L"Callstack:" << L"\r\n"
                    << L"----------------------------------------------------------------\r\n";

                for (const WString& frame : ctx->CallstackFrames)
                    oss << frame << L"\r\n";

                SetClipboard_(hwndDlg, MakeStringSlice(oss.str()));
            }
            return TRUE;

        case ID_BREAK:
            {
                ::DebugBreak();
            }
            return TRUE;

        case ID_MINIDUMP:
            {
                const Filename process(CurrentProcess::Instance().FileName());
                const Filename vfs(process.Dirpath(),
                    VirtualFileSystem::TemporaryBasename(process.BasenameNoExt().c_str(), L".dmp"));

                const WString path = vfs.ToWString();

                MiniDump::Write(path.c_str(), MiniDump::InfoLevel::Large);

                ::MessageBoxExW(hwndDlg, path.c_str(), L"Core dumped", MB_OK|MB_ICONASTERISK, 0);

                SetClipboard_(hwndDlg, MakeStringSlice(path));
            }
            return TRUE;
        }
    }
    return FALSE;
}
//----------------------------------------------------------------------------
static constexpr DialogBox::Result gTemplate_AllButtons[] = {
    DialogBox::Result::Ok, DialogBox::Result::Retry, DialogBox::Result::Ignore, DialogBox::Result::Yes,
    DialogBox::Result::TryAgain, DialogBox::Result::Continue, DialogBox::Result::IgnoreAlways,
    DialogBox::Result::No, DialogBox::Result::Cancel, DialogBox::Result::Abort,
};
//----------------------------------------------------------------------------
static DialogBox::Result Template_CreateDialogBox_(
    DialogBox::Icon icon,
    DialogBox::Type buttons,
    const WStringSlice& text,
    const WStringSlice& caption ) {

    Template_DialogContext_ ctx;
    ctx.Text = text;
    ctx.Caption = caption;
    ctx.Buttons = buttons;
    ctx.IconId = SystemIcon_(icon);
    ctx.IconResource = ::LoadIconW(nullptr, ctx.IconId);

    VECTORINSITU(Diagnostic, WString, Callstack::MaxDeph) callstackFrames;
    {
        Callstack callstack;
        Callstack::Capture(&callstack, 4, Callstack::MaxDeph);

        ctx.DecodedCallstack = DecodedCallstack(callstack);
        callstackFrames.reserve(ctx.DecodedCallstack.Frames().size());

        STACKLOCAL_WOCSTRSTREAM(tmp, 4096);
        for (const DecodedCallstack::Frame& frame : ctx.DecodedCallstack.Frames()) {
            tmp << frame.Filename() << L'(' << frame.Line() << L"): " << frame.Symbol();
            callstackFrames.emplace_back(tmp.NullTerminatedStr());
            tmp.Reset();
        }
    }
    ctx.CallstackFrames = callstackFrames.MakeConstView();

    static constexpr size_t gAllocSize = 8192;

    HGLOBAL const hgbl = ::GlobalAlloc(GMEM_ZEROINIT, gAllocSize);
    Assert(nullptr != hgbl);
    {
        constexpr size_t buttonWidthPerChar = 4;
        constexpr size_t buttonWidthPadding = 3;
        constexpr size_t buttonHeight = 15;

        MemoryViewWriter writer(MemoryView<u8>((u8*)::GlobalLock(hgbl), gAllocSize));

        const auto eaten = writer.Eat(sizeof(::DLGTEMPLATE));
        Assert(IS_ALIGNED(sizeof(DWORD), eaten.Pointer()));
        ::LPDLGTEMPLATE const tpl = reinterpret_cast<::LPDLGTEMPLATE>(eaten.Pointer());

        tpl->x = 10;
        tpl->y = 10;
        tpl->cx = 400;
        tpl->cy = 200;
        tpl->style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION | DS_SETFONT;
        tpl->cdit = 0;

        writer.WritePOD(WORD(0)); // No menu
        writer.WritePOD(WORD(AtomClass_::Predefined)); // default

        Template_AddCaption_(writer, caption);

        writer.WritePOD(WORD(9)); // Font size
        Template_AddCaption_(writer, L"Segoe UI"); // Font name

        // modal buttons
        const size_t buttonTop = tpl->cy - 5 - buttonHeight;
        size_t buttonRight = tpl->cx - 2;

        for (DialogBox::Result button : gTemplate_AllButtons) {
            if (((size_t)1<<(size_t)button) != (((size_t)1<<(size_t)button) & (size_t)buttons))
                continue;

            const WStringSlice text = ResultCaption_(button);

            const size_t w = buttonWidthPadding*2 + buttonWidthPerChar*text.size();
            buttonRight -= buttonWidthPadding + w;

            Template_AddItem_(writer, buttonRight, buttonTop, w, buttonHeight,
                ResultToID_(button),
                WS_CHILD | WS_VISIBLE, AtomClass_::Button);
            Template_AddCaption_(writer, text);

            writer.WritePOD(WORD(0));

            tpl->cdit++;
        }

        Template_AddItem_(writer, 5, buttonTop, 32, buttonHeight, ID_COPY,
            WS_CHILD | WS_VISIBLE, AtomClass_::Button);
        Template_AddCaption_(writer, L"Copy");
        writer.WritePOD(WORD(0));
        tpl->cdit++;

        Template_AddItem_(writer, 40, buttonTop, 32, buttonHeight, ID_BREAK,
            WS_CHILD | WS_VISIBLE, AtomClass_::Button);
        Template_AddCaption_(writer, L"Break");
        writer.WritePOD(WORD(0));
        tpl->cdit++;

        Template_AddItem_(writer, 75, buttonTop, 55, buttonHeight, ID_MINIDUMP,
            WS_CHILD | WS_VISIBLE, AtomClass_::Button);
        Template_AddCaption_(writer, L"Minidump");
        writer.WritePOD(WORD(0));
        tpl->cdit++;

        Template_AddItem_(writer, 15, 10, 32, 32, ID_ICON,
            WS_CHILD | WS_VISIBLE | SS_ICON | SS_LEFT, AtomClass_::Static);
        writer.WritePOD(WORD(0));
        writer.WritePOD(WORD(0));
        tpl->cdit++;

        Template_AddItem_(writer, 45, 8, 350, 47, ID_TEXT,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY, AtomClass_::Edit);
        Template_AddCaption_(writer, text);
        writer.WritePOD(WORD(0));
        tpl->cdit++;

        Template_AddItem_(writer, 5, 5+50+5, 390, 127, ID_STACK,
            WS_BORDER | WS_HSCROLL | WS_VSCROLL | WS_CHILD | WS_VISIBLE, AtomClass_::ListBox);
        writer.WritePOD(WORD(0));
        tpl->cdit++;
    }
    ::GlobalUnlock(hgbl);

    const INT_PTR ret = DialogBoxIndirectParamW(NULL, (::LPDLGTEMPLATE)hgbl, NULL, (::DLGPROC)Template_DialogProc_, (LPARAM)&ctx);
    if (-1 == ret) {
        const WString lastError = GetLastErrorToWString(GetLastError());
        LOG(Error, L"Failed to create dialog box : {0}", lastError);
        AssertNotReached();
    }

    ::GlobalFree(hgbl);

    return (DialogBox::Result)IDToResult_(ret);
}
//----------------------------------------------------------------------------
} //!namespace
#endif //!OS_WINDOWS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DialogBox::Result Show(const WStringSlice& text, const WStringSlice& caption, DialogBox::Type dialogType, DialogBox::Icon iconType)
{
    Assert(not text.empty());
    Assert(not caption.empty());
    return Template_CreateDialogBox_(iconType, dialogType, text, caption);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DialogBox
} //!namespace Core
