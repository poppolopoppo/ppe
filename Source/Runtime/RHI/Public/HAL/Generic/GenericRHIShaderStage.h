#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#include "Container/AssociativeVector.h"
#include "Container/RawStorage.h"

#include "IO/StringView.h"
#include "Meta/Enum.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericShaderStageFlags : u32 {
    Vertex                   = 1<<0,
    TesselationControl       = 1<<1,
    TesselationEvaluation    = 1<<2,
    Geometry                 = 1<<3,
    Fragment                 = 1<<4,
    Compute                  = 1<<5,

    AllGraphics = Vertex|TesselationControl|TesselationEvaluation|Geometry|Fragment|Compute,

    RayCast                  = 1<<6,
    RayAnyHit                = 1<<7,
    RayClosestHit            = 1<<8,
    RayMiss                  = 1<<9,
    RayIntersection          = 1<<10,
    RayCallable              = 1<<11,

    AllRaytrace = RayCast|RayAnyHit|RayClosestHit|RayMiss|RayIntersection|RayCallable,

    Mesh                     = 1<<12,
    Task                     = 1<<13,

    AllMeshShading = Mesh|Task,
};
ENUM_FLAGS(EGenericShaderStageFlags);
//----------------------------------------------------------------------------
enum class EGenericShaderStageCreateFlags : u32 {
    None                     = 0,
    AllowVaryingSubgroupSize = 1<<0,
    RequireFullSubgroups     = 1<<1,
};
ENUM_FLAGS(EGenericShaderStageCreateFlags);
//----------------------------------------------------------------------------
struct FGenericShaderSpecialization {
    using FConstantID = u32;

    struct FMapEntry {
        u32 Offset;
        u32 Size;
    };

    ASSOCIATIVE_VECTOR(RHIState, FConstantID, FMapEntry) Entries;
    RAWSTORAGE(RHIState, u8) ConstantData;
};
//----------------------------------------------------------------------------
struct FGenericShaderStage {
    EGenericShaderStageFlags Stage;
    EGenericShaderStageCreateFlags CreateFlags;

    FStringView EntryPoint;

    FGenericShaderSpecialization Specialization;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
