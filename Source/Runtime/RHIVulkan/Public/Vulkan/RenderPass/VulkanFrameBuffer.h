#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/ImageDesc.h"

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

        hash_t HashValue{ 0 };
        FRawRenderPassID RenderPassId; // strong ref
        uint2 Dimension{ 0 };
        FImageLayer Layers;
        FAttachments Attachments; // weak ref
    };

    FVulkanFramebuffer() = default;
    FVulkanFramebuffer(
        const TMemoryView<const TPair<FRawImageID, FImageViewDesc>>& attachments,
        FRawRenderPassID renderPass,
        const uint2& dim, u32 layers );
    ~FVulkanFramebuffer();

    FVulkanFramebuffer(FVulkanFramebuffer&& rvalue) NOEXCEPT;
    FVulkanFramebuffer& operator =(FVulkanFramebuffer&& ) = delete;

    auto Read() const { return _pass.LockShared(); }

    hash_t HashValue() const { return Read()->HashValue; }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    bool AllResourcesAlive(const FVulkanResourceManager& manager) const;

    NODISCARD bool Construct(const FVulkanResourceManager& device ARGS_IF_RHIDEBUG(FConstChar debugName));
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
