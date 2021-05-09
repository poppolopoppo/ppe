#include "stdafx.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformNotification.h"

#include "Application.h"
#include "Application/ApplicationBase.h"

#include "Container/SparseArray.h"
#include "Container/Stack.h"
#include "Container/StringHashMap.h"
#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformApplication.h"
#include "HAL/PlatformIncludes.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformWindow.h"
#include "HAL/Windows/ComPtr.h"
#include "HAL/Windows/LastError.h"
#include "Meta/AlignedStorage.h"
#include "Meta/Singleton.h"
#include "Misc/Function.h"
#include "Thread/AtomicSpinLock.h"
#include "Thread/ThreadContext.h"

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
class FWindowsSystrayCmds_ : Meta::TStaticSingleton<FWindowsSystrayCmds_> {
    friend Meta::TStaticSingleton<FWindowsSystrayCmds_>;
public:
    using singleton_type = Meta::TStaticSingleton<FWindowsSystrayCmds_>;

    using singleton_type::Get;
    using singleton_type::Destroy;

    static void Create() {
        singleton_type::Create();
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

    mutable FAtomicSpinLock Barrier;
    SPARSEARRAY_INSITU(Window, FUserCmd) Commands;
};
//----------------------------------------------------------------------------
static ::HWND GWindowsSystrayWindow_ = NULL;
//----------------------------------------------------------------------------
class FWindowsSystray_ : Meta::TStaticSingleton<FWindowsSystray_> {
    friend Meta::TStaticSingleton<FWindowsSystray_>;
public:
    using singleton_type = Meta::TStaticSingleton<FWindowsSystray_>;

    ~FWindowsSystray_() {
        _active = false;
        _backgroundWorker.join();
        Assert(NULL == _hWnd);
    }

    using singleton_type::Get;
    using singleton_type::Destroy;

    static void Create() { singleton_type::Create(); }

    using ENotificationIcon = FWindowsPlatformNotification::ENotificationIcon;
    void Notify(ENotificationIcon icon, const FWStringView& title, const FWStringView& text) {
        Assert(not title.empty());
        Assert(not text.empty());

        ::NOTIFYICONDATAW nid;
        ::SecureZeroMemory(&nid, sizeof(nid));

        nid.cbSize = sizeof(nid);
        nid.uID = checked_cast<::UINT>(FCurrentProcess::Get().AppIcon());
        nid.uFlags = NIF_INFO | NIF_SHOWTIP;

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

        const Meta::FLockGuard scopeLock(_barrier);
        if (NULL == _hWnd)
            return;

        nid.hWnd = _hWnd;

        if (not ::Shell_NotifyIconW(NIM_MODIFY, &nid))
            PPE_THROW_IT(FLastErrorException("Shell_NotifyIconW(NIM_MODIFY)"));
    }

    // http://gxuanchung.blogspot.com/2010/08/win32-create-window-menu.html
    void ShowPopup(::HWND hWnd) {
        Assert_NoAssume(hWnd == _hWnd);
        Assert_NoAssume(_backgroundWorker.get_id() == std::this_thread::get_id());
        Assert_NoAssume(not _barrier.try_lock());

        // create popup menu from systray commands :
        ::HMENU const hSystrayPopup = ::CreatePopupMenu();

        STACKLOCAL_POD_STACK(FSparseDataId, userCmdRefs, FWindowsSystrayCmds_::Get().Commands.capacity());

        WSTRING_HASHMAP(Window, ::HMENU, ECase::Insensitive) subMenus;
        {
            const auto& systrayCmds = FWindowsSystrayCmds_::Get();
            const FAtomicSpinLock::FScope scopeLock(systrayCmds.Barrier);

            for (const FWindowsSystrayCmds_::FUserCmd& userCmd : systrayCmds.Commands) {
                ::HMENU hSubPopup = NULL;
                if (not TryGetValue(subMenus, userCmd.Category, &hSubPopup)) {
                    Assert(NULL == hSubPopup);
                    hSubPopup = ::CreatePopupMenu();
                    Assert(hSubPopup);
                    Verify(::AppendMenuW(hSystrayPopup, MF_STRING | MF_POPUP, (::UINT_PTR)hSubPopup, userCmd.Category.data()));
                    subMenus.Add(userCmd.Category) = hSubPopup;
                }
                Assert(hSubPopup);

                const FSparseDataId cmdIndex = systrayCmds.Commands.IndexOf(userCmd);
                userCmdRefs.Push(cmdIndex);
                const ::UINT cmdIDI = checked_cast<::UINT>(userCmdRefs.size());

                Verify(::AppendMenuW(hSubPopup, MF_STRING, (::UINT_PTR)cmdIDI, userCmd.Label.data()));
            }
        }

        // show popup as immediate :
        ::POINT lpClickPoint;
        Verify(::GetCursorPos(&lpClickPoint));

        if (hWnd) {
            ::ShowWindow(hWnd, SW_SHOW);
            ::SetForegroundWindow(hWnd);
        }

        Assert(hWnd);
        const ::UINT cmdIDI = ::TrackPopupMenuEx(
            hSystrayPopup,
            TPM_LEFTALIGN | TPM_BOTTOMALIGN |
            TPM_RIGHTBUTTON |
            TPM_RETURNCMD,
            lpClickPoint.x, lpClickPoint.y,
            hWnd,
            NULL );

        if (not cmdIDI)
            LOG_LASTERROR(Notification, L"TrackPopupMenuEx");

        if (hWnd)
            ::ShowWindow(hWnd, SW_HIDE);

        // destroy menus :
        if (not ::DestroyMenu(hSystrayPopup)) // DestroyMenu is recursive
            PPE_THROW_IT(FLastErrorException("DestroyMenu"));

        // run user command if any was selected :
        if (cmdIDI) {
            const FSparseDataId cmdIndex = userCmdRefs[cmdIDI - 1];

            const auto& systrayCmds = FWindowsSystrayCmds_::Get();
            const FAtomicSpinLock::FScope scopeLock(systrayCmds.Barrier);

            const FWindowsSystrayCmds_::FUserCmd* const pUserCmd = systrayCmds.Commands.Find(cmdIndex);

            if (pUserCmd) {
                LOG(Notification, Emphasis, L"launch windows systray command <{0}/{1}>", pUserCmd->Category, pUserCmd->Label);
                pUserCmd->Delegate();
            }
        }
    }

private:
    FWindowsSystray_()
        : _active{ true }
        , _hWnd(NULL) {
        // start the background thread for pumping events
        _backgroundWorker = std::thread(&WorkerEntryPoint_, this);
    }

