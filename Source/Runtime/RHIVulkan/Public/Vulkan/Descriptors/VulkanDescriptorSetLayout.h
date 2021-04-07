#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/PipelineResources.h"

#include "Thread/CriticalSection.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanDescriptorSet {
    VkDescriptorSet DescriptorSet{ VK_NULL_HANDLE };
    u8 IndexInPool{ UMax };
};
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanDescriptorSetLayout final : Meta::FNonCopyable {
public:
    using FBindings = VECTORINSITU(RHIDescriptor, VkDescriptorSetLayoutBinding, 3);
    using FDescriptorSetCache = TFixedSizeStack<FVulkanDescriptorSet, 32>;
    using FDynamicData = FPipelineResources::FDynamicData;
    using FPoolSizeArray = TFixedSizeStack<VkDescriptorPoolSize, 10>;
    using FSharedUniformMap = FPipelineDesc::FSharedUniformMap;

    struct FInternalPool {
        hash_t HashValue;
        VkDescriptorSetLayout Layout{ VK_NULL_HANDLE };
        FDynamicData DynamicData;
        FSharedUniformMap Uniforms;

        FPoolSizeArray PoolSizes;
        u32 MaxIndex{ 0 };
        u32 ElementCount{ 0 };
        u32 DynamicOffsetCount{ 0 };
    };

    FVulkanDescriptorSetLayout() = default;
    FVulkanDescriptorSetLayout(FBindings* pbindings, const FSharedUniformMap& uniforms);
    ~FVulkanDescriptorSetLayout();

    FVulkanDescriptorSetLayout(FVulkanDescriptorSetLayout&&) = default;
    FVulkanDescriptorSetLayout& operator =(FVulkanDescriptorSetLayout&&) = delete;

    auto Read() const { return _pool.LockShared(); }

    hash_t HashValue() const { return Read()->HashValue; }

#ifdef USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _debugName; }
#endif

    bool Create(const FVulkanDevice& device, FBindings&& rbindings);
    void TearDown(FVulkanResourceManager& resources);

    bool AllocateDescriptorSet(FVulkanDescriptorSet* pdescriptors, FVulkanResourceManager& resources) const;
    void DeallocateDescriptorSet(FVulkanResourceManager& resources, const FVulkanDescriptorSet& descriptors) const;

    bool operator ==(const FVulkanDescriptorSetLayout& other) const;
    bool operator !=(const FVulkanDescriptorSetLayout& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const FVulkanDescriptorSetLayout& layout) { return layout.HashValue(); }

private:
    void AddUniform_(FBindings* pbinding, const FPipelineDesc::FVariantUniform& var);
    void AddImage_(FBindings* pbinding, const FPipelineDesc::FImage& img, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);
    void AddTexture_(FBindings* pbinding, const FPipelineDesc::FTexture& tex, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);
    void AddSampler_(FBindings* pbinding, const FPipelineDesc::FSampler& samp, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);
    void AddSubpassInput_(FBindings* pbinding, const FPipelineDesc::FSubpassInput& spi, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);
    void AddUniformBuffer_(FBindings* pbinding, const FPipelineDesc::FUniformBuffer& ub, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);
    void AddStorageBuffer_(FBindings* pbinding, const FPipelineDesc::FStorageBuffer& sb, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);
    void AddRayTracingScene_(FBindings* pbinding, const FPipelineDesc::FRayTracingScene& rts, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags);
    void IncDescriptorCount_(VkDescriptorType type);

    TRHIThreadSafe<FInternalPool> _pool;

    mutable FCriticalSection _descriptorSetBarrier;
    mutable FDescriptorSetCache _descriptorSetCache;

#ifdef USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
