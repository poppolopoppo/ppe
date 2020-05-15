#pragma once

#include "Core_fwd.h"

#include "IO/StringView.h"
#include "Time/Timestamp.h"

#include "BuildVersion.generated.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FBuildVersion {
    FStringView Branch;
    FStringView Revision;
    FStringView Family;
    FStringView Compiler;
    FTimestamp Timestamp;
};
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
