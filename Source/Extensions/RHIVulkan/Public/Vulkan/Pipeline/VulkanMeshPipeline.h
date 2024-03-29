#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Pipeline/VulkanGraphicsPipeline.h"

#include "RHI/PipelineDesc.h"
#include "RHI/RenderState.h"

#include "Container/HashMap.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanMeshPipeline final {
    friend class FVulkanPipelineCache;
public:

    struct FPipelineInstance {
        hash_t HashValue{ Meta::ForceInit };
        FRawPipelineLayoutID LayoutId;
        FRawRenderPassID RenderPassId;
        FRenderState RenderState;
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
            return (LayoutId == other.LayoutId && RenderPassId == other.RenderPassId && RenderState == other.RenderState &&
                    DynamicState == other.DynamicState && SubpassIndex == other.SubpassIndex && ViewportCount == other.ViewportCount );
        }
        bool operator !=(const FPipelineInstance& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FPipelineInstance& instance) NOEXCEPT { return instance.HashValue; }
    };

    using FShaderModule = FVulkanGraphicsPipeline::FShaderModule;
    using FShaderModules = FVulkanGraphicsPipeline::FShaderModules;

    using FInstanceMap = HASHMAP(RHIPipeline, FPipelineInstance, VkPipeline);
    using FVertexAttributes = FMeshPipelineDesc::FVertexAttributes;

    struct FInternalPipeline {
        FPipelineLayoutID BaseLayoutId;
        FShaderModules Shaders;

        EPrimitiveTopology Topology{ Default };
        bool EarlyFragmentTests{ true };
    };

    FVulkanMeshPipeline() = default;
#if USE_PPE_RHIDEBUG
    ~FVulkanMeshPipeline();
#endif

    FVulkanMeshPipeline(FVulkanMeshPipeline&& rvalue) = delete;
    FVulkanMeshPipeline& operator =(FVulkanMeshPipeline&& ) = delete;

    auto Read() const { return _pipeline.LockShared(); }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    NODISCARD bool Construct(const FMeshPipelineDesc& desc, FRawPipelineLayoutID layoutId ARGS_IF_RHIDEBUG(FConstChar debugName));
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
