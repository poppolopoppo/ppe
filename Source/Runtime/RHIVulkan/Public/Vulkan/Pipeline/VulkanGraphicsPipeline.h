#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/PipelineDesc.h"
#include "RHI/RenderState.h"

#include "Container/HashMap.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanGraphicsPipeline final : public FRefCountable {
    friend class FVulkanPipelineCache;
public:

    struct FPipelineInstance {
        hash_t HashValue;
        FRawPipelineLayoutID LayoutId;
        FRawRenderPassID RenderPassId;
        FRenderState RenderState;
        FVertexInputState VertexInput;
        EPipelineDynamicState DynamicState{ Default };
        u8 SubpassIndex{ 0 };
        u8 ViewportCount{ 0 };
#if USE_PPE_RHIDEBUG
        FPackedDebugMode DebugMode{};
#endif

        FPipelineInstance() = default;

        void Invalidate(); // updates HashValue

        bool operator ==(const FPipelineInstance& other) const {
            ONLY_IF_RHIDEBUG(if (DebugMode != other.DebugMode) return false);
            return (LayoutId == other.LayoutId && RenderPassId == other.RenderPassId && RenderState == other.RenderState && VertexInput == other.VertexInput &&
                    DynamicState == other.DynamicState && SubpassIndex == other.SubpassIndex && ViewportCount == other.ViewportCount );
        }
        bool operator !=(const FPipelineInstance& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FPipelineInstance& instance) { return instance.HashValue; }
    };

    struct FShaderModule {
        PVulkanShaderModule Module;
        VkShaderStageFlagBits Stage{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM };
#if USE_PPE_RHIDEBUG
        EShaderDebugMode DebugMode{ Default };
#endif
    };

    using FInstanceMap = HASHMAP(RHIPipeline, FPipelineInstance, VkPipeline);
    using FShaderModules = TFixedSizeStack<FShaderModule, 8>;
    using FTopologyBits = FGraphicsPipelineDesc::FTopologyBits;
    using FVertexAttributes = FGraphicsPipelineDesc::FVertexAttributes;

    struct FInternalPipeline {
        FPipelineLayoutID BaseLayoutId;
        FShaderModules Shaders;

        FTopologyBits SupportedTopology;
        FVertexAttributes VertexAttributes;
        u32 PatchControlPoints{ 0 };
        bool EarlyFragmentTests{ true };
    };

    FVulkanGraphicsPipeline() = default;
    ~FVulkanGraphicsPipeline();

    FVulkanGraphicsPipeline(FVulkanGraphicsPipeline&& rvalue) = delete;
    FVulkanGraphicsPipeline& operator =(FVulkanGraphicsPipeline&& ) = delete;

    auto Read() const { return _pipeline.LockShared(); }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    NODISCARD bool Construct(const FGraphicsPipelineDesc& desc, FRawPipelineLayoutID layoutId ARGS_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(FVulkanResourceManager& resources);

private:
    TRHIThreadSafe<FInternalPipeline> _pipeline;

    mutable TThreadSafe<FInstanceMap, EThreadBarrier::RWLock> _sharedInstances; // used by pipeline cache

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
