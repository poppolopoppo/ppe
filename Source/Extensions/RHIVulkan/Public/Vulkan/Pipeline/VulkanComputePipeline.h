#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Pipeline/VulkanShaderModule.h"

#include "RHI/PipelineDesc.h"

#include "Container/HashMap.h"
#include "Thread/ReadWriteLock.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanComputePipeline final : public FRefCountable {
    friend class FVulkanPipelineCache;
public:

    struct FPipelineInstance {
        hash_t HashValue{ 0 };
        FRawPipelineLayoutID LayoutId;
        uint3 LocalGroupSize{ 0 };
        VkPipelineCreateFlags Flags{ Default };
#if USE_PPE_RHIDEBUG
        FPackedDebugMode DebugMode{};
#endif

        FPipelineInstance() = default;

        void Invalidate() NOEXCEPT; // updates HashValue

        bool operator ==(const FPipelineInstance& other) const {
            ONLY_IF_RHIDEBUG(if (DebugMode != other.DebugMode) return false);
            return (LayoutId == other.LayoutId && LocalGroupSize == other.LocalGroupSize && Flags == other.Flags);
        }
        bool operator !=(const FPipelineInstance& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FPipelineInstance& instance) { return instance.HashValue; }
    };

    struct FShaderModule {
        PVulkanShaderModule Module;
#if USE_PPE_RHIDEBUG
        EShaderDebugMode DebugMode{ Default };
#endif
    };

    using FInstanceMap = HASHMAP(RHIPipeline, FPipelineInstance, VkPipeline);
    using FShaderModules = TFixedSizeStack<FShaderModule, static_cast<size_t>(EShaderDebugMode::_Count)>;

    struct FInternalPipeline {
        FPipelineLayoutID BaseLayoutId;
        FShaderModules Shaders;

        uint3 DefaultLocalGroupSize{ 0 };
        uint3 LocalSizeSpecialization{ FComputePipelineDesc::UndefinedSpecialization };
    };

    FVulkanComputePipeline() = default;
#if USE_PPE_RHIDEBUG
    ~FVulkanComputePipeline();
#endif

    FVulkanComputePipeline(FVulkanComputePipeline&& rvalue) = delete;
    FVulkanComputePipeline& operator =(FVulkanComputePipeline&& ) = delete;

    auto Read() const { return _pipeline.LockShared(); }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    NODISCARD bool Construct(const FComputePipelineDesc& desc, FRawPipelineLayoutID layoutId ARGS_IF_RHIDEBUG(FConstChar debugName));
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
