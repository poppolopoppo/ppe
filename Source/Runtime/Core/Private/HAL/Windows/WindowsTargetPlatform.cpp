// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Windows/WindowsTargetPlatform.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/PlatformMisc.h"
#include "IO/String.h"

#include <winnt_version.h>
#include <VersionHelpers.h>

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
    FString version;

    if (::IsWindows10OrGreater()) {
        version = "Windows 10.0 or greater";
    }
    else if (::IsWindows8Point1OrGreater()) {
        version = "Windows 8.1 or greater";
    }
    else if (::IsWindows8OrGreater()) {
        version = "Windows 8.0 or greater";
    }
    else if (::IsWindows7SP1OrGreater()) {
        version = "Windows 7 SP1 or greater";
    }
    else if (::IsWindows7OrGreater()) {
        version = "Windows 7 or greater";
    }
    else if (::IsWindowsXPOrGreater()) {
        version = "Windows XP or greater";
    }
    else {
        version = "Windows Antique";
    }

    if (::IsWindowsServer()) {
        version += " Server";
    }
    if (::IsActiveSessionCountLimited()) {
        version += " [TRIAL]";
    }

    return version;
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

#endif //!PLATFORM_WINDOWS

