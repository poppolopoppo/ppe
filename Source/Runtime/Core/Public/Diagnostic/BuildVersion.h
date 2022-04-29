#pragma once

#include "Core_fwd.h"

#include "Modular/ModuleInfo.h"
#include "Time/Timestamp.h"

#include "BuildVersion.generated.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline CONSTEXPR FBuildVersion CurrentBuildVersion() NOEXCEPT {
    return FBuildVersion{
        Generated::BuildBranch,
        Generated::BuildRevision,
        STRINGIZE(BUILD_FAMILY),
        STRINGIZE(BUILD_COMPILER),
        FTimestamp{ Generated::BuildTimestamp }
    };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
