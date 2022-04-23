#pragma once

#include "RHI_fwd.h"

#include "HAL/TargetRHI_fwd.h"

#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"
#include "Modular/Modular_fwd.h"
#include "Meta/Enum.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETargetRHI : u32 {
    Vulkan              = 0,
    _Count,
    Unknown             = UINT32_MAX,
};
PPE_RHI_API FTextWriter& operator <<(FTextWriter& oss, ETargetRHI rhi);
PPE_RHI_API FWTextWriter& operator <<(FWTextWriter& oss, ETargetRHI rhi);
//----------------------------------------------------------------------------
enum class ERHIFeature : u32 {
    Headless            = 1u<<0,
    Discrete            = 1u<<1,
    Graphics            = 1u<<2,
    Compute             = 1u<<3,
    AsyncCompute        = 1u<<4,
    RayTracing          = 1u<<5,
    MeshDraw            = 1u<<6,
    SamplerFeedback     = 1u<<7,
    TextureSpaceShading = 1u<<8,
    VariableShadingRate = 1u<<9,
    ConservativeDepth   = 1u<<10,
    HighDynamicRange    = 1u<<11,
    VSync               = 1u<<12,

    Debugging           = 1u<<30,
    Profiling           = 1u<<31,

    All = 0xFFFF,
    Minimal = Graphics|Compute,
    Recommended = Minimal|AsyncCompute|Discrete,
};
ENUM_FLAGS(ERHIFeature);
PPE_RHI_API FTextWriter& operator <<(FTextWriter& oss, ERHIFeature features);
PPE_RHI_API FWTextWriter& operator <<(FWTextWriter& oss, ERHIFeature features);
//----------------------------------------------------------------------------
struct FRHIDeviceCreateInfo {
    FStringView DeviceName;
    size_t MaxStagingBufferMemory{ ~0_b };
    size_t StagingBufferSize{ 0_b };
    ERHIFeature Features{ ERHIFeature::Recommended };
};
//----------------------------------------------------------------------------
struct FRHISurfaceCreateInfo {
    RHI::FWindowHandle Hwnd{ nullptr };
    uint2 Dimensions{ uint2::MaxValue };
    bool EnableFullscreen{ false };
    bool EnableVSync{ false };
};
//----------------------------------------------------------------------------
class PPE_RHI_API ITargetRHI : Meta::FNonCopyableNorMovable {
public:
    ITargetRHI() = default;
    virtual ~ITargetRHI() = default;

    virtual ETargetRHI RHI() const NOEXCEPT = 0;

    virtual FString DisplayName() const NOEXCEPT = 0;
    virtual FString FullName() const NOEXCEPT = 0;
    virtual FString ShortName() const NOEXCEPT = 0;

    virtual ERHIFeature RecommendedFeatures() const NOEXCEPT = 0;
    virtual bool RequiresFeature(ERHIFeature feature) const NOEXCEPT = 0;
    virtual bool SupportsFeature(ERHIFeature feature) const NOEXCEPT = 0;

    NODISCARD virtual bool CreateService(
        URHIService* pRHIService,
        const FModularDomain& domain,
        const FRHIDeviceCreateInfo& deviceInfo,
        const FRHISurfaceCreateInfo* pOptionalWindow = nullptr) const = 0;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
