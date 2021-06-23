
#include "stdafx.h"

#include "Vulkan/Pipeline/VulkanPipelineLayout.h"

#include "RHI/ResourceProxy.h"
#include "Vulkan/Descriptors/VulkanDescriptorSetLayout.h"
#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanPipelineLayout::FVulkanPipelineLayout(const FPipelineDesc::FPipelineLayout& ppln, FDescriptorSetLayoutsView sets) {
    Assert_NoAssume(ppln.DescriptorSets.size() == sets.size());

    const auto exclusive = _data.LockExclusive();
    Assert(exclusive->Layout == VK_NULL_HANDLE);

    exclusive->HashValue = Default;

    AddDescriptorSets_(
        &exclusive->HashValue,
        &exclusive->DescriptorSets,
        ppln, sets );

    AddPushConstants_(
        &exclusive->HashValue,
        &exclusive->PushConstants,
        ppln );
}
//----------------------------------------------------------------------------
FVulkanPipelineLayout::FVulkanPipelineLayout(FVulkanPipelineLayout&& rvalue) NOEXCEPT {
    const auto src = rvalue._data.LockExclusive();
    const auto dst = _data.LockExclusive();
    AssertRelease(dst->Layout == VK_NULL_HANDLE);

    *dst = std::move(*src);
    Assert_NoAssume(src->DescriptorSets.empty());
    Assert_NoAssume(src->PushConstants.empty());

    src->Layout = VK_NULL_HANDLE;
    src->HashValue = Default;
    src->FirstDescriptorSet = UMax;

    ONLY_IF_RHIDEBUG(_debugName = std::move(rvalue._debugName));
}
//----------------------------------------------------------------------------
FVulkanPipelineLayout::~FVulkanPipelineLayout() {
    Assert_NoAssume(_data.LockExclusive()->Layout == VK_NULL_HANDLE);
}
//----------------------------------------------------------------------------
void FVulkanPipelineLayout::AddDescriptorSets_(
    hash_t* pHash, FDescriptorSets* pSetsInfo,
    const FPipelineDesc::FPipelineLayout& ppln, FDescriptorSetLayoutsView sets ) {

    pSetsInfo->clear();

    hash_combine(*pHash, ppln.DescriptorSets.size());

    forrange(i, 0, ppln.DescriptorSets.size()) {
        const FPipelineDesc::FDescriptorSet& ds = ppln.DescriptorSets[i];
        Assert(ds.Id.Valid());
        Assert(ds.BindingIndex < MaxDescriptorSets);

        const FVulkanDescriptorSetLayout& layout = sets[i].second->Data();
        Assert(layout.Handle());

        pSetsInfo->insert({
            ds.Id,
            FDescriptorLayout{
                sets[i].first,
                layout.Handle(),
                ds.BindingIndex
            }
        });

        hash_combine(*pHash, ds.Id);
        hash_combine(*pHash, ds.BindingIndex);
        hash_combine(*pHash, layout);
    }
}
//----------------------------------------------------------------------------
void FVulkanPipelineLayout::AddPushConstants_(
    hash_t* pHash, FPushConstants* pConstants,
    const FPipelineDesc::FPipelineLayout& ppln ) {

    *pConstants = ppln.PushConstants;

    hash_combine(*pHash, ppln.PushConstants.size());;

    for (const auto& cs : ppln.PushConstants) {
        Assert(cs.first.Valid());

        hash_combine(*pHash, cs.second);
    }
}
//----------------------------------------------------------------------------
bool FVulkanPipelineLayout::Construct(const FVulkanDevice& device, VkDescriptorSetLayout emptyLayout ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    const auto exclusive = _data.LockExclusive();
    Assert_NoAssume(exclusive->Layout == VK_NULL_HANDLE);

    ONLY_IF_RHIDEBUG(_debugName = debugName);

    FDescriptorSetLayouts vkLayouts;
    FPushConstantRanges vkRanges;

    for (VkDescriptorSetLayout& vkLayout : vkLayouts)
        vkLayout = VK_NULL_HANDLE;

    u32 minSet = checked_cast<u32>(vkLayouts.size());
    u32 maxSet = 0;

    foreachitem(it, exclusive->DescriptorSets) {
        const FDescriptorLayout& ds = it->second;
        Assert(ds.Layout);
        Assert(vkLayouts[ds.Index] == emptyLayout);

        vkLayouts[ds.Index] = ds.Layout;

        minSet = Min(minSet, ds.Index);
        maxSet = Max(maxSet, ds.Index);
    }

    for (auto& pc : exclusive->PushConstants) {
        VkPushConstantRange range{};
        range.offset = checked_cast<u32>(pc.second.Offset);
        range.size = checked_cast<u32>(pc.second.Size);
        range.stageFlags = VkCast(pc.second.StageFlags);

        vkRanges.Push(range);
    }

    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.setLayoutCount = (maxSet + 1);
    info.pSetLayouts = vkLayouts.data();
    info.pushConstantRangeCount = checked_cast<u32>(vkRanges.size());
    info.pPushConstantRanges = vkRanges.data();

    VK_CHECK( device.vkCreatePipelineLayout(
        device.vkDevice(), &info, device.vkAllocator(), &exclusive->Layout) );

    exclusive->FirstDescriptorSet = minSet;

    return true;
}
//----------------------------------------------------------------------------
void FVulkanPipelineLayout::TearDown(FVulkanResourceManager& resources) {
    const auto exclusive = _data.LockExclusive();
    Assert_NoAssume(exclusive->Layout != VK_NULL_HANDLE);

    if (exclusive->Layout) {
        const FVulkanDevice& device = resources.Device();
        device.vkDestroyPipelineLayout(device.vkDevice(), exclusive->Layout, device.vkAllocator());
    }

    foreachitem(ds, exclusive->DescriptorSets)
        resources.ReleaseResource(ds->second.Id);

    exclusive->DescriptorSets.clear();
    exclusive->PushConstants.clear();

    exclusive->Layout = VK_NULL_HANDLE;
    exclusive->HashValue = Default;
    exclusive->FirstDescriptorSet = UMax;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineLayout::AllResourcesAlive(const FVulkanResourceManager& resources) const {
    const auto shared = _data.LockShared();

    foreachitem(ds, shared->DescriptorSets) {
        if (not resources.IsResourceAlive(ds->second.Id))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineLayout::operator==(const FVulkanPipelineLayout& other) const NOEXCEPT {
    const auto lhs = _data.LockShared();
    const auto rhs = other._data.LockShared();

    if (lhs->HashValue != rhs->HashValue)
        return false;

    if (lhs->DescriptorSets.size() != rhs->DescriptorSets.size())
        return false;

    return std::equal(
        lhs->DescriptorSets.begin(), lhs->DescriptorSets.end(),
        rhs->DescriptorSets.begin(), rhs->DescriptorSets.end(),
        [](const auto& a, const auto& b) NOEXCEPT -> bool {
            if (a.second.Index != b.second.Index)
                return false;

            Assert(a.second.Layout);
            Assert(b.second.Layout);

            if (not (a.second.Id == b.second.Id))
                return false;

            return true;
        });
}
//----------------------------------------------------------------------------
bool FVulkanPipelineLayout::DescriptorSetLayout(u32* pBinding, FRawDescriptorSetLayoutID* pLayout, const FDescriptorSetID& id) const {
    Assert(pBinding);
    Assert(pLayout);
    Assert(id);

    const auto shared = _data.LockShared();

    const auto it = shared->DescriptorSets.find(id);
    if (Likely(shared->DescriptorSets.end() != it)) {
        *pBinding = it->second.Index;
        *pLayout = it->second.Id;
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
