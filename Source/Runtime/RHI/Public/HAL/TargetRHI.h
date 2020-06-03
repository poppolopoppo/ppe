#pragma once

#include "RHI_fwd.h"

#include "HAL/TargetRHI_fwd.h"

#include "Diagnostic/Logger_fwd.h"
#include "IO/String_fwd.h"

#define PPE_RHI_MAKEINCLUDE(_BASENAME) \
    STRINGIZE(HAL/TARGET_RHI/CONCAT(TARGET_RHI, _BASENAME).h)

#define PPE_RHI_TARGETALIAS(_BASENAME) \
    CONCAT3(F, TARGET_RHI, _BASENAME)

#define PPE_RHI_MAKEALIAS(_BASENAME) \
    namespace PPE { \
        using CONCAT(F, _BASENAME) = PPE_RHI_TARGETALIAS(_BASENAME); \
    }

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETargetRHI {
    Vulkan         = 0,

    _Count,

    Current         = TARGET_RHI,
};
//----------------------------------------------------------------------------
enum class ERHIFeature {
    HighEndGraphics     = 1<<0,
    Compute             = 1<<1,
    AsyncCompute        = 1<<2,
    Raytracing          = 1<<3,
    Meshlet             = 1<<4,
    SamplerFeedback     = 1<<5,
    TextureSpaceShading = 1<<6,
};
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
inline const ITargetRHI& CurrentRHI() { return TargetRHI(ETargetRHI::Current); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
