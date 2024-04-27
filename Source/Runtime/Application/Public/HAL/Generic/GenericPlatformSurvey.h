#pragma once

#include "Application_fwd.h"

#include "Container/Vector.h"
#include "IO/String.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericDisplayAdapter {
    FGenericDisplayAdapter() = default;

    enum class EVendor {
        Amd,
        Intel,
        Nvidia,
        Unknown,
    };

    FWString DeviceName;
    FWString DeviceDescription;
    bool Primary{false};
    EVendor Vendor{Default};
};
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericMonitorInfo {
    FGenericMonitorInfo() = default;

    struct FResolution {
        u32 Width{0};
        u32 Height{0};
        u32 BitsPerPixel{0};
        u32 RefreshRate{0};
    };

    enum EOrientation {
        Default,
        Rotate90,
        Rotate180,
        Rotate270,
    };

    FWString MonitorName;

    int ScreenX{0};
    int ScreenY{0};

    int SafeX{0};
    int SafeY{0};
    size_t SafeWidth{0};
    size_t SafeHeight{0};

    bool Primary{false};
    EOrientation Orientation{Default};

    u32 DPIScale{0};

    FResolution CurrentResolution{};
    VECTORINSITU(Survey, FResolution, 8) SupportedResolutions;
};
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericStorageInfo {
    FGenericStorageInfo() = default;

    FWString MountPath;
    FWString FileSystem;
    FWString VolumeName;

    u64 FreeSizeInBytes{0};
    u64 TotalSizeInBytes{0};

    bool CanRead        : 1;
    bool CanWrite       : 1;
    bool CanCreateDir   : 1;
    bool CanListFiles   : 1;

    bool Remote         : 1;
    bool Removable      : 1;
};
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericPlatformSurvey {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasDisplayAdapter, false);
    STATIC_CONST_INTEGRAL(bool, HasMonitor, false);
    STATIC_CONST_INTEGRAL(bool, HasStorageInfo, false);

    using FDisplayAdapter = FGenericDisplayAdapter;
    using FDisplayAdapters = VECTORINSITU(Survey, FDisplayAdapter, 2);

    static bool PrimaryDisplayAdapter(FDisplayAdapter* displayAdapter) = delete;
    static bool DisplayAdapters(FDisplayAdapters* displayAdapters) = delete;

    using FMonitorInfo = FGenericMonitorInfo;
    using FMonitorInfos = VECTORINSITU(Survey, FMonitorInfo, 3);

    static bool PrimaryMonitor(FMonitorInfo* monitor) = delete;
    static bool MonitorFromPoint(int x, int y, FMonitorInfo* monitor) = delete;
    static bool MonitorFromCursor(FMonitorInfo* monitor) = delete;
    static bool MonitorFromWindow(const FGenericWindow& window, FMonitorInfo* monitor) = delete;
    static bool MonitorInfos(FMonitorInfos* monitors) = delete;

    using FStorageInfo = FGenericStorageInfo;
    using FStorageInfos = VECTORINSITU(Survey, FStorageInfo, 4);

    static bool ApplicationStorage(FStorageInfo* storage) = delete;
    static bool SystemStorage(FStorageInfo* storage) = delete;
    static bool StorageFromPath(const FWStringView& path, FStorageInfo* storage) = delete;
    static bool StorageInfos(FStorageInfos* storages) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
