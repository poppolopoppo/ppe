// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/TargetPlatform.h"

#include "Diagnostic/Logger.h"

#include "HAL/LinuxTargetPlatform.h"
#include "HAL/WindowsTargetPlatform.h"

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, HAL)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const ITargetPlaftorm* GAllPlatforms[] = {
    /* Windows  */&FWindowsTargetPlatform::Get(),
    /* Linux    */&FLinuxTargetPlatform::Get(),
    /* MacOS    *///&FMacOSTargetPlatform::Get(),
    // #TODO add future platforms here
    // #TODO put platform in extension modules and inverse this dependency
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TMemoryView<const ITargetPlaftorm* const> AllTargetPlatforms() {
    return MakeConstView(GAllPlatforms);
}
//----------------------------------------------------------------------------
const ITargetPlaftorm& TargetPlatform(ETargetPlatform platform) {
    STATIC_ASSERT(size_t(ETargetPlatform::Windows) == 0);
    STATIC_ASSERT(size_t(ETargetPlatform::Linux) == 1);
    Assert(static_cast<size_t>(platform) < lengthof(GAllPlatforms));
    return (*GAllPlatforms[static_cast<size_t>(platform)]);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
