#include "stdafx.h"

#include "HAL/TargetPlatform.h"

#include "Diagnostic/Logger.h"

#include "HAL/Windows/WindowsTargetPlatform.h"

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, HAL)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const ITargetPlaftorm* GAllPlatforms[] = {
    /* Windows  */&FWindowsTargetPlatform::Get(),
    /* Linux    *///&FLinuxTargetPlatform::Get(), #TODO
    /* MacOS    *///&FMacOSTargetPlatform::Get(), #TODO
    // #TODO add future platforms here
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
    AssertRelease(platform == ETargetPlatform::Windows); // #TODO
    STATIC_ASSERT(size_t(ETargetPlatform::Windows) == 0);
    return (*GAllPlatforms[size_t(ETargetPlatform::Windows)]);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE