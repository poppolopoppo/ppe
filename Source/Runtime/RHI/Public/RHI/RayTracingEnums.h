#pragma once

#include "RHI_fwd.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ERayTracingGeometryFlags : u32 {
    Opaque                      = 1 << 0,   // indicates that this geometry does not invoke the any-hit shaders even if present in a hit group
    NoDuplicateAnyHitInvocation = 1 << 1,
    _Last,
    Unknown                     = 0,
};
ENUM_FLAGS(ERayTracingGeometryFlags);
//----------------------------------------------------------------------------
enum class ERayTracingInstanceFlags : u32 {
    TriangleCullDisable         = 1 << 0,
    TriangleFrontCCW            = 1 << 1,
    ForceOpaque                 = 1 << 2,   // enable ERayTracingGeometryFlags::Opaque flag
    ForceNonOpaque              = 1 << 3,   // disable ERayTracingGeometryFlags::Opaque flag
    _Last,
    Unknown                     = 0,
};
ENUM_FLAGS(ERayTracingInstanceFlags);
//----------------------------------------------------------------------------
enum class ERayTracingPolicyFlags : u32 {
    AllowUpdate                 = 1 << 0,
    AllowCompaction             = 1 << 1,
    PreferFastTrace             = 1 << 2,
    PreferFastBuild             = 1 << 3,
    LowMemory                   = 1 << 4,
    _Last,
    Unknown                     = 0,
};
ENUM_FLAGS(ERayTracingPolicyFlags);
//----------------------------------------------------------------------------
enum class ERayTracingGroupType {
    Unknown,
    MissShader,
    TriangleHitShader,
    ProceduralHitShader,
    CallableShader,
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
