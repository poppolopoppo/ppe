#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Container/FixedSizeHashTable.h"
#include "Container/Stack.h"
#include "RHI/PipelineDesc.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanPipelineLayout final : Meta::FNonCopyable {
public:

    struct FDescriptorLayout {
        FRawDescriptorSetLayoutID Id;
        VkDescriptorSetLayout Layout{ VK_NULL_HANDLE };
        u32 Index{ 0 };

        FDescriptorLayout() = default;
        FDescriptorLayout(FRawDescriptorSetLayoutID id, VkDescriptorSetLayout layout, u32 index) NOEXCEPT
        :   Id(id), Layout(layout), Index(index)
        {}
    };
    PPE_ASSUME_FRIEND_AS_POD(FDescriptorLayout);

    using FDescriptorSets = TFixedSizeHashMap<FDescriptorSetID, FDescriptorLayout, MaxDescriptorSets>;
    using FDescriptorSetLayouts = TStaticArray<VkDescriptorSetLayout, MaxDescriptorSets>;
    using FPushConstantRanges = TFixedSizeStack<VkPushConstantRange, MaxPushConstantsCount>;
    using FDescriptorSetLayoutsView = TMemoryView<const TPair<FRawPipelineResourcesID, TResourceProxy<VkDescriptorSetLayout>*>>;

    struct FInternalData {
        hash_t HashValue;
        VkPipelineLayout Layour{ VK_NULL_HANDLE };
        FDescriptorSets DescriptorSets;
        FPushConstantDatas PushConstants;
        u32 FirstDescriptorSet{ UMax };
    };

    FVulkanPipelineLayout() = default;
    ~FVulkanPipelineLayout();

    FVulkanPipelineLayout(FVulkanPipelineLayout&& ) = default;
    FVulkanPipelineLayout& operator =(FVulkanPipelineLayout&& ) = delete;

    auto Read() const { return _data.LockShared(); }

    hash_t HashValue() const { return Read()->HashValue; }

#ifdef USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _debugName; }
#endif

    bool AllResourcesAlive(const FVulkanResourceManager& manager) const;
    bool DescriptorLayout(u32* pbinding, FRawDescriptorSetLayoutID* playout, const FDescriptorSetID& id) const;

    bool Create(const FVulkanDevice& device, VkDescriptorSetLayout emptyLayout);
    void TearDown(const FVulkanResourceManager& resources);

    bool operator ==(const FVulkanPipelineLayout& other) const;
    bool operator !=(const FVulkanPipelineLayout& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const FVulkanPipelineLayout& layout) { return layout.HashValue(); }

private:
    void AddDescriptorSets_(hash_t* phash, FDescriptorSets* psets, const FPipelineDesc::FPipelineLayout& ppln, FDescriptorSetLayoutsView sets) const;
    void AddPushCosntants_(hash_t* phash, FPushConstantDatas* pconstants, const FPipelineDesc::FPipelineLayout& ppln) const;

    TRHIThreadSafe<FInternalData> _data;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
