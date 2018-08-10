#include "stdafx.h"

#include "HAL/Windows/WindowsTargetPlatform.h"

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
    case PPE::EPlatformFeature::Client:
        return false;
    case PPE::EPlatformFeature::Server:
        return false;
    case PPE::EPlatformFeature::Editor:
        return false;
    case PPE::EPlatformFeature::DataGeneration:
        return false;
    case PPE::EPlatformFeature::HighQuality:
        return false;
    case PPE::EPlatformFeature::CookedData:
        return false;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
bool FWindowsTargetPlatform::SupportsFeature(EPlatformFeature feature) const {
    switch (feature) {
    case PPE::EPlatformFeature::Client:
        return true;
    case PPE::EPlatformFeature::Server:
        return true;
    case PPE::EPlatformFeature::Editor:
        return true;
    case PPE::EPlatformFeature::DataGeneration:
        return true;
    case PPE::EPlatformFeature::HighQuality:
        return true;
    case PPE::EPlatformFeature::CookedData:
        return true;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE