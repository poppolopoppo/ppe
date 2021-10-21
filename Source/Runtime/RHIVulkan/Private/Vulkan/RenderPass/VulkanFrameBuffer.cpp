
#include "stdafx.h"

#include "Vulkan/RenderPass/VulkanFrameBuffer.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanFramebuffer::FVulkanFramebuffer(
    const TMemoryView<const TPair<FRawImageID, FImageViewDesc>>& attachments,
    FRawRenderPassID renderPass, const uint2& dim, u32 layers ) {
    Assert_NoAssume(renderPass.Valid());

    const auto exclusiveFB = _fb.LockExclusive();

    exclusiveFB->Attachments.Assign(attachments);
    exclusiveFB->RenderPassId = renderPass;
    exclusiveFB->Dimension = dim;
    exclusiveFB->Layers = FImageLayer{ layers };

    exclusiveFB->HashValue = hash_tuple(
        exclusiveFB->Attachments,
        exclusiveFB->RenderPassId,
        exclusiveFB->Dimension,
        exclusiveFB->Layers );
}
//----------------------------------------------------------------------------
FVulkanFramebuffer::FVulkanFramebuffer(FVulkanFramebuffer&& rvalue) NOEXCEPT
:   _fb(std::move(*rvalue._fb.LockExclusive()))
#if USE_PPE_ASSERT
,   _debugName(std::move(rvalue._debugName))
#endif
{

}
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
FVulkanFramebuffer::~FVulkanFramebuffer() {
    const auto exclusiveFB = _fb.LockExclusive();

    Assert_NoAssume(VK_NULL_HANDLE == exclusiveFB->Framebuffer);
    Assert_NoAssume(not exclusiveFB->RenderPassId.Valid());
}
#endif
//----------------------------------------------------------------------------
bool FVulkanFramebuffer::AllResourcesAlive(const FVulkanResourceManager& manager) const {
    const auto sharedFB = _fb.LockShared();

    for (const TPair<FRawImageID, FImageViewDesc>& rt : sharedFB->Attachments) {
        if (not manager.IsResourceAlive(rt.first))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanFramebuffer::Construct(const FVulkanResourceManager& resources ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    const auto exclusiveFB = _fb.LockExclusive();
    Assert_NoAssume(VK_NULL_HANDLE == exclusiveFB->Framebuffer);
    Assert_NoAssume(exclusiveFB->RenderPassId);

    const FVulkanDevice& device = resources.Device();

    ONLY_IF_RHIDEBUG(_debugName = debugName);

    TFixedSizeStack<VkImageView, MaxColorBuffers + 1> imageViews;
    for (TPair<FRawImageID, FImageViewDesc>& rt : exclusiveFB->Attachments) {
        const FVulkanImage& img = resources.ResourceData(rt.first);

        Meta::TOptional<FImageViewDesc> desc;
        const VkImageView vkImageView = img.MakeView(device, desc);
        LOG_CHECK(RHI, VK_NULL_HANDLE != vkImageView);

        Assert(desc.has_value());
        rt.second = desc.value();

        imageViews.Push(vkImageView);
    }

    // create frame buffer

    VkFramebufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = resources.ResourceData(exclusiveFB->RenderPassId).Handle();
    info.attachmentCount = checked_cast<u32>(imageViews.size());
    info.pAttachments = imageViews.data();
    info.width = exclusiveFB->Dimension.x;
    info.height = exclusiveFB->Dimension.y;
    info.layers = *exclusiveFB->Layers;

    VK_CHECK( device.vkCreateFramebuffer(
        device.vkDevice(),
        &info,
        device.vkAllocator(),
        &exclusiveFB->Framebuffer ));

    return true;
}
//----------------------------------------------------------------------------
void FVulkanFramebuffer::TearDown(FVulkanResourceManager& resources) {
    const auto exclusiveFB = _fb.LockExclusive();
    Assert_NoAssume(VK_NULL_HANDLE == exclusiveFB->Framebuffer);
    Assert_NoAssume(exclusiveFB->RenderPassId);

    if (VK_NULL_HANDLE != exclusiveFB->Framebuffer) {
        const FVulkanDevice& device = resources.Device();
        device.vkDestroyFramebuffer(
            device.vkDevice(),
            exclusiveFB->Framebuffer,
            device.vkAllocator() );
    }

    if (exclusiveFB->RenderPassId.Valid())
        resources.ReleaseResource(exclusiveFB->RenderPassId);

    exclusiveFB->Framebuffer = VK_NULL_HANDLE;
    exclusiveFB->RenderPassId = Default;
    exclusiveFB->Dimension = uint2::Zero;
    exclusiveFB->Layers = Default;
    exclusiveFB->HashValue = Default;

    exclusiveFB->Attachments.clear();
    ONLY_IF_RHIDEBUG( _debugName.Clear() );
}
//----------------------------------------------------------------------------
bool FVulkanFramebuffer::operator ==(const FVulkanFramebuffer& other) const NOEXCEPT {
    const auto lhs = _fb.LockShared();
    const auto rhs = other._fb.LockShared();

    if (lhs->HashValue != rhs->HashValue)
        return false;

    return (lhs->Dimension == rhs->Dimension and
            lhs->Layers == rhs->Layers and
            lhs->RenderPassId == rhs->RenderPassId and
            lhs->Attachments == rhs->Attachments );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
