#pragma once

#include "Container/FlatSet.h"
#include "Vulkan/VulkanCommon.h"

#include "RHI/PipelineDesc.h"

#include "Meta/Utility.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanRayTracingPipeline final {
    friend class FVulkanPipelineCache;
public:
    using FSpecializationConstants = FPipelineDesc::FSpecializationConstants;
#if USE_PPE_RHIDEBUG
    using FDebugModeBits = Meta::TStaticBitset<static_cast<u32>(EShaderDebugMode::_Count)>;
#endif

    struct FShaderModule {
        FRTShaderID ShaderId;
        PVulkanShaderModule Module;
        VkShaderStageFlagBits Stage{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM };
        FSpecializationConstants SpecializationConstants;
#if USE_PPE_RHIDEBUG
        EShaderDebugMode DebugMode{ Default };

        bool operator ==(const FShaderModule& other) const { return (ShaderId == other.ShaderId and DebugMode == other.DebugMode); }
        bool operator < (const FShaderModule& other) const { return (ShaderId == other.ShaderId ? DebugMode < other.DebugMode : ShaderId < other.ShaderId); }
#else
        bool operator ==(const FShaderModule& other) const { return (ShaderId == other.ShaderId); }
        bool operator < (const FShaderModule& other) const { return operator < (other.ShaderId); }
#endif

        bool operator !=(const FShaderModule& other) const { return not operator ==(other); }
        bool operator >=(const FShaderModule& other) const { return not operator <(other); }

        bool operator ==(const FRTShaderID& shaderId) const { return (ShaderId == shaderId); }
        bool operator !=(const FRTShaderID& shaderId) const { return (not operator ==(shaderId)); }

        bool operator < (const FRTShaderID& shaderId) const { return (ShaderId < shaderId); }
        bool operator >=(const FRTShaderID& shaderId) const { return (not operator < (shaderId)); }
    };

    using FShaderModules = FLATSET_INSITU(RHIRayTracing, FShaderModule, 4);

    struct FInternalPipeline {
        FPipelineLayoutID BaseLayoutId;
        FShaderModules Shaders;
#if USE_PPE_RHIDEBUG
        FDebugModeBits DebugModeBits;
#endif
    };

    FVulkanRayTracingPipeline() = default;
#if USE_PPE_RHIDEBUG
    ~FVulkanRayTracingPipeline();
#endif

    FVulkanRayTracingPipeline(FVulkanRayTracingPipeline&& rvalue) = delete;
    FVulkanRayTracingPipeline& operator =(FVulkanRayTracingPipeline&& ) = delete;

    auto Read() const { return _pipeline.LockShared(); }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    NODISCARD bool Construct(const FRayTracingPipelineDesc& desc, FRawPipelineLayoutID layoutId ARGS_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(FVulkanResourceManager& resources);

private:
    TRHIThreadSafe<FInternalPipeline> _pipeline;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
