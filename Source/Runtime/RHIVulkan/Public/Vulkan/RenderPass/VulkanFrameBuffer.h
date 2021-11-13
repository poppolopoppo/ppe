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

        hash_t HashValue{ Meta::ForceInit };
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

#if USE_PPE_ASSERT
    ~FVulkanFramebuffer();
#endif

    FVulkanFramebuffer(FVulkanFramebuffer&& rvalue) NOEXCEPT;
    FVulkanFramebuffer& operator =(FVulkanFramebuffer&& ) = delete;

    auto Read() const { return _fb.LockShared(); }
    VkFramebuffer Handle() const { return Read()->Framebuffer; }

    hash_t HashValue() const { return Read()->HashValue; }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    bool AllResourcesAlive(const FVulkanResourceManager& manager) const;

    NODISCARD bool Construct(const FVulkanResourceManager& resources ARGS_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(FVulkanResourceManager& resources);

    bool operator ==(const FVulkanFramebuffer& other) const NOEXCEPT;
    bool operator !=(const FVulkanFramebuffer& other) const NOEXCEPT { return (not operator ==(other)); }

    friend hash_t hash_value(const FVulkanFramebuffer& renderPass) { return renderPass.HashValue(); }

private:
    TRHIThreadSafe<FInternalData> _fb;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
