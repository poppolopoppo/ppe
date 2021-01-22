#pragma once

#include "RHI_fwd.h"

#include "HAL/TargetRHI_fwd.h"

#include "Diagnostic/Logger_fwd.h"
#include "IO/String_fwd.h"
#include "IO/TextWriter_fwd.h"
#include "Meta/Enum.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETargetRHI : u32 {
    Vulkan         = 0,

    _Count,

    Current         = TARGET_RHI,
};
PPE_RHI_API FTextWriter& operator <<(FTextWriter& oss, ETargetRHI rhi);
PPE_RHI_API FWTextWriter& operator <<(FWTextWriter& oss, ETargetRHI rhi);
//----------------------------------------------------------------------------
enum class ERHIFeature : u32 {
    Headless            = 1<<0,
    Discrete            = 1<<1,
    Graphics            = 1<<2,
    Compute             = 1<<3,
    AsyncCompute        = 1<<4,
    Raytracing          = 1<<5,
    Meshlet             = 1<<6,
    SamplerFeedback     = 1<<7,
    TextureSpaceShading = 1<<8,
    VariableShadingRate = 1<<9,
    ConservativeDepth   = 1<<10,
    HighDynamicRange    = 1<<11,

    Debugging           = 1<<12,
    Profiling           = 1<<13,

    All = 0xFFFF,
    Minimal = Graphics|Compute,
    Default = Minimal|AsyncCompute|Discrete
};
ENUM_FLAGS(ERHIFeature);
PPE_RHI_API FTextWriter& operator <<(FTextWriter& oss, ERHIFeature features);
PPE_RHI_API FWTextWriter& operator <<(FWTextWriter& oss, ERHIFeature features);
//----------------------------------------------------------------------------
class PPE_RHI_API ITargetRHI : Meta::FNonCopyableNorMovable {
public:
    ITargetRHI() = default;
    virtual ~ITargetRHI() = default;

    virtual ETargetRHI RHI() const = 0;

    virtual FString DisplayName() const = 0;
    virtual FString FullName() const = 0;
    virtual FString ShortName() const = 0;

    virtual bool RequiresFeature(ERHIFeature feature) const = 0;
    virtual bool SupportsFeature(ERHIFeature feature) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
