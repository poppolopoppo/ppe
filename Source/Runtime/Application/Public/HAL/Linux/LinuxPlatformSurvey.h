#pragma once

#include "HAL/Generic/GenericPlatformSurvey.h"

#ifndef PLATFORM_LINUX
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
FWD_REFPTR(LinuxWindow);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FLinuxPlatformSurvey : FGenericPlatformSurvey {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasDisplayAdapter, true);
    STATIC_CONST_INTEGRAL(bool, HasMonitor, true);
    STATIC_CONST_INTEGRAL(bool, HasStorageInfo, true);

    using FDisplayAdapter = FGenericPlatformSurvey::FDisplayAdapter;
    using FDisplayAdapters = FGenericPlatformSurvey::FDisplayAdapters;

    static bool PrimaryDisplayAdapter(FDisplayAdapter* displayAdapter);
    static bool DisplayAdapters(FDisplayAdapters* displayAdapters);

    using FMonitorInfo = FGenericPlatformSurvey::FMonitorInfo;
    using FMonitorInfos = FGenericPlatformSurvey::FMonitorInfos;

    static bool PrimaryMonitor(FMonitorInfo* monitor);
    static bool MonitorFromPoint(int x, int y, FMonitorInfo* monitor);
    static bool MonitorFromCursor(FMonitorInfo* monitor);
    static bool MonitorFromWindow(const FLinuxWindow& window, FMonitorInfo* monitor);
    static bool MonitorInfos(FMonitorInfos* monitors);

    using FStorageInfo = FGenericPlatformSurvey::FStorageInfo;
    using FStorageInfos = FGenericPlatformSurvey::FStorageInfos;

    static bool ApplicationStorage(FStorageInfo* storage);
    static bool SystemStorage(FStorageInfo* storage);
    static bool StorageFromPath(const FWStringView& path, FStorageInfo* storage);
    static bool StorageInfos(FStorageInfos* storages);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE