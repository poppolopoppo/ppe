#pragma once

#include "RHI_fwd.h"

#include "ResourceEnums.h"
#include "ResourceId.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FMemoryDesc {
    EMemoryType Type{ EMemoryType::Default };
    FMemPoolID Pool;
    u32 Alignment{ 0 };
    u32 ExternalRequirements{ 0 };

    FMemoryDesc() = default;
    FMemoryDesc(EMemoryType type) : Type(type) {}
    FMemoryDesc(EMemoryType type, FMemPoolID pool) : Type(type), Pool(pool) {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
