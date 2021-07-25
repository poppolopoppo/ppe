
#include "stdafx.h"

#include "Vulkan/RenderPass/VulkanRenderPass.h"

#include "External/vulkan/Vulkan-Header.git/include/vulkan/vulkan_beta.h"
#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"
#include "Vulkan/RenderPass/VulkanLogicalRenderPass.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool operator ==(const VkAttachmentDescription& lhs, const VkAttachmentDescription& rhs) NOEXCEPT {
    return (
        (lhs.flags == rhs.flags) &&
        (lhs.format == rhs.format) &&
        (lhs.samples == rhs.samples) &&
        (lhs.loadOp == rhs.loadOp) &&
        (lhs.storeOp == rhs.storeOp) &&
        (lhs.stencilLoadOp == rhs.stencilLoadOp) &&
        (lhs.stencilStoreOp == rhs.stencilStoreOp) &&
        (lhs.initialLayout == rhs.initialLayout) &&
        (lhs.finalLayout == rhs.finalLayout) );
}
//----------------------------------------------------------------------------
static hash_t hash_value(const VkAttachmentDescription& desc) NOEXCEPT {
    return hash_tuple(
        desc.flags,
        desc.format,
        desc.samples,
        desc.loadOp,
        desc.storeOp,
        desc.stencilLoadOp,
        desc.stencilStoreOp,
        desc.initialLayout,
        desc.finalLayout );
}
//----------------------------------------------------------------------------
static bool operator ==(const VkAttachmentReference& lhs, const VkAttachmentReference& rhs) NOEXCEPT {
    return (
        (lhs.attachment == rhs.attachment) &&
        (lhs.layout == rhs.layout) );
}
//----------------------------------------------------------------------------
static hash_t hash_value(const VkAttachmentReference& ref) NOEXCEPT {
    return hash_tuple(ref.attachment, ref.layout);
}
//----------------------------------------------------------------------------
static bool operator ==(const VkSubpassDescription& lhs, const VkSubpassDescription& rhs) NOEXCEPT {

    const auto lhsResolve = (lhs.pResolveAttachments ? TMemoryView{ lhs.pResolveAttachments, lhs.colorAttachmentCount } : Default);
    const auto rhsResolve = (rhs.pResolveAttachments ? TMemoryView{ rhs.pResolveAttachments, rhs.colorAttachmentCount } : Default);

    return (
        (lhs.flags == rhs.flags) &&
        (lhs.pipelineBindPoint == rhs.pipelineBindPoint) &&
        (TMemoryView{ lhs.pInputAttachments, lhs.inputAttachmentCount }.RangeEqual({ rhs.pInputAttachments, rhs.inputAttachmentCount })) &&
        (TMemoryView{ lhs.pColorAttachments, lhs.colorAttachmentCount }.RangeEqual({ rhs.pColorAttachments, rhs.colorAttachmentCount })) &&
        (lhsResolve.RangeEqual(rhsResolve)) &&
        (!!lhs.pDepthStencilAttachment == !!rhs.pDepthStencilAttachment) &&
        (not lhs.pDepthStencilAttachment or *lhs.pDepthStencilAttachment == *rhs.pDepthStencilAttachment) &&
        (TMemoryView{ lhs.pPreserveAttachments, lhs.preserveAttachmentCount }).RangeEqual({ rhs.pPreserveAttachments, rhs.preserveAttachmentCount }) );
}
//----------------------------------------------------------------------------
static hash_t hash_value(const VkSubpassDescription& desc) NOEXCEPT {
    hash_t h = hash_tuple(desc.flags, desc.pipelineBindPoint);

    hash_range(h, desc.pInputAttachments, desc.inputAttachmentCount);
    hash_range(h, desc.pColorAttachments, desc.colorAttachmentCount);
    hash_range(h, desc.pResolveAttachments, desc.colorAttachmentCount);
    hash_range(h, desc.pPreserveAttachments, desc.preserveAttachmentCount);

    if (desc.pDepthStencilAttachment)
        hash_combine(h, *desc.pDepthStencilAttachment);

    return h;
}
//----------------------------------------------------------------------------
static bool operator ==(const VkSubpassDependency& lhs, const VkSubpassDependency& rhs) NOEXCEPT {
    return (
        (lhs.srcSubpass == rhs.srcSubpass) &&
        (lhs.dstSubpass == rhs.dstSubpass) &&
        (lhs.srcStageMask == rhs.srcStageMask) &&
        (lhs.dstStageMask == rhs.dstStageMask) &&
        (lhs.srcAccessMask == rhs.srcAccessMask) &&
        (lhs.dstAccessMask == rhs.dstAccessMask) &&
        (lhs.dependencyFlags == rhs.dependencyFlags) );
}
//----------------------------------------------------------------------------
static void ComputeRenderPassHash_(
    hash_t* outMainHash,
    hash_t* outAttachmentHash,
    FVulkanRenderPass::FSubpassHashes* outSubpassesHash,
    const VkRenderPassCreateInfo& createInfo ) NOEXCEPT {
    Assert(outMainHash);
    Assert(outAttachmentHash);
    Assert(outSubpassesHash);
    Assert(nullptr == createInfo.pNext);

    *outAttachmentHash = hash_range(createInfo.pAttachments, createInfo.attachmentCount);

    *outMainHash = hash_tuple(
        createInfo.flags,
        createInfo.subpassCount,
        *outAttachmentHash );

    outSubpassesHash->Resize(createInfo.subpassCount);
    forrange(i, 0, createInfo.subpassCount) {
        const hash_t subpassHash = hash_value(createInfo.pSubpasses[i]);
        outSubpassesHash->at(i) = subpassHash;
        hash_combine(*outMainHash, subpassHash);
    }

    hash_range(*outMainHash, createInfo.pDependencies, createInfo.dependencyCount);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanRenderPass::~FVulkanRenderPass() {
    const auto exclusivePass = _pass.LockExclusive();

    Assert_NoAssume(VK_NULL_HANDLE == exclusivePass->RenderPass);
}
#endif
//----------------------------------------------------------------------------
FVulkanRenderPass::FVulkanRenderPass(TMemoryView<const FVulkanLogicalRenderPass* const> logicalRenderPasses) {
    AssertReleaseMessage(L"not supported yet", 1 == logicalRenderPasses.size());

    const FVulkanLogicalRenderPass* const pLogicalRenderPass = logicalRenderPasses.front();
    Assert(pLogicalRenderPass);

    const auto exclusivePass = _pass.LockExclusive();

    u32 maxIndex = 0;

    exclusivePass->Subpasses.Resize(1);
    VkSubpassDescription& subpassDesc = exclusivePass->Subpasses[0];
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount = 0;
    subpassDesc.pColorAttachments = nullptr;

    // setup color attachments

    exclusivePass->Attachments.Resize(exclusivePass->Attachments.capacity());

    for (const FVulkanLogicalRenderPass::FColorTarget& ct : pLogicalRenderPass->ColorTargets()) {
        const VkImageLayout layout = ct.Layout;

        VkAttachmentDescription& attachment = exclusivePass->Attachments[ct.Index];
        attachment.flags = 0; // #TODO: VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT
        attachment.format = VkCast(ct.Desc.Format);
        attachment.samples = ct.Samples;
        attachment.loadOp = ct.LoadOp;
        attachment.storeOp = ct.StoreOp;
        attachment.initialLayout = layout;
        attachment.finalLayout = layout;

        exclusivePass->AttachmentsRef.Push(ct.Index, layout);
        ++subpassDesc.colorAttachmentCount;

        maxIndex = Max(ct.Index + 1, maxIndex);
    }

    if (subpassDesc.colorAttachmentCount)
        subpassDesc.pColorAttachments = exclusivePass->AttachmentsRef.data();

    // setup depth-stencil attachment

    if (pLogicalRenderPass->DepthStencilTarget().Valid()) {
        const FVulkanLogicalRenderPass::FDepthStencilTarget& ds = pLogicalRenderPass->DepthStencilTarget();
        const VkImageLayout layout = ds.Layout;

        VkAttachmentDescription& attachment = exclusivePass->Attachments[maxIndex];
        attachment.flags = 0;
        attachment.format = VkCast(ds.Desc.Format);
        attachment.samples = ds.Samples;
        attachment.loadOp = ds.LoadOp;
        attachment.stencilLoadOp = ds.LoadOp; // #TODO: use resource state to change state
        attachment.storeOp = ds.StoreOp;
        attachment.stencilStoreOp = ds.StoreOp;
        attachment.initialLayout = layout;
        attachment.finalLayout = layout;

        exclusivePass->AttachmentsRef.Push(maxIndex++, layout);
        subpassDesc.pDepthStencilAttachment = exclusivePass->AttachmentsRef.Peek();
    }

    exclusivePass->Attachments.Resize(maxIndex);

    // setup create info

    exclusivePass->CreateInfo = {};
    exclusivePass->CreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    exclusivePass->CreateInfo.flags = 0;
    exclusivePass->CreateInfo.attachmentCount = checked_cast<u32>(exclusivePass->Attachments.size());
    exclusivePass->CreateInfo.pAttachments = exclusivePass->Attachments.data();
    exclusivePass->CreateInfo.subpassCount = checked_cast<u32>(exclusivePass->Subpasses.size());
    exclusivePass->CreateInfo.pSubpasses = exclusivePass->Subpasses.data();

    ComputeRenderPassHash_(
        &exclusivePass->HashValue,
        &exclusivePass->AttachmentHash,
        &exclusivePass->SubpassesHash,
        exclusivePass->CreateInfo );
}
//----------------------------------------------------------------------------
FVulkanRenderPass::FVulkanRenderPass(FVulkanRenderPass&& rvalue) NOEXCEPT
:   _pass(std::move(*rvalue._pass.LockExclusive()))
#if USE_PPE_RHIDEBUG
,   _debugName(std::move(rvalue._debugName))
#endif
{

}
//----------------------------------------------------------------------------
bool FVulkanRenderPass::Construct(const FVulkanDevice& device ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    const auto exclusivePass = _pass.LockExclusive();
    Assert_NoAssume(VK_NULL_HANDLE == exclusivePass->RenderPass);

    VK_CHECK( device.vkCreateRenderPass(
        device.vkDevice(),
        &exclusivePass->CreateInfo,
        device.vkAllocator(),
        &exclusivePass->RenderPass ));

    ONLY_IF_RHIDEBUG(_debugName = debugName);
    return true;
}
//----------------------------------------------------------------------------
void FVulkanRenderPass::TearDown(FVulkanResourceManager& resources) {
    const auto exclusivePass = _pass.LockExclusive();
    Assert_NoAssume(VK_NULL_HANDLE != exclusivePass->RenderPass);

    if (VK_NULL_HANDLE != exclusivePass->RenderPass) {
        const FVulkanDevice& device = resources.Device();
        device.vkDestroyRenderPass(
            device.vkDevice(),
            exclusivePass->RenderPass,
            device.vkAllocator() );
    }

    exclusivePass->RenderPass = VK_NULL_HANDLE;
    exclusivePass->CreateInfo = Default;

    exclusivePass->HashValue = Default;
    exclusivePass->AttachmentHash = Default;
    exclusivePass->SubpassesHash.clear();

    exclusivePass->Attachments.clear();
    exclusivePass->AttachmentsRef.clear();
    exclusivePass->InputAttachments.clear();
    exclusivePass->ResolveAttachments.clear();

    exclusivePass->Subpasses.clear();
    exclusivePass->Dependencies.clear();
    exclusivePass->Preserves.clear();

    ONLY_IF_RHIDEBUG(_debugName.Clear());
}
//----------------------------------------------------------------------------
bool FVulkanRenderPass::operator==(const FVulkanRenderPass& other) const NOEXCEPT {
    const auto lhs = _pass.LockShared();
    const auto rhs = other._pass.LockShared();

    return (
        (lhs->HashValue == rhs->HashValue) &&
        (lhs->AttachmentHash == rhs->AttachmentHash) &&
        (lhs->SubpassesHash.MakeView().RangeEqual(rhs->SubpassesHash.MakeView())) &&
        (lhs->CreateInfo.flags == rhs->CreateInfo.flags) &&
        (TMemoryView{ lhs->CreateInfo.pAttachments, lhs->CreateInfo.attachmentCount }).RangeEqual({ rhs->CreateInfo.pAttachments, rhs->CreateInfo.attachmentCount }) &&
        (TMemoryView{ lhs->CreateInfo.pSubpasses, lhs->CreateInfo.subpassCount }).RangeEqual({ rhs->CreateInfo.pSubpasses, rhs->CreateInfo.subpassCount }) &&
        (TMemoryView{ lhs->CreateInfo.pDependencies, lhs->CreateInfo.dependencyCount }).RangeEqual({ rhs->CreateInfo.pDependencies, rhs->CreateInfo.dependencyCount }) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