    std::mutex _barrier;
    std::atomic_bool _active;
    ::HWND _hWnd;
    std::thread _backgroundWorker;

    static void WorkerEntryPoint_(FWindowsSystray_* systray) {
        Assert(nullptr == GWindowsSystrayWindow_);

        const FThreadContextStartup threadStartup("WindowsBackgroundSystray", PPE_THREADTAG_OTHER);

        threadStartup.Context().SetAffinityMask(FPlatformThread::SecondaryThreadAffinity());
        threadStartup.Context().SetPriority(EThreadPriority::Idle);

        FPlatformWindow hiddenWindow;
        {
            FPlatformWindow::FWindowDefinition def;
            FPlatformWindow::HiddenWindowDefinition(&def);
            VerifyRelease(FPlatformWindow::CreateWindow(&hiddenWindow, L"PPE - Systray", def));
        }
        {
            const Meta::FLockGuard scopeLock(systray->_barrier);
            Assert(NULL == systray->_hWnd);
            GWindowsSystrayWindow_ = systray->_hWnd = hiddenWindow.HandleWin32();
            ShowSystray_(systray->_hWnd);
        }

        LOG(HAL, Debug, L"started windows systray background thread for message pump");

        while (systray->_active) {
            if (systray->_barrier.try_lock()) {
                hiddenWindow.PumpMessages();
                systray->_barrier.unlock();
            }
            FPlatformProcess::Sleep(.3f);
        }

        LOG(HAL, Debug, L"stopping windows systray background thread for message pump");

        {
            const Meta::FLockGuard scopeLock(systray->_barrier);
            Assert(hiddenWindow.HandleWin32() == systray->_hWnd);
            Assert(GWindowsSystrayWindow_ == systray->_hWnd);
            HideSystray_(systray->_hWnd);
            GWindowsSystrayWindow_ = systray->_hWnd = NULL;
        }
    }

    // https://chgi.developpez.com/windows/trayicon/
    static NO_INLINE void ShowSystray_(::HWND hWnd) {
        ::NOTIFYICONDATAW nid;
        ::SecureZeroMemory(&nid, sizeof(nid));

        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = FWindowsPlatformNotification::WM_SYSTRAY;
        nid.uVersion = NOTIFYICON_VERSION_4;

        const FCurrentProcess& process = FCurrentProcess::Get();
        nid.uID = checked_cast<::UINT>(process.AppIcon());
        nid.hIcon = LoadIconForSystray_(
            reinterpret_cast<::HINSTANCE>(process.AppHandle()),
            process.AppIcon());

        const FPlatformApplication& app = RunningApp();
        Assert(app.Name().size() < ARRAYSIZE(nid.szTip));
        ::memcpy(&nid.szTip, app.Name().data(), app.Name().size() * sizeof(wchar_t));

        if (not ::Shell_NotifyIconW(NIM_ADD, &nid))
            PPE_THROW_IT(FLastErrorException("Shell_NotifyIconW(NIM_ADD)"));
        if (not ::Shell_NotifyIconW(NIM_SETVERSION, &nid))
            PPE_THROW_IT(FLastErrorException("Shell_NotifyIconW(NIM_SETVERSION)"));
    }

    static NO_INLINE void HideSystray_(::HWND hWnd) {
        ::NOTIFYICONDATAW nid;
        ::SecureZeroMemory(&nid, sizeof(nid));

        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uID = checked_cast<::UINT>(FCurrentProcess::Get().AppIcon());
        nid.uFlags = NIF_STATE;
        nid.dwState = NIS_HIDDEN;

        if (not ::Shell_NotifyIconW(NIM_DELETE, &nid))
            PPE_THROW_IT(FLastErrorException("Shell_NotifyIconW(NIM_DELETE)"));
    }

