#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformNotification.h"

#ifdef PLATFORM_WINDOWS

#include "Container/SparseArray.h"
#include "Container/StringHashMap.h"
#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformApplication.h"
#include "HAL/PlatformIncludes.h"
#include "HAL/Windows/ComPtr.h"
#include "HAL/Windows/LastError.h"
#include "Thread/AtomicSpinLock.h"

#include <shellapi.h>
#include <Shobjidl.h>

namespace PPE {
namespace Application {
LOG_CATEGORY(, Notification)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const ::GUID GWindowsSystrayGUID_ = { 0x5031c0b4, 0x7d30, 0x4b3a, { 0x80, 0xee, 0xce, 0x1, 0xee, 0x23, 0x9d, 0xc1 } };
static ::BOOL GWindowsSystrayVisible_ = FALSE;
static TComPtr<::ITaskbarList3> GWindowsTaskBar_;
static FAtomicSpinLock& WindowsTaskBarCS_() {
    ONE_TIME_DEFAULT_INITIALIZE(FAtomicSpinLock, GInstance);
    return GInstance;
}
//----------------------------------------------------------------------------
struct FWindowsSystrayCmds_ {
    static FWindowsSystrayCmds_& Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FWindowsSystrayCmds_, GInstance);
        return GInstance;
    }

    struct FUserCmd {
        FWString Category;
        FWString Label;
        FWindowsPlatformNotification::FSystrayDelegate Delegate;

        FUserCmd(
            const FWStringView& category,
            const FWStringView& label,
            FWindowsPlatformNotification::FSystrayDelegate&& cmd )
        :   Category(category)
        ,   Label(label)
        ,   Delegate(cmd)
        {}
    };

    SPARSEARRAY(Window, FUserCmd, 16) UserCmds;

    static ::UINT IDI_From_UserCmd(FSparseDataId index) {
        return checked_cast<::UINT>(1000 + index);
    }

    static FSparseDataId IDI_To_UserCmd(::UINT idi) {
        Assert(idi >= 1000);
        return checked_cast<FSparseDataId>(idi - 1000);
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// https://chgi.developpez.com/windows/trayicon/
void FWindowsPlatformNotification::ShowSystray() {
    Assert(not GWindowsSystrayVisible_);

    ::NOTIFYICONDATAW nid;
    ::SecureZeroMemory(&nid, sizeof(nid));

    nid.cbSize = sizeof(nid);
    nid.guidItem = GWindowsSystrayGUID_;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID | NIF_TIP;
    nid.uCallbackMessage = FWindowsPlatformNotification::WM_SYSTRAY;

    const FCurrentProcess& process = FCurrentProcess::Get();
    nid.hIcon = ::LoadIcon(NULL, MAKEINTRESOURCEW(process.AppIcon()));

    const FPlatformApplication& app = FPlatformApplication::Get();
    nid.hWnd = NULL;// #TODO (app.MainWindow() ? app.MainWindow()->NativeHandle() : NULL);
    Assert(app.Name().size() < ARRAYSIZE(nid.szTip));
    ::memcpy(&nid.szTip, app.Name().data(), app.Name().size() * sizeof(wchar_t));

    LOG(Notification, Info, L"show systray icon");

    const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());

    if (not ::Shell_NotifyIconW(NIM_ADD, &nid))
        PPE_THROW_IT(FLastErrorException("Shell_NotifyIconW"));

    GWindowsSystrayVisible_ = TRUE;
}
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::HideSystray() {
    Assert(GWindowsSystrayVisible_);

    ::NOTIFYICONDATAW nid;
    ::SecureZeroMemory(&nid, sizeof(nid));

    nid.cbSize = sizeof(nid);
    nid.guidItem = GWindowsSystrayGUID_;
    nid.uFlags = NIF_STATE | NIF_GUID;
    nid.dwState = NIS_HIDDEN;

    LOG(Notification, Info, L"hide systray icon");

    const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());

