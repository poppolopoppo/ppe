#pragma once

#include "Vulkan/VulkanCommon.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanFramebuffer final : Meta::FNonCopyable {
public:
    using FAttachments = TFixedSizeStack<TPair<FRawImageID, FImageViewDesc>, MaxColorBuffers + 1>;

    struct FInternalData {
        VkFramebuffer Framebuffer{ VK_NULL_HANDLE};

        hash_t HashValue;
        FRawRenderPassID RenderPassId; // strong ref
        uint2 Dimension{ 0 };
        FImageLayer Layers;
        FAttachments Attachments; // weak ref
    };

    FVulkanFramebuffer() = default;
    explicit FVulkanFramebuffer(TMemoryView<const FVulkanLogicalRenderPass*> logicalRenderPasses);
    ~FVulkanFramebuffer();

    FVulkanFramebuffer(FVulkanFramebuffer&& ) = default;
    FVulkanFramebuffer& operator =(FVulkanFramebuffer&& ) = delete;

    auto Read() const { return _pass.LockShared(); }

    hash_t HashValue() const { return Read()->HashValue; }

#if USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _debugName; }
#endif

    bool AllResourcesAlive(const FVulkanResourceManager& manager) const;

    bool Create(const FVulkanDevice& device ARGS_IF_RHIDEBUG(FStringView debugName));
    void TearDown(FVulkanResourceManager& resources);

    bool operator ==(const FVulkanFramebuffer& other) const;
    bool operator !=(const FVulkanFramebuffer& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const FVulkanFramebuffer& renderPass) { return renderPass.HashValue(); }

private:
    TRHIThreadSafe<FInternalData> _pass;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
