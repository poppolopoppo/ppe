#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/PipelineResources.h"

#include "Thread/CriticalSection.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanDescriptorSet {
    VkDescriptorSet First{ VK_NULL_HANDLE };
    u8 IndexInPool{ UMax };
};
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanDescriptorSetLayout final : Meta::FNonCopyable {
public:
    using FBindings = VECTORINSITU(RHIDescriptor, VkDescriptorSetLayoutBinding, 5);
    using FOptionalFlags = VECTORINSITU(RHIDescriptor, VkDescriptorBindingFlags, 5);
    using FDescriptorSetCache = TFixedSizeStack<FVulkanDescriptorSet, 32>;
    using FDynamicData = FPipelineResources::FDynamicData;
    using FPoolSizeArray = TFixedSizeStack<VkDescriptorPoolSize, 10>;
    using FSharedUniformMap = FPipelineDesc::PUniformMap;

    struct FInternalPool {
        VkDescriptorSetLayout Layout{ VK_NULL_HANDLE };
        FDynamicData ResourcesTemplate;
        FSharedUniformMap Uniforms;
        hash_t HashValue{ Meta::ForceInit };

        FPoolSizeArray PoolSizes;
        u32 MaxIndex{ 0 };
        u32 ElementCount{ 0 };
        u32 DynamicOffsetCount{ 0 };
    };

    FVulkanDescriptorSetLayout() NOEXCEPT;
    FVulkanDescriptorSetLayout(
        FBindings* pBindings,
        FOptionalFlags* pOptionalFlags,
        const FVulkanDevice& device,
        FSharedUniformMap&& uniforms);
    ~FVulkanDescriptorSetLayout();

    FVulkanDescriptorSetLayout(FVulkanDescriptorSetLayout&& rvalue) NOEXCEPT;
    FVulkanDescriptorSetLayout& operator =(FVulkanDescriptorSetLayout&&) = delete;

    auto Read() const { return _pool.LockShared(); }

    VkDescriptorSetLayout Handle() const { return Read()->Layout; }
    const FDynamicData& Resources() const { return Read()->ResourcesTemplate; }
    hash_t HashValue() const { return Read()->HashValue; }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    NODISCARD bool Construct(
        const FVulkanDevice& device,
        const TMemoryView<const VkDescriptorSetLayoutBinding>& bindings,
        const TMemoryView<const VkDescriptorBindingFlags>& optionalFlags
        ARGS_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(FVulkanResourceManager& resources);

    NODISCARD bool AllocateDescriptorSet(FVulkanDescriptorSet* pDescriptors, FVulkanResourceManager& resources) const;
    void DeallocateDescriptorSet(FVulkanResourceManager& resources, const FVulkanDescriptorSet& descriptors) const;

    bool operator ==(const FVulkanDescriptorSetLayout& other) const NOEXCEPT;
    bool operator !=(const FVulkanDescriptorSetLayout& other) const NOEXCEPT { return (not operator ==(other)); }

    friend hash_t hash_value(const FVulkanDescriptorSetLayout& layout) NOEXCEPT { return layout.HashValue(); }

private:
    static void AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FVariantUniform& var);
    static void AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FImage& img, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);
    static void AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FTexture& tex, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);
    static void AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FSampler& samp, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);
    static void AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FSubpassInput& spi, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);
    static void AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FUniformBuffer& ub, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);
    static void AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FStorageBuffer& sb, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);
    static void AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FRayTracingScene& rts, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);

    static void BindDescriptor_(FBindings* pBinding, FInternalPool& pool, VkDescriptorType type, u32 bindingIndex, u32& arraySize, EShaderStages stageFlags);

    TRHIThreadSafe<FInternalPool> _pool;

    mutable TThreadSafe<FDescriptorSetCache, EThreadBarrier::CriticalSection> _descriptorSetCache;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