    static NO_INLINE ::HICON LoadIconForSystray_(::HINSTANCE hInstance, size_t idi) {
        ::HICON hIcon = NULL;

        typedef ::HRESULT(WINAPI *FLoadIconMetric)(::HINSTANCE hinst, ::PCWSTR pszName, int lims, _Out_::HICON *phico);

        ::HMODULE const hComctl32 = FPlatformProcess::AttachToDynamicLibrary(L"Comctl32.dll");

        if (hComctl32) {
            auto* const hLoadIconMetric = (FLoadIconMetric)FPlatformProcess::DynamicLibraryFunction(hComctl32, "LoadIconMetric");

            if (hLoadIconMetric) {
                hLoadIconMetric(hInstance, MAKEINTRESOURCEW(idi), LIM_SMALL, &hIcon);
            }
            else {
                LOG(HAL, Warning, L"failed to bind LoadIconMetric function from Comctl32.dll, fallback on LoadIcon");
            }

            FPlatformProcess::DetachFromDynamicLibrary(hComctl32);
        }

        if (NULL == hIcon)
            hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCEW(idi));

        if (NULL == hIcon)
            PPE_THROW_IT(FLastErrorException("LoadIcon"));

        return hIcon;
    }
};
//----------------------------------------------------------------------------
static TComPtr<::ITaskbarList3> GWindowsTaskBar_;
static FAtomicSpinLock& WindowsTaskBarCS_() {
    ONE_TIME_DEFAULT_INITIALIZE(FAtomicSpinLock, GInstance);
    return GInstance;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::ShowSystray() {
    FWindowsSystray_::Create();
}
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::HideSystray() {
    FWindowsSystray_::Destroy();
}
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::NotifySystray(ENotificationIcon icon, const FWStringView& title, const FWStringView& text) {
    if (GWindowsSystrayWindow_)
        FWindowsSystray_::Get().Notify(icon, title, text);
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformNotification::AddSystrayCommand(
    const FWStringView& category,
    const FWStringView& label,
    FSystrayDelegate&& cmd ) {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    Assert(not category.empty());
    Assert(not label.empty());
    Assert(cmd);

    FSparseDataId cmdIndex = INDEX_NONE;
    {
        auto& systray = FWindowsSystrayCmds_::Get();
        const FAtomicSpinLock::FScope scopeLock(systray.Barrier);
        cmdIndex = systray.Commands.Emplace(category, label, std::move(cmd));
    }
    Assert(INDEX_NONE != cmdIndex);

    LOG(Notification, Debug, L"add systray command <{0}/{1}> -> #{2}", category, label, cmdIndex);

    return cmdIndex;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformNotification::RemoveSystrayCommand(size_t index) {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    Assert(index != INDEX_NONE);

    LOG(Notification, Debug, L"remove systray command #{0}", index);

    auto& systray = FWindowsSystrayCmds_::Get();
    const FAtomicSpinLock::FScope scopeLock(systray.Barrier);

    return systray.Commands.Remove(index);
}
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::SetTaskbarState(ETaskbarState state) {
    Assert(GWindowsTaskBar_);

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

    const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());

    if (not SUCCEEDED(GWindowsTaskBar_->SetProgressState(GWindowsSystrayWindow_, tbpFlags)))
        PPE_THROW_IT(FLastErrorException("SetProgressState"));
}
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::SetTaskbarProgress(size_t completed, size_t total) {
    Assert(GWindowsTaskBar_);
    Assert(completed <= total);

    const ::ULONGLONG ullCompleted = checked_cast<::ULONGLONG>(completed);
    const ::ULONGLONG ullTotal = checked_cast<::ULONGLONG>(total);

    const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());

    if (not SUCCEEDED(GWindowsTaskBar_->SetProgressValue(GWindowsSystrayWindow_, ullCompleted, ullTotal)))
        PPE_THROW_IT(FLastErrorException("SetProgressValue"));
}
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::SummonSystrayPopupMenuWin32(::HWND hWnd) {
    FWindowsSystray_::Get().ShowPopup(hWnd);
}
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::Start() {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();

    Verify(SUCCEEDED(::CoInitialize(NULL)));

    {
        Assert(not GWindowsTaskBar_);

        const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());

        ::ITaskbarList3* taskbar = nullptr;
        if (not SUCCEEDED(::CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (void **)&taskbar)))
            PPE_THROW_IT(FLastErrorException("CoCreateInstance"));

        AssertRelease(taskbar);
        GWindowsTaskBar_.Reset(taskbar);
    }
    {
        FWindowsSystrayCmds_::Create();
    }
}
//----------------------------------------------------------------------------
void FWindowsPlatformNotification::Shutdown() {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    {
        FWindowsSystrayCmds_::Destroy();
    }
    {
        Assert(GWindowsTaskBar_);
        const FAtomicSpinLock::FScope scopeLock(WindowsTaskBarCS_());

        GWindowsTaskBar_.Reset();
    }

    ::CoUninitialize();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
