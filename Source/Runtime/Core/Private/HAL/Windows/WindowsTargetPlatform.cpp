#include "stdafx.h"

#include "WindowsTargetPlatform.h"

#include "HAL/PlatformMisc.h"
#include "IO/String.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FWindowsTargetPlatform& FWindowsTargetPlatform::Get() {
    ONE_TIME_DEFAULT_INITIALIZE(const FWindowsTargetPlatform, GTargetPlatform);
    return GTargetPlatform;
}
//----------------------------------------------------------------------------
FString FWindowsTargetPlatform::DisplayName() const {
    return "Windows";
}
//----------------------------------------------------------------------------
FString FWindowsTargetPlatform::FullName() const {
    return "Windows 8.1 or higher";
}
//----------------------------------------------------------------------------
FString FWindowsTargetPlatform::ShortName() const {
    return "Win";
}
//----------------------------------------------------------------------------
bool FWindowsTargetPlatform::RequiresFeature(EPlatformFeature feature) const {
    switch (feature) {
    case Core::EPlatformFeature::Client:
        return false;
    case Core::EPlatformFeature::Server:
        return false;
    case Core::EPlatformFeature::Editor:
        return false;
    case Core::EPlatformFeature::DataGeneration:
        return false;
    case Core::EPlatformFeature::HighQuality:
        return false;
    case Core::EPlatformFeature::CookedData:
        return false;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
bool FWindowsTargetPlatform::SupportsFeature(EPlatformFeature feature) const {
    switch (feature) {
    case Core::EPlatformFeature::Client:
        return true;
    case Core::EPlatformFeature::Server:
        return true;
    case Core::EPlatformFeature::Editor:
        return true;
    case Core::EPlatformFeature::DataGeneration:
        return true;
    case Core::EPlatformFeature::HighQuality:
        return true;
    case Core::EPlatformFeature::CookedData:
        return true;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE