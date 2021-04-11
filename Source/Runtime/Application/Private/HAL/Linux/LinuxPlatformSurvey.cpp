#include "stdafx.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformSurvey.h"

#include "HAL/PlatformFile.h"
#include "HAL/PlatformIncludes.h"
#include "HAL/Linux/LinuxWindow.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace Application {
LOG_CATEGORY(, Survey);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FLinuxPlatformSurvey::PrimaryDisplayAdapter(FDisplayAdapter* displayAdapter) {
    UNUSED(displayAdapter);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformSurvey::DisplayAdapters(FDisplayAdapters* displayAdapters) {
    UNUSED(displayAdapters);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformSurvey::PrimaryMonitor(FMonitorInfo* monitor) {
    UNUSED(monitor);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformSurvey::MonitorFromPoint(int x, int y, FMonitorInfo* monitor) {
    UNUSED(x);
    UNUSED(y);
    UNUSED(monitor);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformSurvey::MonitorFromCursor(FMonitorInfo* monitor) {
    UNUSED(monitor);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformSurvey::MonitorFromWindow(const FLinuxWindow& window, FMonitorInfo* monitor) {
    UNUSED(window);
    UNUSED(monitor);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformSurvey::MonitorInfos(FMonitorInfos* monitors) {
    UNUSED(monitors);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformSurvey::ApplicationStorage(FStorageInfo* storage) {
    UNUSED(storage);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformSurvey::SystemStorage(FStorageInfo* storage) {
    UNUSED(storage);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformSurvey::StorageFromPath(const FWStringView& path, FStorageInfo* storage) {
    UNUSED(path);
    UNUSED(storage);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformSurvey::StorageInfos(FStorageInfos* storages) {
    UNUSED(storages);
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_LINUX
