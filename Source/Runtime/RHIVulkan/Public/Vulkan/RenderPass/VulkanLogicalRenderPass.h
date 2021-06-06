#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/ImageDesc.h"
#include "RHI/RenderState.h"

#include "Allocator/SlabHeap.h"
#include "Allocator/SlabAllocator.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanLogicalRenderPass final : Meta::FNonCopyable {
public:
    struct FColorTarget {
        u32 Index{ Default };
        FRawImageID ImageId;
        const FVulkanLocalImage* ImagePtr{ nullptr };
        FImageViewDesc Desc;
        VkSampleCountFlagBits Samples{ VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM };
        VkAttachmentLoadOp LoadOp{ VK_ATTACHMENT_LOAD_OP_MAX_ENUM };
        VkAttachmentStoreOp StoreOp{ VK_ATTACHMENT_STORE_OP_MAX_ENUM };
        EResourceState State{ Default };

        hash_t ImageHash;
        mutable VkImageLayout Layout{ VK_IMAGE_LAYOUT_UNDEFINED }; // *not* hashed

        FColorTarget() = default;
    };

    struct FDepthStencilTarget : FColorTarget {
        FDepthStencilTarget() = default;
        FDepthStencilTarget(const FColorTarget& colorTarget) NOEXCEPT : FColorTarget(colorTarget) {}

        bool Valid() const { return ImageId.Valid(); }
    };

    using FClearValues = TStaticArray<VkClearValue, MaxColorBuffers + 1/* depth/stencil */>;
    using FColorTargets = TFixedSizeStack<FColorTarget, MaxColorBuffers>;
    using FViewports = TFixedSizeStack<VkViewport, MaxViewports>;
    using FScissors = TFixedSizeStack<VkRect2D, MaxViewports>;
    using FShadingRatePerViewport = TFixedSizeStack<VkShadingRatePaletteNV, MaxViewports>;

    using FMutableImages = TMemoryView<const TPair<const FVulkanLocalImage*, EResourceState>>;
    using FMutableBuffers = TMemoryView<const TPair<const FVulkanLocalBuffer*, EResourceState>>;

    FVulkanLogicalRenderPass() = default;
    ~FVulkanLogicalRenderPass();

    FVulkanLogicalRenderPass(FVulkanLogicalRenderPass&& ) = default;
    FVulkanLogicalRenderPass& operator =(FVulkanLogicalRenderPass&& ) = delete;

    TMemoryView<IVulkanDrawTask* const> DrawTasks() const { return _drawTasks.MakeConstView(); }

    const FColorTargets& ColorTargets() const { return _colorTargets; }
    const FDepthStencilTarget& DepthStencilTarget() const { return _depthStencilTarget; }
    TMemoryView<const VkClearValue> ClearValues() const { return _clearValues.MakeConstView(); }

    const FRectangleI& Area() const { return _area; }
    bool IsSubmitted() const { return _isSubmitted; }

    FRawFramebufferID FramebufferId() const { return _framebufferId; }
    FRawRenderPassID RenderPassId() const { return _renderPassId; }
    u32  SubpassIndex() const { return _subpassIndex; }

    TMemoryView<const VkViewport> Viewports() const { return _viewports.MakeConstView(); }
    TMemoryView<const VkRect2D> Scissors() const { return _scissors.MakeConstView(); }

    const FBlendState& BlendState() const { return _blendState; }
    const FDepthBufferState& DepthState() const { return _depthState; }
    const FStencilBufferState& StencilState() const { return _stencilState; }
    const FRasterizationState& RasterizationState() const { return _rasterizationState; }
    const FMultisampleState& MultisampleState() const { return _multisampleState; }

    const FVulkanPipelineResourceSet& PerPassResources() const { return _perPassResources; }

    FMutableImages MutableImages() const { return _mutableImages; }
    FMutableBuffers MutableBuffers() const { return _mutableBuffers; }

    bool HasShadingRateImage() const { return (!!_shadingRateImage); }
    bool ShadingRateImage(const FVulkanLocalImage** pimage, FImageViewDesc* pdesc) const;
    TMemoryView<const VkShadingRatePaletteNV> ShadingRatePalette() const { return _shadingRatePalette.MakeConstView(); }

    template <typename _DrawTask, typename... _Args>
    _DrawTask* EmplaceTask(_Args&&... args) {
        _DrawTask* const ptask = new (_localHeap) _DrawTask(*this, std::forward<_Args>(args)...);
        Assert(ptask);
        _drawTasks.push_back(ptask);
        return ptask;
    }

    NODISCARD bool Construct(FVulkanCommandBuffer& commandBuffer, const FRenderPassDesc& desc);
    void TearDown(const FVulkanResourceManager& resources);

    void SetRenderPass(FRawRenderPassID renderPass, u32 subpass, FRawFramebufferID framebuffer, u32 depthIndex);

    bool Submit(
        FVulkanCommandBuffer& cmd,
        TMemoryView<const TPair<FRawImageID, EResourceState>> images,
        TMemoryView<const TPair<FRawBufferID, EResourceState>> buffers );

#if USE_PPE_RHIDEBUG
    void SetShaderDebugIndex(EShaderDebugIndex id);
#endif

private:
    FRawFramebufferID _framebufferId;
    FRawRenderPassID _renderPassId;
    u32 _subpassIndex{ 0 };

    SLABHEAP_POOLED(RHIRenderPass) _localHeap;
    VECTOR_SLAB(IVulkanDrawTask*) _drawTasks{ FSlabAllocator(_localHeap) };

    FColorTargets _colorTargets;
    FDepthStencilTarget _depthStencilTarget;
    FClearValues _clearValues;

    FViewports _viewports;
    FScissors _scissors;

    FBlendState _blendState;
    FDepthBufferState _depthState;
    FStencilBufferState _stencilState;
    FRasterizationState _rasterizationState;
    FMultisampleState _multisampleState;

    FRectangleI _area;
    bool _isSubmitted{ false };

    FVulkanPipelineResourceSet _perPassResources;

    const FVulkanLocalImage* _shadingRateImage{ nullptr };
    FImageLayer _shadingRateImageLayer;
    FMipmapLevel _shadingRateImageLevel;
    FShadingRatePerViewport _shadingRatePalette;

    FMutableImages _mutableImages;
    FMutableBuffers _mutableBuffers;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
