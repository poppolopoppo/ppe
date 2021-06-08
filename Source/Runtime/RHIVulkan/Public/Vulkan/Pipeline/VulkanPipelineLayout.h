#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Container/Array.h"
#include "Container/FixedSizeHashTable.h"
#include "Container/Stack.h"
#include "RHI/PipelineDesc.h"

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

    using FDescriptorSets = TFixedSizeHashMap<FDescriptorSetID, FDescriptorLayout, MaxDescriptorSets>;
    using FDescriptorSetLayouts = TStaticArray<VkDescriptorSetLayout, MaxDescriptorSets>;
    using FPushConstants = FPipelineDesc::FPushConstants;
    using FPushConstantRanges = TFixedSizeStack<VkPushConstantRange, MaxPushConstantsCount>;
    using FDescriptorSetLayoutsView = TMemoryView<const TPair<FRawDescriptorSetLayoutID, TResourceProxy<FVulkanDescriptorSetLayout>*>>;

    struct FInternalData {
        hash_t HashValue;
        VkPipelineLayout Layour{ VK_NULL_HANDLE };
        FDescriptorSets DescriptorSets;
        FPushConstants PushConstants;
        u32 FirstDescriptorSet{ UMax };
    };

    FVulkanPipelineLayout() = default;
    FVulkanPipelineLayout(const FPipelineDesc::FPipelineLayout& ppln, FDescriptorSetLayoutsView sets);
    ~FVulkanPipelineLayout();

    FVulkanPipelineLayout(FVulkanPipelineLayout&& rvalue) NOEXCEPT;
    FVulkanPipelineLayout& operator =(FVulkanPipelineLayout&& ) = delete;

    auto Read() const { return _data.LockShared(); }

    hash_t HashValue() const { return Read()->HashValue; }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    bool AllResourcesAlive(const FVulkanResourceManager& manager) const;
    NODISCARD bool DescriptorLayout(u32* pbinding, FRawDescriptorSetLayoutID* playout, const FDescriptorSetID& id) const;

    NODISCARD bool Construct(const FVulkanDevice& device, VkDescriptorSetLayout emptyLayout ARGS_IF_RHIDEBUG(FConstChar debugName));
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
