#pragma once

#include "RHI_fwd.h"

#include "ResourceEnums.h"
#include "ResourceId.h"

#include "Meta/Optional.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FMemoryRequirements {
    u32 MemoryTypeBits{ 0 };
    u32 Alignment{ 0 };
};
//----------------------------------------------------------------------------
struct FMemoryDesc {
    EMemoryType Type{ EMemoryType::Default };
    FMemPoolID Pool;
    u32 Alignment{ 0 };
    Meta::TOptional<FMemoryRequirements> ExternalRequirements;

    FMemoryDesc() = default;
    FMemoryDesc(EMemoryType type) NOEXCEPT : Type(type) {}
    FMemoryDesc(EMemoryType type, FMemPoolID pool) NOEXCEPT : Type(type), Pool(pool) {}
};
PPE_ASSUME_TYPE_AS_POD(FMemoryDesc)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
