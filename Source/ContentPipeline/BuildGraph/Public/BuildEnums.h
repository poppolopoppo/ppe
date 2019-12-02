#pragma once

#include "BuildGraph_fwd.h"

#include "IO/TextWriter_fwd.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EBuildFlags {
    None        = 0,
    Rebuild     = 1<<0,
    CacheRead   = 1<<1,
    CacheWrite  = 1<<2,
    DryRun      = 1<<3,
    StopOnError = 1<<4,
    Verbose     = 1<<5,

    Cache       = CacheRead | CacheWrite,
    Default     = Cache,
};
ENUM_FLAGS(EBuildFlags);
//----------------------------------------------------------------------------
enum class EBuildResult {
    Unbuilt     = 0,
    UpToDate    = 1,
    Built       = 2,
    Failed      = 3,
};
//----------------------------------------------------------------------------
inline EBuildResult Combine(EBuildResult lhs, EBuildResult rhs) {
    return EBuildResult{ Max(int(lhs), int(rhs)) };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_BUILDGRAPH_API FTextWriter& operator <<(FTextWriter& oss, ContentPipeline::EBuildFlags flags);
PPE_BUILDGRAPH_API FWTextWriter& operator <<(FWTextWriter& oss, ContentPipeline::EBuildFlags flags);
//----------------------------------------------------------------------------
PPE_BUILDGRAPH_API FTextWriter& operator <<(FTextWriter& oss, ContentPipeline::EBuildResult result);
PPE_BUILDGRAPH_API FWTextWriter& operator <<(FWTextWriter& oss, ContentPipeline::EBuildResult result);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
