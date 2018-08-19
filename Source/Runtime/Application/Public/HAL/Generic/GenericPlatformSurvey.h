#pragma once

#include "Application_fwd.h"

#include "Container/Vector.h"
#include "IO/String.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGenericDisplayAdapter {
    enum class EVendor {
        Amd,
        Intel,
        Nvidia,
    };

    size_t DeviceIndex;
    FString DeviceName;
    FString DriverName;
    EVendor Vendor;
};
//----------------------------------------------------------------------------
struct FGenericMonitorInfo {
    struct FResolution {
        size_t Width;
        size_t Height;
        float RefreshRate; 
    };

    FString MonitorName;
    size_t MonitorIndex;
    size_t NativeResolutionIndex;
    VECTORINSITU(Survey, FResolution, 8) SupportedResolutions; 
};
//----------------------------------------------------------------------------
struct FGenericStorageInfo {
    FString StorageName;
    FWString StoragePath;
    size_t UsedSizeInBytes;
    size_t TotalSizeInBytes;
    bool CanRead : 1;
    bool CanWrite : 1;
    bool CanListFiles : 1;
};
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericPlatformSurvey {
public: // must be defined for every platform
    

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE