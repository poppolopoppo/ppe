#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformSurvey.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/PlatformFile.h"
#include "HAL/PlatformIncludes.h"
#include "HAL/Windows/LastError.h"
#include "HAL/Windows/WindowsWindow.h"

namespace PPE {
namespace Application {
LOG_CATEGORY(, Survey);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void WindowsDisplayAdapter_(const ::DISPLAY_DEVICEW& dev, FWindowsPlatformSurvey::FDisplayAdapter* adapter) {
    Assert(adapter);

    adapter->DeviceName = MakeCStringView(dev.DeviceName);
    adapter->DeviceDescription = MakeCStringView(dev.DeviceString);
    adapter->Primary = !!(dev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE);
    adapter->Vendor = FGenericDisplayAdapter::EVendor::Unknown; // #TODO not available directly, should use RHI but don't wanna :'(
}
//----------------------------------------------------------------------------
static bool WindowsMonitorInfo_(::HMONITOR hMonitor, FWindowsPlatformSurvey::FMonitorInfo* monitor) {
    Assert(monitor);

    ::MONITORINFOEXW mi;
    mi.cbSize = sizeof(mi);

    if (not ::GetMonitorInfoW(hMonitor, &mi)) {
        LOG_LASTERROR(Survey, L"GetMonitorInfoW");
        return false;
    }

    ONLY_IF_ASSERT(const size_t screenW = checked_cast<size_t>(mi.rcMonitor.right - mi.rcMonitor.left));
    ONLY_IF_ASSERT(const size_t screenH = checked_cast<size_t>(mi.rcMonitor.bottom - mi.rcMonitor.top));

    monitor->MonitorName = MakeCStringView(mi.szDevice);
    monitor->Primary = !!(mi.dwFlags & MONITORINFOF_PRIMARY);
    monitor->ScreenX = checked_cast<int>(mi.rcMonitor.left);
    monitor->ScreenY = checked_cast<int>(mi.rcMonitor.top);
    monitor->SafeX = checked_cast<int>(mi.rcWork.left);
    monitor->SafeY = checked_cast<int>(mi.rcWork.top);
    monitor->SafeWidth = checked_cast<size_t>(mi.rcWork.right - mi.rcWork.left);
    monitor->SafeHeight = checked_cast<size_t>(mi.rcWork.bottom - mi.rcWork.top);

    // check current display resolution and orientation
    ::DEVMODE devmode;
    ::SecureZeroMemory(&devmode, sizeof(::DEVMODE));
    devmode.dmSize = sizeof(::DEVMODE);
    devmode.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY|DM_DISPLAYORIENTATION;
    Verify(::EnumDisplaySettingsExW(mi.szDevice, ENUM_CURRENT_SETTINGS, &devmode, 0));

    switch (devmode.dmDisplayOrientation) {
    case DMDO_DEFAULT:
        monitor->Orientation = FGenericMonitorInfo::Default;
        break;
    case DMDO_90:
        monitor->Orientation = FGenericMonitorInfo::Rotate90;
        break;
    case DMDO_180:
        monitor->Orientation = FGenericMonitorInfo::Rotate180;
        break;
    case DMDO_270:
        monitor->Orientation = FGenericMonitorInfo::Rotate270;
        break;
    default:
        AssertNotImplemented();
    }

    monitor->CurrentResolution.Width = checked_cast<u32>(devmode.dmPelsWidth);
    monitor->CurrentResolution.Height = checked_cast<u32>(devmode.dmPelsHeight);
    monitor->CurrentResolution.BitsPerPixel = checked_cast<u32>(devmode.dmBitsPerPel);
    monitor->CurrentResolution.RefreshRate = checked_cast<u32>(devmode.dmDisplayFrequency);

    Assert_NoAssume(screenW == monitor->CurrentResolution.Width);
    Assert_NoAssume(screenH == monitor->CurrentResolution.Height);

    // list supported resolutions :
    for (int iModeNum = 0; ; ++iModeNum) {
        devmode.dmSize = sizeof(::DEVMODE);
        devmode.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY;

        if (not ::EnumDisplaySettingsExW(mi.szDevice, iModeNum, &devmode, 0))
            break;

        // limit results to resolutions with the same bit depth than current resolution
        if (devmode.dmBitsPerPel == monitor->CurrentResolution.BitsPerPixel) {
            FGenericMonitorInfo::FResolution& res = monitor->SupportedResolutions.push_back_Default();
            res.Width = checked_cast<u32>(devmode.dmPelsWidth);
            res.Height = checked_cast<u32>(devmode.dmPelsHeight);
            res.BitsPerPixel = checked_cast<u32>(devmode.dmBitsPerPel);
            res.RefreshRate = checked_cast<u32>(devmode.dmDisplayFrequency);
        }
    }
    Assert_NoAssume(not monitor->SupportedResolutions.empty()); // at least one, which is currently setted

    return true;
}
//----------------------------------------------------------------------------
static bool WindowsStorageInfo_(const wchar_t* path, FWindowsPlatformSurvey::FStorageInfo* storage) {
    Assert(path);
    Assert(storage);

    wchar_t volumeName[MAX_PATH + 1];
    wchar_t filesystemName[MAX_PATH + 1];
    ::DWORD serialNumber = 0;
    ::DWORD maximumComponentLength = 0;
    ::DWORD filesystemFlags = 0;
    if (not ::GetVolumeInformationW(
        path,
        volumeName, lengthof(volumeName),
        &serialNumber,
        &maximumComponentLength,
        &filesystemFlags,
        filesystemName, lengthof(filesystemName)) ) {
        LOG_LASTERROR(Survey, L"GetVolumeInformationW");
        return false;
    }

    const ::UINT driveType = ::GetDriveTypeW(path);
    switch (driveType) {
    case DRIVE_FIXED:
        storage->Removable = false;
        storage->Remote = false;
        break;
    case DRIVE_REMOTE:
        storage->Removable = false;
        storage->Remote = true;
        break;
    case DRIVE_REMOVABLE:
        storage->Removable = true;
        storage->Remote = false;
        break;
    default:
        return false;
    }

    ::ULARGE_INTEGER bytesAvailableToCaller;
    ::ULARGE_INTEGER totalNumberOfBytes;
    ::ULARGE_INTEGER totalNumberOfFreeBytes;
    if (not ::GetDiskFreeSpaceExW(path, &bytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        LOG_LASTERROR(Survey, L"GetDiskFreeSpaceExW");
        return false;
    }

    storage->MountPath = MakeCStringView(path);
    storage->FileSystem = MakeCStringView(filesystemName);
    storage->VolumeName = MakeCStringView(volumeName);
    storage->FreeSizeInBytes = checked_cast<u64>(totalNumberOfFreeBytes.QuadPart);
    storage->TotalSizeInBytes = checked_cast<u64>(totalNumberOfBytes.QuadPart);
    storage->CanRead = true;
    storage->CanListFiles = true;
    storage->CanWrite = !(filesystemFlags & FILE_READ_ONLY_VOLUME);
    storage->CanCreateDir = storage->CanWrite;

    return true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FWindowsPlatformSurvey::PrimaryDisplayAdapter(FDisplayAdapter* displayAdapter) {
    Assert(displayAdapter);

    ::DISPLAY_DEVICEW dev;
    ::SecureZeroMemory(&dev, sizeof(dev));
    dev.cb = sizeof(dev);

    for (::DWORD iDevNum = 0; ; ++iDevNum) {
        if (not ::EnumDisplayDevicesW(NULL, iDevNum, &dev, EDD_GET_DEVICE_INTERFACE_NAME)) {
            LOG_LASTERROR(Survey, L"EnumDisplayDevicesW");
            break;
        }

        if (dev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
            WindowsDisplayAdapter_(dev, displayAdapter);

            LOG(Survey, Info, L"primary display adapter <{0}> : {1}",
                displayAdapter->DeviceName, displayAdapter->DeviceDescription);

            return true;
        }
    }

    AssertNotReached(); // every system *HAS* a GPU
}
//----------------------------------------------------------------------------
bool FWindowsPlatformSurvey::DisplayAdapters(FDisplayAdapters* displayAdapters) {
    Assert(displayAdapters);
    Assert(displayAdapters->empty());

    ::DISPLAY_DEVICEW dev;
    ::SecureZeroMemory(&dev, sizeof(dev));
    dev.cb = sizeof(dev);

    for (::DWORD iDevNum = 0; ; ++iDevNum) {
        if (not ::EnumDisplayDevicesW(NULL, iDevNum, &dev, EDD_GET_DEVICE_INTERFACE_NAME))
            break;

        WindowsDisplayAdapter_(dev, &displayAdapters->push_back_Default());
    }

    return true;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformSurvey::PrimaryMonitor(FMonitorInfo* monitor) {
    Assert(monitor);

    if (WindowsMonitorInfo_(NULL, monitor)) {
        LOG(Survey, Info, L"primary monitor <{0}> : {1}x{2} {3}bpp @ {4}hz",
            monitor->MonitorName,
            monitor->CurrentResolution.Width,
            monitor->CurrentResolution.Height,
            monitor->CurrentResolution.BitsPerPixel,
            monitor->CurrentResolution.RefreshRate );

        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformSurvey::MonitorFromPoint(int x, int y, FMonitorInfo* monitor) {
    Assert(monitor);

    ::POINT pt;
    pt.x = checked_cast<::LONG>(x);
    pt.y = checked_cast<::LONG>(y);

    ::HMONITOR const hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
    if (NULL == hMonitor) {
        LOG_LASTERROR(Survey, L"MonitorFromPoint");
        return false;
    }

    return WindowsMonitorInfo_(hMonitor, monitor);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformSurvey::MonitorFromCursor(FMonitorInfo* monitor) {
    ::POINT mouse;
    return (::GetCursorPos(&mouse)
        ? MonitorFromPoint(mouse.x, mouse.y, monitor)
        : false );
}
//----------------------------------------------------------------------------
bool FWindowsPlatformSurvey::MonitorFromWindow(const FWindowsWindow& window, FMonitorInfo* monitor) {
    Assert(monitor);
    Assert(window.Handle());

    ::HMONITOR const hMonitor = ::MonitorFromWindow(window.HandleWin32(), MONITOR_DEFAULTTONULL);
    if (NULL == hMonitor) {
        LOG_LASTERROR(Survey, L"MonitorFromWindow");
        return false;
    }

    return WindowsMonitorInfo_(hMonitor, monitor);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformSurvey::MonitorInfos(FMonitorInfos* monitors) {
    Assert(monitors);
    Assert(monitors->empty());

    auto const addMonitor = [](::HMONITOR hMonitor, ::HDC , ::LPRECT , ::LPARAM lParam) -> ::BOOL {
        Assert(hMonitor);

        FMonitorInfo monitor;
        if (WindowsMonitorInfo_(hMonitor, &monitor)) {
            FMonitorInfos* const _monitors = (FMonitorInfos*)lParam;
            _monitors->emplace_back(std::move(monitor));
            return TRUE;
        }
        else {
            return FALSE;
        }
    };

    if (::EnumDisplayMonitors(NULL, NULL, addMonitor, (::LPARAM)monitors)) {
        LOG(Survey, Info, L"found {0} monitors", monitors->size());
        return true;
    }
    else {
        LOG_LASTERROR(Survey, L"EnumDisplayMonitors");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformSurvey::ApplicationStorage(FStorageInfo* storage) {
    Assert(storage);

    return WindowsStorageInfo_(*FPlatformFile::WorkingDirectory(), storage);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformSurvey::SystemStorage(FStorageInfo* storage) {
    Assert(storage);

    return WindowsStorageInfo_(*FPlatformFile::SystemDirectory(), storage);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformSurvey::StorageFromPath(const FWStringView& path, FStorageInfo* storage) {
    Assert(not path.empty());
    Assert(storage);

    const FWString cpath(path); // null terminated
    wchar_t fullname[MAX_PATH + 1];
    if (0 == ::GetFullPathNameW(cpath.data(), lengthof(fullname), fullname, NULL)) {
        LOG_LASTERROR(Survey, L"GetFullPathNameW");
        return false;
    }

    return WindowsStorageInfo_(fullname, storage);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformSurvey::StorageInfos(FStorageInfos* storages) {
    Assert(storages);
    Assert(storages->empty());

    wchar_t drivePaths[1024];
    const ::DWORD bufferSize = ::GetLogicalDriveStringsW(lengthof(drivePaths) - 1, drivePaths);

    FStorageInfo storage;
    const wchar_t* const driveBegin = drivePaths;
    const wchar_t* const driveEnd = drivePaths + bufferSize;
    for (const wchar_t* it = driveBegin; it != driveEnd; ) {
        const FWStringView drivePath = MakeCStringView(it);
        it += drivePath.size() + 1;

        if (WindowsStorageInfo_(drivePath.data(), &storage))
            storages->emplace_back(std::move(storage));
    }

    return (not storages->empty());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
