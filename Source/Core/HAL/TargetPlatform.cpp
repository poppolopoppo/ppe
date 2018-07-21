#include "stdafx.h"

#include "TargetPlatform.h"

#include "Diagnostic/Logger.h"

#include "Windows/WindowsTargetPlatform.h"

namespace Core {
LOG_CATEGORY(CORE_API, HAL)
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
} //!namespace Core
