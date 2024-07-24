// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/LinuxTargetPlatform.h"

#include "HAL/PlatformMisc.h"
#include "IO/String.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FLinuxTargetPlatform& FLinuxTargetPlatform::Get() {
    ONE_TIME_DEFAULT_INITIALIZE(const FLinuxTargetPlatform, GTargetPlatform);
    return GTargetPlatform;
}
//----------------------------------------------------------------------------
FString FLinuxTargetPlatform::DisplayName() const {
    return "Linux";
}
//----------------------------------------------------------------------------
FString FLinuxTargetPlatform::FullName() const {
    return "Linux 8.1 or higher";
}
//----------------------------------------------------------------------------
FString FLinuxTargetPlatform::ShortName() const {
    return "Linux";
}
//----------------------------------------------------------------------------
bool FLinuxTargetPlatform::RequiresFeature(EPlatformFeature feature) const {
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
bool FLinuxTargetPlatform::SupportsFeature(EPlatformFeature feature) const {
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