    if (not ::Shell_NotifyIconW(NIM_DELETE, &nid))
        PPE_THROW_IT(FLastErrorException("Shell_NotifyIconW"));

    GWindowsSystrayVisible_ = FALSE;
}
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::NotifySystray(ENotificationIcon icon, const FWStringView& title, const FWStringView& text) {
    Assert(not title.empty());
    Assert(not text.empty());
    Assert(GWindowsSystrayVisible_);

    ::NOTIFYICONDATAW nid;
    ::SecureZeroMemory(&nid, sizeof(nid));

    nid.cbSize = sizeof(nid);
    nid.guidItem = GWindowsSystrayGUID_;
    nid.uFlags = NIF_INFO | NIF_SHOWTIP | NIF_GUID;

    switch (icon) {
    case ENotificationIcon::None:
        nid.dwInfoFlags = NIIF_NONE;
        LOG(Notification, Debug, L"notify systray : {0} -- {1}", title, text);
        break;
    case ENotificationIcon::Info:
        nid.dwInfoFlags = NIIF_INFO;
        LOG(Notification, Info, L"notify systray : {0} -- {1}", title, text);
        break;
    case ENotificationIcon::Warning:
        nid.dwInfoFlags = NIIF_WARNING;
        LOG(Notification, Warning, L"notify systray : {0} -- {1}", title, text);
        break;
    case ENotificationIcon::Error:
        nid.dwInfoFlags = NIIF_ERROR;
        LOG(Notification, Error, L"notify systray : {0} -- {1}", title, text);
        break;
    }

    Assert(title.size() < ARRAYSIZE(nid.szInfoTitle));
    ::SecureZeroMemory(&nid.szInfoTitle, sizeof(nid.szInfoTitle));
    ::memcpy(&nid.szInfoTitle, title.data(), title.SizeInBytes());

    Assert(text.size() < ARRAYSIZE(nid.szInfo));
    ::SecureZeroMemory(&nid.szInfo, sizeof(nid.szInfo));
    ::memcpy(&nid.szInfo, text.data(), text.SizeInBytes());

    const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());

    if (not ::Shell_NotifyIconW(NIM_MODIFY, &nid))
        PPE_THROW_IT(FLastErrorException("Shell_NotifyIconW"));
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformNotification::AddSystrayCommand(
    const FWStringView& category,
    const FWStringView& label,
    FSystrayDelegate&& cmd ) {
    Assert(not category.empty());
    Assert(not label.empty());
    Assert(cmd);

    FSparseDataId cmdIndex = INDEX_NONE;
    {
        const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());
        cmdIndex = FWindowsSystrayCmds_::Get().UserCmds.Emplace(category, label, std::move(cmd));
    }
    Assert(INDEX_NONE != cmdIndex);

    LOG(Notification, Debug, L"add systray command <{0}/{1}> -> #{2}", category, label, cmdIndex);

    return cmdIndex;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformNotification::RemoveSystrayCommand(size_t index) {
    Assert(index != INDEX_NONE);

    LOG(Notification, Debug, L"remove systray command #{0}", index);

    const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());

    return FWindowsSystrayCmds_::Get().UserCmds.Remove(index);
}
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::SetTaskbarState(ETaskbarState state) {
    Assert(GWindowsTaskBar_);

    const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());

    ::TBPFLAG tbpFlags;

    switch (state) {
    case ETaskbarState::Normal:
        tbpFlags = TBPF_NORMAL;
        break;
    case ETaskbarState::Progress:
        tbpFlags = TBPF_NORMAL;
        break;
    case ETaskbarState::NoProgress:
        tbpFlags = TBPF_NOPROGRESS;
        break;
    case ETaskbarState::Paused:
        tbpFlags = TBPF_PAUSED;
        break;
    case ETaskbarState::Error:
        tbpFlags = TBPF_ERROR;
        break;
    case ETaskbarState::Indeterminate:
        tbpFlags = TBPF_INDETERMINATE;
        break;
    default:
        AssertNotImplemented();
    }

    if (not SUCCEEDED(GWindowsTaskBar_->SetProgressState(NULL, tbpFlags)))
        PPE_THROW_IT(FLastErrorException("SetProgressState"));
}
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::SetTaskbarProgress(size_t completed, size_t total) {
    Assert(GWindowsTaskBar_);
    Assert(completed <= total);

    const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());

    const ::ULONGLONG ullCompleted = checked_cast<::ULONGLONG>(completed);
    const ::ULONGLONG ullTotal = checked_cast<::ULONGLONG>(total);

    if (not SUCCEEDED(GWindowsTaskBar_->SetProgressValue(NULL, ullCompleted, ullTotal)))
        PPE_THROW_IT(FLastErrorException("SetProgressValue"));
}
//----------------------------------------------------------------------------
// http://gxuanchung.blogspot.com/2010/08/win32-create-window-menu.html
void FWindowsPlatformNotification::SummonSystrayPopupMenuWin32(::HWND hWnd) {
    Assert(GWindowsTaskBar_);

    const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());

    // create popup menu from systray commands :
    ::HMENU const hSystrayPopup = ::CreatePopupMenu();

    WSTRING_HASHMAP(Window, ::HMENU, ECase::Insensitive) subMenus;

    const auto& userCmds = FWindowsSystrayCmds_::Get().UserCmds;
    for (const FWindowsSystrayCmds_::FUserCmd& userCmd : userCmds) {
        ::HMENU hSubPopup = NULL;
        if (not TryGetValue(subMenus, userCmd.Category, &hSubPopup)) {
            Assert(NULL == hSubPopup);
            hSubPopup = ::CreatePopupMenu();
            Assert(hSubPopup);
            Verify(::AppendMenuW(hSystrayPopup, MF_STRING | MF_POPUP, (::UINT_PTR)hSubPopup, userCmd.Category.data()));
            subMenus.Add(userCmd.Category) = hSubPopup;
        }
        Assert(hSubPopup);

        const FSparseDataId cmdIndex = userCmds.IndexOf(userCmd);
        const ::UINT cmdIDI = FWindowsSystrayCmds_::IDI_From_UserCmd(cmdIndex);

        Verify(::AppendMenuW(hSubPopup, MF_STRING, (::UINT_PTR)cmdIDI, userCmd.Label.data()));
    }

    // show popup as immediate :
    ::POINT mousePos;
    Verify(::GetCursorPos(&mousePos));

    if (hWnd)
        ::SetForegroundWindow(hWnd);

    const ::UINT cmdIDI = ::TrackPopupMenuEx(
        hSystrayPopup,
        TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
        mousePos.x, mousePos.y,
        hWnd,
        NULL );

    // destroy menus :
    if (not ::DestroyMenu(hSystrayPopup)) // DestroyMenu is recursive
        PPE_THROW_IT(FLastErrorException("DestroyMenu"));

    // run user command if any was selected :
    if (cmdIDI) {
        const FSparseDataId cmdIndex = FWindowsSystrayCmds_::IDI_To_UserCmd(cmdIDI);
        const FWindowsSystrayCmds_::FUserCmd* const pUserCmd = userCmds.Find(cmdIndex);
        AssertRelease(pUserCmd);

        LOG(Notification, Emphasis, L"launch systray command <{0}/{1}>", pUserCmd->Category, pUserCmd->Label);

        pUserCmd->Delegate();
    }
}
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::Start() {
    Assert(NULL == GWindowsTaskBar_);

    const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());

    ::CoInitialize(NULL);

    ::ITaskbarList3* taskbar = nullptr;
    if (not SUCCEEDED(::CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (void **)&taskbar)))
        PPE_THROW_IT(FLastErrorException("CoCreateInstance"));

    AssertRelease(taskbar);
    GWindowsTaskBar_.Reset(taskbar);
}
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::Shutdown() {
    Assert(GWindowsTaskBar_);

    const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());

    GWindowsTaskBar_.Reset();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
