#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/PipelineDesc.h"

#include "Meta/Utility.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanRayTracingPipeline final : public FRefCountable {
    friend class FVulkanPipelineCache;
public:
    using FSpecializationConstants = FPipelineDesc::FSpecializationConstants;
#if USE_PPE_RHIDEBUG
    using FDebugModeBits = Meta::TStaticBitset<u32(EShaderDebugMode::_Count)>;
#endif

    struct FShaderModule {
        FRTShaderID ShaderId;
        PVulkanShaderModule Module;
        VkShaderStageFlagBits Stage{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM };
        FSpecializationConstants SpecializationConstants;
#if USE_PPE_RHIDEBUG
        EShaderDebugMode DebugMode{ Default };
#endif

        bool operator ==(const FShaderModule& other) const { return (ShaderId == other.ShaderId); }
        bool operator !=(const FShaderModule& other) const { return (not operator ==(other)); }

        bool operator < (const FShaderModule& other) const { return (ShaderId < other.ShaderId); }
        bool operator >=(const FShaderModule& other) const { return (not operator < (other)); }
    };

    using FShaderModules = TFixedSizeStack<FShaderModule, 4>;

    struct FInternalPipeline {
        FPipelineLayoutID BaseLayoutId;
        FShaderModules Shaders;
#if USE_PPE_RHIDEBUG
        FDebugModeBits DebugModeBits;
#endif
    };

    FVulkanRayTracingPipeline() = default;
    ~FVulkanRayTracingPipeline();

    FVulkanRayTracingPipeline(FVulkanRayTracingPipeline&& ) = default;
    FVulkanRayTracingPipeline& operator =(FVulkanRayTracingPipeline&& ) = delete;

    auto Read() const { return _pipeline.LockShared(); }

#ifdef USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _debugName; }
#endif

    bool Create(const FRayTracingPipelineDesc& desc, FRawPipelineLayoutID layoutId ARGS_IF_RHIDEBUG(FStringView debugName));
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
