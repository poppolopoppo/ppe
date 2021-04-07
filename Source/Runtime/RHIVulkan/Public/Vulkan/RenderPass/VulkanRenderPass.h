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

        hash_t HashValue;
        hash_t AttachmentHash;
        FSubpassHashes SubpassHashes;

        VkRenderPassCreateInfo CreateInfo{};
        FAttachmentDescs AttachmentDescs;
        FAttachmentRefs1 InputRefs;
        FAttachmentRefs1 AttachmentRefs1;
        FAttachmentRefs2 AttachmentRefs2;
        FSubpassDescs Subpasses;
        FDependencies Dependencies;
        FPreserves Preserves;
    };

    FVulkanRenderPass() = default;
    explicit FVulkanRenderPass(TMemoryView<const FVulkanLogicalRenderPass*> logicalRenderPasses);
    ~FVulkanRenderPass();

    FVulkanRenderPass(FVulkanRenderPass&& ) = default;
    FVulkanRenderPass& operator =(FVulkanRenderPass&& ) = delete;

    auto Read() const { return _pass.LockShared(); }

    hash_t HashValue() const { return Read()->HashValue; }

#if USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _debugName; }
#endif

    bool Create(const FVulkanDevice& device ARGS_IF_RHIDEBUG(FStringView debugName));
    void TearDown(FVulkanResourceManager& resources);

    bool operator ==(const FVulkanRenderPass& other) const;
    bool operator !=(const FVulkanRenderPass& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const FVulkanRenderPass& renderPass) { return renderPass.HashValue(); }

private:
    void Invalidate_(TMemoryView<const FVulkanLogicalRenderPass*> logicalRenderPasses);

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
