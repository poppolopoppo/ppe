#pragma once

#include "Vulkan/VulkanCommon.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanRenderPass final : Meta::FNonCopyable {
public:
    STATIC_CONST_INTEGRAL(u32, MaxAttachments, MaxColorBuffers + 1);
    STATIC_CONST_INTEGRAL(u32, MaxDependencies, MaxRenderPassSubpasses * 2);

    using FAttachmentDescs = TFixedSizeStack<VkAttachmentDescription, MaxAttachments>;
    using FAttachmentRefs1 = TFixedSizeStack<VkAttachmentReference, MaxAttachments * MaxRenderPassSubpasses>;
    using FAttachmentRefs2 = TFixedSizeStack<VkAttachmentReference, MaxRenderPassSubpasses>;
    using FDependencies = TFixedSizeStack<VkSubpassDependency, MaxDependencies>;
    using FPreserves = TFixedSizeStack<u32, MaxColorBuffers * MaxRenderPassSubpasses>;
    using FSubpassDescs = TFixedSizeStack<VkSubpassDescription, MaxRenderPassSubpasses>;
    using FSubpassHashes = TFixedSizeStack<hash_t, MaxRenderPassSubpasses>;

    struct FInternalPass {
        VkRenderPass RenderPass{ VK_NULL_HANDLE };

        hash_t HashValue{ Meta::ForceInit };
        hash_t AttachmentHash{ Meta::ForceInit };
        FSubpassHashes SubpassesHash;

        VkRenderPassCreateInfo CreateInfo{};
        FAttachmentDescs Attachments;
        FAttachmentRefs1 AttachmentsRef;
        FAttachmentRefs1 InputAttachments;
        FAttachmentRefs2 ResolveAttachments;
        FSubpassDescs Subpasses;
        FDependencies Dependencies;
        FPreserves Preserves;
    };

    FVulkanRenderPass() = default;
    explicit FVulkanRenderPass(TMemoryView<const FVulkanLogicalRenderPass* const> logicalRenderPasses);

#if USE_PPE_RHIDEBUG
    ~FVulkanRenderPass();
#endif

    FVulkanRenderPass(FVulkanRenderPass&& rvalue) NOEXCEPT;
    FVulkanRenderPass& operator =(FVulkanRenderPass&& ) = delete;

    auto Read() const { return _pass.LockShared(); }

    VkRenderPass Handle() const { return Read()->RenderPass; }

    hash_t HashValue() const { return Read()->HashValue; }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    NODISCARD bool Construct(const FVulkanDevice& device ARGS_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(FVulkanResourceManager& resources);

    bool operator ==(const FVulkanRenderPass& other) const NOEXCEPT ;
    bool operator !=(const FVulkanRenderPass& other) const NOEXCEPT { return (not operator ==(other)); }

    friend hash_t hash_value(const FVulkanRenderPass& renderPass) { return renderPass.HashValue(); }

private:
    TRHIThreadSafe<FInternalPass> _pass;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
