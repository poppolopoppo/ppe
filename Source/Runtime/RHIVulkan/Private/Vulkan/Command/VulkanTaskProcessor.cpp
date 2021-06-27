#include "stdafx.h"

#include "Vulkan/Command/VulkanTaskProcessor.h"

#include "Vulkan/Command/VulkanCommandBuffer.h"
#include "Vulkan/Command/VulkanDrawTask.h"
#include "Vulkan/Pipeline/VulkanPipelineCache.h"

#include "RHI/DrawContext.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Resource>
static auto CommitResourceBarrier_(
    const void* pResource, FVulkanBarrierManager& barriers
    ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* pDebugger) ) {
    return static_cast<const _Resource*>(pResource)->CommitBarrier(barriers ARGS_IF_RHIDEBUG(pDebugger));
}
//----------------------------------------------------------------------------
template <typename _Visitor, typename _Task>
static void VisitDrawTask_(void* visitor, void* data) {
    static_cast<_Visitor*>(visitor)->Visit(
        *static_cast<TVulkanDrawTask<_Task>*>(data) );
}
//----------------------------------------------------------------------------
static void OverrideBlendStates_(FBlendState* pBlendState, const FColorBuffers& overrides) {
    Assert(pBlendState);

    for (const auto& it : overrides) {
        pBlendState->Buffers[static_cast<u32>(it.first)] = it.second;
    }
}
//----------------------------------------------------------------------------
static void OverrideDepthStencilStates_(
    FDepthBufferState* pDepthBufferState,
    FStencilBufferState* pStencilBufferState,
    FRasterizationState* pRasterizationState,
    EPipelineDynamicState* pPipelineDynamicStates,
    const FDrawDynamicStates& overrides ) {
    Assert(pDepthBufferState);
    Assert(pStencilBufferState);
    Assert(pRasterizationState);
    Assert(pPipelineDynamicStates);

    // depth

    if (overrides.HasEnableDepthTest())
        pDepthBufferState->EnableDepthTest = overrides.EnableDepthTest();
    if (overrides.HasEnableDepthWrite())
        pDepthBufferState->EnableDepthWrite = overrides.EnableDepthWrite();

    if (pDepthBufferState->EnableDepthTest) {
        if (overrides.HasDepthCompareOp())
            pDepthBufferState->CompareOp = overrides.DepthCompareOp();
    }

    // stencil

    if (overrides.HasEnableStencilTest())
        pStencilBufferState->EnabledStencilTests = overrides.EnableStencilTest();

    if (pStencilBufferState->EnabledStencilTests) {
        if (overrides.HasStencilFailOp())
            pStencilBufferState->Front.FailOp = pStencilBufferState->Back.FailOp = overrides.StencilFailOp();
        if (overrides.HasStencilPassOp())
            pStencilBufferState->Front.PassOp = pStencilBufferState->Back.PassOp = overrides.StencilPassOp();
        if (overrides.HasStencilDepthFailOp())
            pStencilBufferState->Front.DepthFailOp = pStencilBufferState->Back.DepthFailOp = overrides.StencilDepthFailOp();

        if (overrides.HasStencilCompareMask())
            *pPipelineDynamicStates |= EPipelineDynamicState::StencilCompareMask;
        if (overrides.HasStencilReference())
            *pPipelineDynamicStates |= EPipelineDynamicState::StencilReference;
        if (overrides.HasStencilWriteMask())
            *pPipelineDynamicStates |= EPipelineDynamicState::StencilWriteMask;
    }

    // rasterizer

    if (overrides.HasCullMode())
        pRasterizationState->CullMode = overrides.CullMode();
    if (overrides.HasEnableRasterizerDiscard())
        pRasterizationState->EnableDiscard = overrides.EnableRasterizerDiscard();
    if (overrides.HasEnableFrontFaceCCW())
        pRasterizationState->EnableFrontFaceCCW = overrides.EnableFrontFaceCCW();

}
//----------------------------------------------------------------------------
static void SetupExtensions_(EPipelineDynamicState* pPipelineDynamicStates, const FVulkanLogicalRenderPass& rp) {
    Assert(pPipelineDynamicStates);

    if (rp.HasShadingRateImage())
        *pPipelineDynamicStates |= EPipelineDynamicState::ShadingRatePalette;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Pipeline resource barriers
//----------------------------------------------------------------------------
class FVulkanTaskProcessor::FPipelineResourceBarriers final {
public:
    FPipelineResourceBarriers(FVulkanTaskProcessor& processor, TMemoryView<const u32> dynamicOffsets) NOEXCEPT;

    void operator ()(const FUniformID& uni, const FPipelineResources::FBuffer& buffer);
    void operator ()(const FUniformID& uni, const FPipelineResources::FTexelBuffer& texelBuffer);
    void operator ()(const FUniformID& uni, const FPipelineResources::FImage& image);
    void operator ()(const FUniformID& uni, const FPipelineResources::FTexture& texture);
    void operator ()(const FUniformID& uni, const FPipelineResources::FSampler& sampler);
    void operator ()(const FUniformID& uni, const FPipelineResources::FRayTracingScene& scene);

private:
    FVulkanTaskProcessor& _processor;
    TMemoryView<const u32> _dynamicOffsets;
};
//----------------------------------------------------------------------------
// Draw task barriers
//----------------------------------------------------------------------------
class FVulkanTaskProcessor::FDrawTaskBarriers final {
public:
    using FFragmentOutput = FGraphicsPipelineDesc::FFragmentOutput;

    FDrawTaskBarriers(FVulkanTaskProcessor& processor, const FVulkanLogicalRenderPass& logicalRenderPass) NOEXCEPT;

    TMemoryView<const FFragmentOutput> FragmentOutputs() const {
        return _fragmentOutputs.MakeView().CutBefore(_fragmentMaxCount);
    }

    bool EnableEarlyFragmentTests() const { return _enableEarlyFragmentTests; }
    bool EnableLateFragmentTests() const { return _enableLateFragmentTests; }
    bool EnableDepthWrite() const { return _enableDepthWrite; }
    bool EnableStencilWrite() const { return _enableStencilWrite; }
    bool EnableRasterizerDiscard() const { return _enableRasterizerDiscard; }
    bool EnableCompatibleFragmentOutput() const { return _enableCompatibleFragmentOutput; }

    void Visit(const TVulkanDrawTask<FDrawVertices>& task);
    void Visit(const TVulkanDrawTask<FDrawIndexed>& task);
    void Visit(const TVulkanDrawTask<FDrawMeshes>& task);
    void Visit(const TVulkanDrawTask<FDrawVerticesIndirect>& task);
    void Visit(const TVulkanDrawTask<FDrawIndexedIndirect>& task);
    void Visit(const TVulkanDrawTask<FDrawMeshesIndirect>& task);
    void Visit(const TVulkanDrawTask<FDrawVerticesIndirectCount>& task);
    void Visit(const TVulkanDrawTask<FDrawIndexedIndirectCount>& task);
    void Visit(const TVulkanDrawTask<FDrawMeshesIndirectCount>& task);
    void Visit(const TVulkanDrawTask<FCustomDraw>& task);

    template <typename _Pipeline>
    void MergePipeline(const FDrawDynamicStates& ds, const _Pipeline& pipeline) NOEXCEPT;

    template <typename _DrawTask>
    void ExtractDescriptorSets(const FRawPipelineLayoutID& layoutId, const _DrawTask& task);

private:
    FVulkanTaskProcessor& _processor;
    const FVulkanLogicalRenderPass& _logicalRenderPass;

    TStaticArray<FFragmentOutput, MaxColorBuffers> _fragmentOutputs;
    u32 _fragmentMaxCount{ 0 };

    bool _enableEarlyFragmentTests          : 1;
    bool _enableLateFragmentTests           : 1;
    bool _enableDepthWrite                  : 1;
    bool _enableStencilWrite                : 1;
    bool _enableRasterizerDiscard           : 1;
    bool _enableCompatibleFragmentOutput    : 1;

};
//----------------------------------------------------------------------------
// Draw task commands
//----------------------------------------------------------------------------
class FVulkanTaskProcessor::FDrawTaskCommands final {
public:
    FDrawTaskCommands(
        FVulkanTaskProcessor& processor,
        const TVulkanFrameTask<FSubmitRenderPass>* pSubmit,
        VkCommandBuffer vkCommandBuffer ) NOEXCEPT;

    void Visit(const TVulkanDrawTask<FDrawVertices>& task);
    void Visit(const TVulkanDrawTask<FDrawIndexed>& task);
    void Visit(const TVulkanDrawTask<FDrawMeshes>& task);
    void Visit(const TVulkanDrawTask<FDrawVerticesIndirect>& task);
    void Visit(const TVulkanDrawTask<FDrawIndexedIndirect>& task);
    void Visit(const TVulkanDrawTask<FDrawMeshesIndirect>& task);
    void Visit(const TVulkanDrawTask<FDrawVerticesIndirectCount>& task);
    void Visit(const TVulkanDrawTask<FDrawIndexedIndirectCount>& task);
    void Visit(const TVulkanDrawTask<FDrawMeshesIndirectCount>& task);
    void Visit(const TVulkanDrawTask<FCustomDraw>& task);

private:
    template <typename _Desc>
    void BindTaskPipeline_(const TVulkanDrawTask<_Desc>& task);

    void BindVertexBuffers_(
        TMemoryView<const FVulkanLocalBuffer* const> vertexBuffers,
        TMemoryView<const VkDeviceSize> vertexOffsets ) const;

    template <typename _DrawTask>
    void BindPipelineResources_(
        const FVulkanPipelineLayout& layout,
        const _DrawTask& task ) const;

    FVulkanTaskProcessor& _processor;
    TPtrRef<const TVulkanFrameTask<FSubmitRenderPass>> _submitRef;
    VkCommandBuffer _vkCommandBuffer;
};
//----------------------------------------------------------------------------
// Draw context
//----------------------------------------------------------------------------
class FVulkanTaskProcessor::FDrawContext final : public IDrawContext {
public:
    enum : u32 {
        GRAPHICS_BIT    = 1,
        MESH_BIT        = 2,
        ALL_BITS        = GRAPHICS_BIT | MESH_BIT,
    };

    FDrawContext(FVulkanTaskProcessor& processor, const FVulkanLogicalRenderPass& logicalRenderPass) NOEXCEPT;

    void Reset() override;

    void PushConstants(const FPushConstantID& id, const void* data, size_t size) override;

    void BindPipeline(FRawGPipelineID id, EPipelineDynamicState dynamicState) override;
    void BindPipeline(FRawMPipelineID id, EPipelineDynamicState dynamicState) override;
    void BindResources(const FDescriptorSetID& id, const FPipelineResources& res) override;
    void BindShadingRateImage(FRawImageID value, FImageLayer layer, FMipmapLevel level) override;
    void BindVertexAttribs(const FVertexInputState&) override;
    void BindVertexBuffer(const FVertexBufferID& id, FRawBufferID buffer, size_t offset) override;
    void BindIndexBuffer(FRawBufferID buffer, size_t offset, EIndexFormat fmt) override;

    void SetColorBuffer(ERenderTargetID id, const FColorBufferState& value) override;
    void SetLogicOp(ELogicOp value) override;
    void SetBlendColor(const FLinearColor& value) override;
    void SetStencilBuffer(const FStencilBufferState& value) override;
    void SetDepthBuffer(const FDepthBufferState& value) override;
    void SetInputAssembly(const FInputAssemblyState& value) override;
    void SetRasterization(const FRasterizationState& value) override;
    void SetMultisample(const FMultisampleState& value) override;
    void SetStencilCompareMask(u32 value) override;
    void SetStencilWriteMask(u32 value) override;
    void SetStencilReference(u32 value) override;
    void SetShadingRatePalette(u32 viewportIndex, TMemoryView<const EShadingRatePalette> value) override;

    void DrawVertices(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) override;
    void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance) override;
    void DrawVerticesIndirect(FRawBufferID indirectBuffer, size_t indirectBufferOffset, u32 drawCount, u32 indirectBufferStride) override;
    void DrawIndexedIndirect(FRawBufferID indirectBuffer, size_t indirectBufferOffset, u32 drawCount, u32 indirectBufferStride) override;
    void DrawVerticesIndirectCount(FRawBufferID indirectBuffer, size_t indirectBufferOffset, FRawBufferID countBuffer, size_t countBufferOffset, u32 maxDrawCount, u32 indirectBufferStride) override;
    void DrawIndexedIndirectCount(FRawBufferID indirectBuffer, size_t indirectBufferOffset, FRawBufferID countBuffer, size_t countBufferOffset, u32 maxDrawCount, u32 indirectBufferStride) override;
    void DrawMeshes(u32 taskCount, u32 firstTask) override;
    void DrawMeshesIndirect(FRawBufferID indirectBuffer, size_t indirectBufferOffset, u32 drawCount, u32 indirectBufferStride) override;
    void DrawMeshesIndirectCount(FRawBufferID indirectBuffer, size_t indirectBufferOffset, FRawBufferID countBuffer, size_t countBufferOffset, u32 maxDrawCount, u32 indirectBufferStride) override;

private:
    NODISCARD bool BindPipeline_(u32 mask);

    FVulkanTaskProcessor& _processor;
    const FVulkanLogicalRenderPass& _logicalRenderPass;

    TPtrRef<const FVulkanGraphicsPipeline> _gPipelineRef{ nullptr };
    TPtrRef<const FVulkanMeshPipeline> _mPipelineRef{ nullptr };
    TPtrRef<const FVulkanPipelineLayout> _pipelineLayoutRef{ nullptr };

    FRenderState _renderState;
    EPipelineDynamicState _dynamicStates{ Default };
    FVertexInputState _vertexInput;
    u32 _changed : 2;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <u32 _Uid>
FORCE_INLINE const auto* FVulkanTaskProcessor::ToLocal_(details::TResourceId<_Uid> id) const {
    return _workerCmd->ToLocal(id);
}
//----------------------------------------------------------------------------
template <u32 _Uid>
FORCE_INLINE const auto* FVulkanTaskProcessor::Resource_(details::TResourceId<_Uid> id) const {
    return _workerCmd->AcquireTransient(id);
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
template <typename _Functor>
void FVulkanTaskProcessor::EditStatistics_(_Functor&& rendering) const {
    _workerCmd->EditStatistics([&rendering](FFrameStatistics& stats) {
        rendering(stats.Renderer);
    });
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#include "VulkanTaskProcessor_PipelineResourceBarriers-inl.h"
#include "VulkanTaskProcessor_DrawTaskBarriers-inl.h"
#include "VulkanTaskProcessor_DrawTaskCommands-inl.h"
#include "VulkanTaskProcessor_DrawContext-inl.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanTaskProcessor::FVulkanTaskProcessor(const SVulkanCommandBuffer& workerCmd, VkCommandBuffer vkCommandBuffer) NOEXCEPT
:   FVulkanDeviceFunctions(workerCmd->Device())
,   _workerCmd(workerCmd)
,   _vkCommandBuffer(vkCommandBuffer)
,   _enableDebugUtils(_workerCmd->Device().Enabled().DebugUtils)
,   _isDefaultScissor(false)
,   _perPassStatesUpdated(false)
,   _enableDispatchBase(_workerCmd->Device().Enabled().DispatchBase)
,   _enableDrawIndirectCount(_workerCmd->Device().Enabled().DrawIndirectCount)
,   _enableMeshShaderNV(_workerCmd->Device().Enabled().MeshShaderNV)
,   _enableRayTracingKHR(_workerCmd->Device().Enabled().RayTracingKHR)
,   _maxDrawIndirectCount(_workerCmd->Device().Limits().maxDrawIndirectCount)
,   _maxDrawMeshTaskCount(_workerCmd->Device().Capabilities().MeshShaderProperties.maxDrawMeshTasksCount)
,   _pendingResourceBarriers(_workerCmd->Allocator()) {
    Assert_NoAssume(_vkCommandBuffer);

#if USE_PPE_RHIDEBUG
    _debugColor = FLinearColor::FromHash( hash_value(_workerCmd->DebugName()) );

    char temp[128];
    Format(temp, "CommandBuffer: {0} {1}\0", _workerCmd->DebugName(), Fmt::Pointer(_vkCommandBuffer));
    CmdPushDebugGroup_(temp, _debugColor);
#endif
}
//----------------------------------------------------------------------------
FVulkanTaskProcessor::~FVulkanTaskProcessor() {
    ONLY_IF_RHIDEBUG( CmdPopDebugGroup_() );
}
//----------------------------------------------------------------------------
// DrawVertices
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit1_DrawVertices(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskBarriers, FDrawVertices>(visitor, data);
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit2_DrawVertices(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskCommands, FDrawVertices>(visitor, data);
}
//----------------------------------------------------------------------------
// DrawIndexed
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit1_DrawIndexed(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskBarriers, FDrawIndexed>(visitor, data);
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit2_DrawIndexed(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskCommands, FDrawIndexed>(visitor, data);
}
//----------------------------------------------------------------------------
// DrawMeshes
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit1_DrawMeshes(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskBarriers, FDrawMeshes>(visitor, data);
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit2_DrawMeshes(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskCommands, FDrawMeshes>(visitor, data);
}
//----------------------------------------------------------------------------
// DrawVerticesIndirect
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit1_DrawVerticesIndirect(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskBarriers, FDrawVerticesIndirect>(visitor, data);
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit2_DrawVerticesIndirect(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskCommands, FDrawVerticesIndirect>(visitor, data);
}
//----------------------------------------------------------------------------
// DrawIndexedIndirect
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit1_DrawIndexedIndirect(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskBarriers, FDrawIndexedIndirect>(visitor, data);
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit2_DrawIndexedIndirect(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskCommands, FDrawIndexedIndirect>(visitor, data);
}
//----------------------------------------------------------------------------
// DrawVerticesIndirectCount
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit1_DrawVerticesIndirectCount(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskBarriers, FDrawVerticesIndirectCount>(visitor, data);
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit2_DrawVerticesIndirectCount(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskCommands, FDrawVerticesIndirectCount>(visitor, data);
}
//----------------------------------------------------------------------------
// DrawIndexedIndirectCount
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit1_DrawIndexedIndirectCount(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskBarriers, FDrawIndexedIndirectCount>(visitor, data);
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit2_DrawIndexedIndirectCount(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskCommands, FDrawIndexedIndirectCount>(visitor, data);
}
//----------------------------------------------------------------------------
// DrawMeshesIndirect
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit1_DrawMeshesIndirect(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskBarriers, FDrawMeshesIndirect>(visitor, data);
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit2_DrawMeshesIndirect(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskCommands, FDrawMeshesIndirect>(visitor, data);
}
//----------------------------------------------------------------------------
// DrawMeshesIndirectCount
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit1_DrawMeshesIndirectCount(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskBarriers, FDrawMeshesIndirectCount>(visitor, data);
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit2_DrawMeshesIndirectCount(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskCommands, FDrawMeshesIndirectCount>(visitor, data);
}
//----------------------------------------------------------------------------
// CustomDraw
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit1_CustomDraw(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskBarriers, FCustomDraw>(visitor, data);
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit2_CustomDraw(void* visitor, void* data) {
    VisitDrawTask_<FDrawTaskCommands, FCustomDraw>(visitor, data);
}
//----------------------------------------------------------------------------
// Debug utils
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::CmdDebugMarker_(FConstChar text, const FLinearColor& color) const {
    if (_enableDebugUtils) {
        Assert(text);

        VkDebugUtilsLabelEXT info{};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        info.pLabelName = text.c_str();
        STATIC_ASSERT(sizeof(color) == sizeof(info.color));
        FPlatformMemory::Memcpy(info.color, &color, sizeof(info.color));

        this->vkCmdInsertDebugUtilsLabelEXT(_vkCommandBuffer, &info);
    }
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::CmdPushDebugGroup_(FConstChar text, const FLinearColor& color) const {
    if (_enableDebugUtils) {
        Assert(text);

        VkDebugUtilsLabelEXT info{};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        info.pLabelName = text.c_str();
        STATIC_ASSERT(sizeof(color) == sizeof(info.color));
        FPlatformMemory::Memcpy(info.color, &color, sizeof(info.color));

        this->vkCmdBeginDebugUtilsLabelEXT(_vkCommandBuffer, &info);
    }
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::CmdPopDebugGroup_() const {
    if (_enableDebugUtils)
        vkCmdEndDebugUtilsLabelEXT(_vkCommandBuffer);
}
//----------------------------------------------------------------------------
#endif //!USE_PPE_RHIDEBUG
//----------------------------------------------------------------------------
// Scissors
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::SetScissor_(
    const FVulkanLogicalRenderPass& rp,
    const TMemoryView<const FRectangleI>& rects ) {
    if (not rects.empty()) {
        TFixedSizeStack<VkRect2D, MaxViewports> vkScissors;

        for (const FRectangleI& src : rects) {
            VkRect2D dst{};
            dst.offset.x = src.Left();
            dst.offset.x = src.Top();
            dst.extent.width = src.Width();
            dst.extent.width = src.Height();

            vkScissors.Push(dst);
        }

        vkCmdSetScissor(
            _vkCommandBuffer,
            0, checked_cast<u32>(vkScissors.size()), vkScissors.data() );

        _isDefaultScissor = false;
    }
    else if (not _isDefaultScissor) {
        const TMemoryView<const VkRect2D> vkScissors = rp.Scissors();

        vkCmdSetScissor(
            _vkCommandBuffer,
            0, checked_cast<u32>(vkScissors.size()), vkScissors.data() );

        _isDefaultScissor = true;
    }
}
//----------------------------------------------------------------------------
// Dynamic states
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::SetDynamicStates_(const FDrawDynamicStates& states) {
    if (states.HasEnableStencilTest() && states.EnableStencilTest()) {
        if (states.HasStencilCompareMask())
            vkCmdSetStencilCompareMask(_vkCommandBuffer, VK_STENCIL_FRONT_AND_BACK, states.StencilCompareMask());

        if (states.HasStencilReference())
            vkCmdSetStencilReference(_vkCommandBuffer, VK_STENCIL_FRONT_AND_BACK, states.StencilReference());

        if (states.HasStencilWriteMask())
            vkCmdSetStencilReference(_vkCommandBuffer, VK_STENCIL_FRONT_AND_BACK, states.StencilWriteMask());

        ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
            rendering.NumDynamicStateChanges += (
                static_cast<u32>(states.HasStencilCompareMask()) +
                static_cast<u32>(states.HasStencilReference()) +
                static_cast<u32>(states.HasStencilWriteMask()) );
        }));
    }
}
//----------------------------------------------------------------------------
// Render target barriers
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::AddRenderTargetBarriers_(const FVulkanLogicalRenderPass& rp, const FDrawTaskBarriers& info) {
    Assert_NoAssume(info.EnableCompatibleFragmentOutput());

    if (rp.DepthStencilTarget().Valid()) {
        const auto& rt = rp.DepthStencilTarget();
        const bool clear = (rt.LoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR);

        EResourceState state = rt.State;
        if (info.EnableEarlyFragmentTests())
            state += EResourceState::EarlyFragmentTests;
        if (info.EnableLateFragmentTests())
            state += EResourceState::LateFragmentTests;
        if (not (info.EnableDepthWrite() | clear))
            state -= EResourceState::_Write;

        rt.Layout = EResourceState_ToImageLayout(state, rt.LocalImage->Read()->AspectMask);

        AddImage_(rt.LocalImage, state, rt.Layout, rt.Desc);
    }

    if (info.EnableRasterizerDiscard())
        return;

    for (auto& rt : rp.ColorTargets()) {
        const EResourceState state = rt.State;
        rt.Layout = EResourceState_ToImageLayout(state, rt.LocalImage->Read()->AspectMask);

        AddImage_(rt.LocalImage, state, rt.Layout, rt.Desc);
    }
}
//----------------------------------------------------------------------------
// Indices
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::BindIndexBuffer_(VkBuffer indexBuffer, VkDeviceSize indexOffset, VkIndexType indexType) {
    if (_indexBuffer != indexBuffer or
        _indexBufferOffset != indexOffset or
        _indexType != indexType ) {
        _indexBuffer = indexBuffer;
        _indexBufferOffset = indexOffset;
        _indexType = indexType;

        vkCmdBindIndexBuffer(_vkCommandBuffer, _indexBuffer, _indexBufferOffset, _indexType);

        ONLY_IF_RHIDEBUG(EditStatistics_([](FFrameStatistics::FRendering& rendering) {
            rendering.NumIndexBufferBindings++;
        }));
    }
}
//----------------------------------------------------------------------------
// Shading rate
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::SetShadingRateImage_(VkImageView* pView, const FVulkanLogicalRenderPass& rp) {
    Assert(pView);

    const FVulkanLocalImage* pLocalImage;
    FImageViewDesc desc;
    if (not rp.ShadingRateImage(&pLocalImage, &desc))
        return;

    *pView = pLocalImage->MakeView(_workerCmd->Device(), false, desc);

    AddImage_(
        pLocalImage,
        EResourceState::ShadingRateImageRead,
        VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV,
        desc );
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::BindShadingRateImage_(VkImageView view) {
    Assert(VK_NULL_HANDLE != view);

    if (_shadingRateImage != view) {
        _shadingRateImage = view;

        vkCmdBindShadingRateImageNV(
            _vkCommandBuffer,
            _shadingRateImage,
            VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV );
    }
}
//----------------------------------------------------------------------------
// Render pass
//----------------------------------------------------------------------------
bool FVulkanTaskProcessor::CreateRenderPass_(TMemoryView<FVulkanLogicalRenderPass* const> passes ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    using FAttachments = TFixedSizeStack<TPair<FRawImageID, FImageViewDesc>, MaxColorBuffers + 1>;

    FVulkanResourceManager& resources = _workerCmd->ResourceManager();

    const FRawRenderPassID renderPassId = resources.CreateRenderPass(passes ARGS_IF_RHIDEBUG(debugName));
    LOG_CHECK(RHI, renderPassId.Valid());

    const FVulkanRenderPass& renderPass = resources.ResourceData(renderPassId);
    const u32 depthAttachmentIndex = (renderPass.Read()->CreateInfo.attachmentCount - 1);
    Assert(depthAttachmentIndex < MaxColorBuffers);

    FRectangleI totalArea;
    FAttachments renderTargets;
    for (const FVulkanLogicalRenderPass* pLogicalRenderPass : passes) {
        Assert(pLogicalRenderPass);

        totalArea.Add(pLogicalRenderPass->Area());

        if (pLogicalRenderPass->DepthStencilTarget().Valid()) {
            auto& src = pLogicalRenderPass->DepthStencilTarget();
            auto& dst = renderTargets[depthAttachmentIndex];

            // compare attachments
            LOG_CHECK(RHI, not dst.first or (dst.first == src.ImageId && dst.second == src.Desc));

            dst.first = src.ImageId;
            dst.second = src.Desc;
        }

        for (auto& src : pLogicalRenderPass->ColorTargets()) {
            auto& dst = renderTargets[src.Index];

            // compare attachments
            LOG_CHECK(RHI, not dst.first or (dst.first == src.ImageId && dst.second == src.Desc));

            dst.first = src.ImageId;
            dst.second = src.Desc;
        }
    }

    const FRawFramebufferID framebufferId = resources.CreateFramebuffer(
        renderTargets.MakeView(),
        renderPassId,  checked_cast<unsigned int>(totalArea.Extents()), 1
        ARGS_IF_RHIDEBUG(debugName) );
    LOG_CHECK(RHI, framebufferId.Valid());

    u32 subpass = 0;
    for (FVulkanLogicalRenderPass* pLogicalRenderPass : passes) {
        pLogicalRenderPass->SetRenderPass(
            renderPassId, subpass++, framebufferId, depthAttachmentIndex );
    }

    return true;
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::BeginRenderPass_(const TVulkanFrameTask<FSubmitRenderPass>& task) {
    Assert_NoAssume(not task.IsSubpass());

    TFixedSizeStack<FVulkanLogicalRenderPass*, 32> logicalRenderPasses;

    for (const TVulkanFrameTask<FSubmitRenderPass>* pSubmit = &task; pSubmit; pSubmit = pSubmit->NextSubpass()) {
        Assert(pSubmit->LogicalPass());
        logicalRenderPasses.Push(pSubmit->LogicalPass());
    }

    // add barriers
    Assert(not logicalRenderPasses.empty());
    FDrawTaskBarriers barriers(*this, *logicalRenderPasses[0]);
    const EResourceState stages = _workerCmd->Device().GraphicsShaderStages();

    for (FVulkanLogicalRenderPass* pLogicalRenderPass : logicalRenderPasses) {

        for (auto& draw : pLogicalRenderPass->DrawTasks()) {
            draw->Process1(&barriers);
        }
        for (auto& item : pLogicalRenderPass->MutableImages()) {
            const auto& sharedImg = item.first->Read();
            AddImage_(
                item.first,
                (item.second | stages),
                EResourceState_ToImageLayout(item.second, sharedImg->AspectMask),
                FImageViewDesc{ sharedImg->Desc } );
        }
        for (auto& item : pLogicalRenderPass->MutableBuffers()) {
            AddBuffer_(item.first, item.second, 0, VK_WHOLE_SIZE);
        }
    }

    // set shading rate
    VkImageView shadingRateView = VK_NULL_HANDLE;
    SetShadingRateImage_(&shadingRateView, *task.LogicalPass());

    // commit all barriers
    AddRenderTargetBarriers_(*task.LogicalPass(), barriers);
    CommitBarriers_();

    // create render pass and frame buffer
    VerifyRelease( CreateRenderPass_(logicalRenderPasses.MakeView() ARGS_IF_RHIDEBUG(task.TaskName())) );

    // begin render pass
    const FRectangleI& area = task.LogicalPass()->Area();
    Assert(area.HasPositiveExtentsStrict());
    const FVulkanFramebuffer* const pFramebuffer = Resource_(task.LogicalPass()->FramebufferId());
    Assert(pFramebuffer);
    const FVulkanRenderPass* const pRenderPass = Resource_(task.LogicalPass()->RenderPassId());
    Assert(pRenderPass);
    const auto sharedRp = pRenderPass->Read();
    Assert_NoAssume(checked_cast<u32>(task.LogicalPass()->ClearValues().size()) >= sharedRp->CreateInfo.attachmentCount);

    VkRenderPassBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = sharedRp->RenderPass;
    info.renderArea.offset.x = area.Left();
    info.renderArea.offset.y = area.Top();
    info.renderArea.extent.width = area.Width();
    info.renderArea.extent.height = area.Height();
    info.clearValueCount = sharedRp->CreateInfo.attachmentCount;
    info.pClearValues = task.LogicalPass()->ClearValues().data();

    vkCmdBeginRenderPass(_vkCommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

    BindShadingRateImage_(shadingRateView);
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::BeginSubpass_(const TVulkanFrameTask<FSubmitRenderPass>& task) {
    Assert(task.IsSubpass());

    // #TODO: barriers for attachments

    vkCmdNextSubpass(_vkCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

    // #TODO: clear attachments

    //vkCmdClearAttachments(_vkCommandBuffer, checked_cast<u32>(task.LogicalPass()->Render) );
}
//----------------------------------------------------------------------------
// Pipeline resources
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::ExtractDescriptorSets_(
    FVulkanDescriptorSets* pDescriptorSets,
    const FVulkanPipelineLayout& layout,
    const FVulkanPipelineResourceSet& resourceSet ) {
    Assert(pDescriptorSets);

    const u32 firstDescriptorSet = layout.Read()->FirstDescriptorSet;

    TStaticArray<TPair<u32, u32>, MaxDescriptorSets> newDynamicOffsets;
    const auto oldDynamicOffsets = resourceSet.DynamicOffsets;

    pDescriptorSets->Resize(resourceSet.Resources.size());

    for (const auto& res : resourceSet.Resources) {
        u32 binding = 0;
        FRawDescriptorSetLayoutID dsLayoutId;
        if (not layout.DescriptorSetLayout(&binding, &dsLayoutId, res.DescriptorSetId))
            continue;

        FPipelineResourceBarriers barriers{ *this, resourceSet.DynamicOffsets.MakeView() };
        res.PipelineResources->EachUniform(barriers);

        Assert_NoAssume(binding >= firstDescriptorSet);
        Assert_NoAssume(res.PipelineResources->Read()->LayoutId == dsLayoutId);
        binding -= firstDescriptorSet;

        (*pDescriptorSets)[binding] = res.PipelineResources->Handle();
        newDynamicOffsets[binding] = { res.DynamicOffsetIndex, res.DynamicOffsetCount };
    }

    // sort dynamic offsets by binding index

    u32 dst = 0;
    for (const TPair<u32, u32>& range : newDynamicOffsets) {
        forrange(i, 0, range.second)
            resourceSet.DynamicOffsets[dst++] = oldDynamicOffsets[range.first + i];
    }
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::BindPipelineResources_(
    const FVulkanPipelineLayout& layout,
    const FVulkanPipelineResourceSet& resourceSet,
    VkPipelineBindPoint bindPoint
    ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex)) {

    // update descriptor sets and add pipeline barriers

    FVulkanDescriptorSets descriptorSets;
    ExtractDescriptorSets_(&descriptorSets, layout, resourceSet);

    if (not descriptorSets.empty()) {
        vkCmdBindDescriptorSets(
            _vkCommandBuffer,
            bindPoint,
            layout.Handle(),
            layout.Read()->FirstDescriptorSet,
            checked_cast<u32>(descriptorSets.size()),
            descriptorSets.data(),
            checked_cast<u32>(resourceSet.DynamicOffsets.size()),
            resourceSet.DynamicOffsets.data() );

        ONLY_IF_RHIDEBUG(EditStatistics_([](FFrameStatistics::FRendering& rendering) {
            rendering.NumDescriptorBinds++;
        }));
    }

#if USE_PPE_RHIDEBUG
    // bind debug mode descriptor sets if needed

    if (debugModeIndex != Default) {
        VkDescriptorSet vkDescriptorSet;
        u32 binding, dynamicOffset;
        if (_workerCmd->Batch()->FindDescriptorSetForDebug(
            &binding, &vkDescriptorSet, &dynamicOffset,
            debugModeIndex ) ) {

            vkCmdBindDescriptorSets(
                _vkCommandBuffer,
                bindPoint,
                layout.Handle(),
                binding,
                1, &vkDescriptorSet,
                1, &dynamicOffset );

            ONLY_IF_RHIDEBUG(EditStatistics_([](FFrameStatistics::FRendering& rendering) {
                rendering.NumDescriptorBinds++;
            }));
        }
    }
#endif
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::BindPipelinePerPassStates_(const FVulkanLogicalRenderPass& rp, VkPipeline vkPipeline) {
    if (_graphicsPipeline.Pipeline != vkPipeline) {
        _graphicsPipeline.Pipeline = vkPipeline;
        vkCmdBindPipeline(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

        ONLY_IF_RHIDEBUG(EditStatistics_([](FFrameStatistics::FRendering& rendering) {
            rendering.NumGraphicsPipelineBindings++;
        }));
    }

    // all pipelines un current render pass have same viewport count and same dynamic states,
    // so those values should not be invalidated
    if (_perPassStatesUpdated)
        return;

    _perPassStatesUpdated = true;

    if (not rp.Viewports().empty()) {
        vkCmdSetViewport(
            _vkCommandBuffer,
            0,
            checked_cast<u32>(rp.Viewports().size()),
            rp.Viewports().data() );
    }

    if (not rp.ShadingRatePalette().empty()) {
        vkCmdSetViewportShadingRatePaletteNV(
            _vkCommandBuffer,
            0,
            checked_cast<u32>(rp.ShadingRatePalette().size()),
            rp.ShadingRatePalette().data() );
    }
}
//----------------------------------------------------------------------------
// Bind pipeline
//----------------------------------------------------------------------------
bool FVulkanTaskProcessor::BindPipeline_(
    FVulkanPipelineLayout const** pPplnLayout,
    const FVulkanLogicalRenderPass& rp,
    const details::FVulkanBaseDrawVerticesTask& task ) {
    Assert(pPplnLayout);

    EPipelineDynamicState pipelineDynamicStates = (
        EPipelineDynamicState::Scissor | EPipelineDynamicState::Viewport );

    FRenderState renderState;
    renderState.Blend = rp.BlendState();
    renderState.Depth = rp.DepthState();
    renderState.Stencil = rp.StencilState();
    renderState.Rasterization = rp.RasterizationState();
    renderState.Multisample = rp.MultisampleState();

    renderState.InputAssembly.Topology = task.Topology;
    renderState.InputAssembly.EnablePrimitiveRestart = task.EnablePrimitiveRestart;

    OverrideBlendStates_(&renderState.Blend, task.ColorBuffers);
    OverrideDepthStencilStates_(
        &renderState.Depth,
        &renderState.Stencil,
        &renderState.Rasterization,
        &pipelineDynamicStates,
        task.DynamicStates );
    SetupExtensions_(&pipelineDynamicStates, rp);

    VkPipeline vkPipeline;
    LOG_CHECK(RHI, _workerCmd->Write()->PipelineCache.CreatePipelineInstance(
        &vkPipeline, pPplnLayout,
        *_workerCmd, rp, *task.Pipeline,
        task.VertexInput, renderState, pipelineDynamicStates
        ARGS_IF_RHIDEBUG(task.DebugModeIndex()) ));

    BindPipelinePerPassStates_(rp, vkPipeline);
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanTaskProcessor::BindPipeline_(
    FVulkanPipelineLayout const** pPplnLayout,
    const FVulkanLogicalRenderPass& rp,
    const details::FVulkanBaseDrawMeshesTask& task ) {
    Assert(pPplnLayout);

    EPipelineDynamicState pipelineDynamicStates = (
        EPipelineDynamicState::Scissor | EPipelineDynamicState::Viewport );

    FRenderState renderState;
    renderState.Blend = rp.BlendState();
    renderState.Depth = rp.DepthState();
    renderState.Stencil = rp.StencilState();
    renderState.Rasterization = rp.RasterizationState();
    renderState.Multisample = rp.MultisampleState();

    OverrideBlendStates_(&renderState.Blend, task.ColorBuffers);
    OverrideDepthStencilStates_(
        &renderState.Depth,
        &renderState.Stencil,
        &renderState.Rasterization,
        &pipelineDynamicStates,
        task.DynamicStates );
    SetupExtensions_(&pipelineDynamicStates, rp);

    VkPipeline vkPipeline;
    LOG_CHECK(RHI, _workerCmd->Write()->PipelineCache.CreatePipelineInstance(
        &vkPipeline, pPplnLayout,
        *_workerCmd, rp, *task.Pipeline,
        renderState, pipelineDynamicStates
        ARGS_IF_RHIDEBUG(task.DebugModeIndex()) ));

    BindPipelinePerPassStates_(rp, vkPipeline);
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanTaskProcessor::BindPipeline_(
    FVulkanPipelineLayout const** pPplnLayout,
    const FVulkanComputePipeline& compute,
    const Meta::TOptional<uint3>& localSize,
    VkPipelineCreateFlags flags
    ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex) ) {

    VkPipeline vkPipeline;
    LOG_CHECK(RHI, _workerCmd->Write()->PipelineCache.CreatePipelineInstance(
        &vkPipeline, pPplnLayout,
        *_workerCmd,
        compute, localSize, flags
        ARGS_IF_RHIDEBUG(debugModeIndex) ));

    Assert(VK_NULL_HANDLE != vkPipeline);

    if (_computePipeline.Pipeline != vkPipeline) {
        _computePipeline.Pipeline = vkPipeline;

        vkCmdBindPipeline(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline);

        ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
            rendering.NumComputePipelineBindings++;
        }));
    }

    return true;
}
//----------------------------------------------------------------------------
// Barriers
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::CommitBarriers_() {
    const auto exclusiveWorker = _workerCmd->Write();
    FVulkanBarrierManager& barriers = exclusiveWorker->BarrierManager;

    for (auto& pending : _pendingResourceBarriers) {
        pending.second(pending.first, barriers ARGS_IF_RHIDEBUG(exclusiveWorker->Debugger.get()));
    }

    _pendingResourceBarriers.clear();

#if USE_PPE_RHIDEBUG && USE_PPE_DEBUG
    if (exclusiveWorker->DebugFullBarriers) {
        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask =
            VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
            VK_ACCESS_INDEX_READ_BIT |
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
            VK_ACCESS_UNIFORM_READ_BIT |
            VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
            VK_ACCESS_SHADER_READ_BIT |
            VK_ACCESS_SHADER_WRITE_BIT |
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_TRANSFER_READ_BIT |
            VK_ACCESS_TRANSFER_WRITE_BIT |
            VK_ACCESS_HOST_READ_BIT |
            VK_ACCESS_HOST_WRITE_BIT;

        const FVulkanDevice& device = _workerCmd->Device();
        EQueueUsage queueUsage = exclusiveWorker->Batch->QueueUsage();
        UNUSED(device);
        UNUSED(queueUsage);

        if ((queueUsage & EQueueUsage::Graphics) and device.Enabled().ShadingRateImageNV)
            barrier.srcAccessMask |= VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV;

        if ((queueUsage & (EQueueUsage::Graphics | EQueueUsage::AsyncCompute)) and device.Enabled().RayTracingKHR)
            barrier.srcAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;

        barrier.dstAccessMask = barrier.srcAccessMask;

        barriers.AddMemoryBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, barrier);
    }
#endif

    barriers.Commit(_workerCmd->Device(), _vkCommandBuffer);
}

//----------------------------------------------------------------------------
// PushConstants
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::PushContants_(const FVulkanPipelineLayout& layout, const FPushConstantDatas& pushConstants) {
    const FPipelineDesc::FPushConstants& map = layout.Read()->PushConstants;
    AssertReleaseMessage(
        L"will be used push constants from previous draw/dispatch calls or may contains undefined values",
        map.size() == pushConstants.size() );

    for (const FPushConstantData& src : pushConstants) {
        if (const FPipelineDesc::FPushConstant* const pPushConstant = map.GetIFP(src.Id)) {
            Assert_NoAssume(src.Size == pPushConstant->Size);

            vkCmdPushConstants(
                _vkCommandBuffer,
                layout.Handle(),
                VkCast(pPushConstant->StageFlags),
                pPushConstant->Offset,
                pPushConstant->Size,
                src.Data );

            ONLY_IF_RHIDEBUG(EditStatistics_([](FFrameStatistics::FRendering& rendering) {
                rendering.NumPushConstants++;
            }));
        }
    }
}
//----------------------------------------------------------------------------
// Submit
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanSubmitRenderPassTask& task) {
    if (task.LogicalPass()->DrawTasks().empty())
        return;

    // invalidate state partially
    _isDefaultScissor = false;
    _perPassStatesUpdated = false;

    if (not task.IsSubpass()) {
        ONLY_IF_RHIDEBUG( CmdPushDebugGroup_(task.TaskName(), task.DebugColor()) );

        BeginRenderPass_(task);
    }
    else {
        ONLY_IF_RHIDEBUG( CmdPopDebugGroup_() );
        ONLY_IF_RHIDEBUG( CmdPushDebugGroup_(task.TaskName(), task.DebugColor()) );

        BeginSubpass_(task);
    }

    // draw
    FDrawTaskCommands commands{ *this, &task, _vkCommandBuffer };

    for (IVulkanDrawTask* pDraw : task.LogicalPass()->DrawTasks()) {
        Assert(pDraw);
        pDraw->Process2(&commands);
    }

    // end render pass
    if (task.IsLastPass()) {
        vkCmdEndRenderPass(_vkCommandBuffer);

        ONLY_IF_RHIDEBUG( CmdPopDebugGroup_() );
    }
}
//----------------------------------------------------------------------------
// Compute dispatch
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanDispatchComputeTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    const FVulkanPipelineLayout* pLayout = nullptr;
    LOG_CHECKVOID(RHI, BindPipeline_(
        &pLayout,
        *task.Pipeline, task.LocalGroupSize,
        VK_PIPELINE_CREATE_DISPATCH_BASE
        ARGS_IF_RHIDEBUG(task.DebugModeIndex) ));

    BindPipelineResources_(
        *pLayout,
        task.Resources(),
        VK_PIPELINE_BIND_POINT_COMPUTE
        ARGS_IF_RHIDEBUG(task.DebugModeIndex) );
    PushContants_(*pLayout, task.PushConstants);

    CommitBarriers_();

    for (auto& cmd : task.Commands) {
        if (_enableDispatchBase) {
            vkCmdDispatchBaseKHR(
                _vkCommandBuffer,
                cmd.BaseGroup.x, cmd.BaseGroup.y, cmd.BaseGroup.z,
                cmd.GroupCount.x, cmd.GroupCount.y, cmd.GroupCount.z );
        }
        else {
            Assert(cmd.BaseGroup == uint3::Zero);
            vkCmdDispatch(
                _vkCommandBuffer,
                cmd.GroupCount.x, cmd.GroupCount.y, cmd.GroupCount.z );
        }
    }

    ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumDispatchCalls += checked_cast<u32>(task.Commands.size());
    }));
}
//----------------------------------------------------------------------------*
void FVulkanTaskProcessor::Visit(const FVulkanDispatchComputeIndirectTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    const FVulkanPipelineLayout* pLayout = nullptr;
    LOG_CHECKVOID(RHI, BindPipeline_(
        &pLayout,
        *task.Pipeline, task.LocalGroupSize,
        0
        ARGS_IF_RHIDEBUG(task.DebugModeIndex) ));

    BindPipelineResources_(
        *pLayout,
        task.Resources(),
        VK_PIPELINE_BIND_POINT_COMPUTE
        ARGS_IF_RHIDEBUG(task.DebugModeIndex) );
    PushContants_(*pLayout, task.PushConstants);

    for (auto& cmd : task.Commands) {
        AddBuffer_(
            task.IndirectBuffer,
            EResourceState::IndirectBuffer,
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset),
            sizeof(FDispatchComputeIndirect::FIndirectCommand) );
    }

    CommitBarriers_();

    for (auto& cmd : task.Commands) {
        vkCmdDispatchIndirect(
            _vkCommandBuffer,
            task.IndirectBuffer->Handle(),
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset) );
    }

    ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumDispatchCalls += checked_cast<u32>(task.Commands.size());
    }));
}
//----------------------------------------------------------------------------
// Transfer ops
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanCopyBufferTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    const auto srcBuffer = task.SrcBuffer->Read();
    const auto dstBuffer = task.DstBuffer->Read();

    FBufferCopyRegions regions;
    regions.Resize(task.Regions.size());

    forrange(i, 0, task.Regions.size()) {
        const FCopyBuffer::FRegion& src = task.Regions[i];

        VkBufferCopy& dst = regions[i];
        dst.srcOffset = static_cast<VkDeviceSize>(src.SrcOffset);
        dst.dstOffset = static_cast<VkDeviceSize>(src.DstOffset);
        dst.size = static_cast<VkDeviceSize>(src.Size);

        Assert(src.Size + src.SrcOffset <= srcBuffer->SizeInBytes());
        Assert(src.Size + src.DstOffset <= dstBuffer->SizeInBytes());

        using range_t = TRange<VkDeviceSize>;
        Assert_NoAssume( (task.SrcBuffer != task.DstBuffer) ||
            range_t(dst.srcOffset, dst.srcOffset + dst.size)
                .Overlaps(range_t(dst.dstOffset, dst.dstOffset + dst.size )) );

        AddBuffer_(task.SrcBuffer, EResourceState::TransferSrc, dst.srcOffset, dst.size);
        AddBuffer_(task.DstBuffer, EResourceState::TransferDst, dst.dstOffset, dst.size);
    }

    CommitBarriers_();

    vkCmdCopyBuffer(
        _vkCommandBuffer,
        srcBuffer->vkBuffer,
        dstBuffer->vkBuffer,
        checked_cast<u32>(regions.size()),
        regions.data() );

    ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumTransferOps++;
    }));
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanCopyImageTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    const auto srcImage = task.SrcImage->Read();
    const auto dstImage = task.DstImage->Read();

    FImageCopyRegions regions;
    regions.Resize(task.Regions.size());

    forrange(i, 0, task.Regions.size()) {
        const FCopyImage::FRegion& src = task.Regions[i];
        const uint3 imageSize = Max(src.Size, 1u);

        VkImageCopy& dst = regions[i];
        dst.srcSubresource.aspectMask = VkCast(src.SrcSubresource.AspectMask, srcImage->PixelFormat());
        dst.srcSubresource.mipLevel = *src.SrcSubresource.MipLevel;
        dst.srcSubresource.baseArrayLayer = *src.SrcSubresource.BaseLayer;
        dst.srcSubresource.layerCount = src.SrcSubresource.LayerCount;
        dst.srcOffset = VkOffset3D{ src.SrcOffset.x, src.SrcOffset.y, src.SrcOffset.z };

        dst.dstSubresource.aspectMask = VkCast(src.DstSubresource.AspectMask, srcImage->PixelFormat());
        dst.dstSubresource.mipLevel = *src.DstSubresource.MipLevel;
        dst.dstSubresource.baseArrayLayer = *src.DstSubresource.BaseLayer;
        dst.dstSubresource.layerCount = src.DstSubresource.LayerCount;
        dst.dstOffset = VkOffset3D{ src.DstOffset.x, src.DstOffset.y, src.DstOffset.z };

        dst.extent = VkExtent3D{ imageSize.x, imageSize.y, imageSize.z };

        Assert_NoAssume(src.SrcSubresource.MipLevel < srcImage->Desc.MaxLevel);
        Assert_NoAssume(src.SrcSubresource.BaseLayer + src.SrcSubresource.LayerCount <= srcImage->ArrayLayers());
        Assert_NoAssume(src.DstSubresource.MipLevel < dstImage->Desc.MaxLevel);
        Assert_NoAssume(src.DstSubresource.BaseLayer + src.DstSubresource.LayerCount <= dstImage->ArrayLayers());

        AddImage_(task.SrcImage, EResourceState::TransferSrc, task.SrcLayout, dst.srcSubresource);
        AddImage_(task.DstImage, EResourceState::TransferDst, task.DstLayout, dst.dstSubresource);
    }

    CommitBarriers_();

    vkCmdCopyImage(
        _vkCommandBuffer,
        srcImage->vkImage,
        task.SrcLayout,
        dstImage->vkImage,
        task.DstLayout,
        checked_cast<u32>(regions.size()),
        regions.data() );

    ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumTransferOps++;
    }));
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanCopyBufferToImageTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    const auto srcBuffer = task.SrcBuffer->Read();
    const auto dstImage = task.DstImage->Read();

    FBufferImageCopyRegions regions;
    regions.Resize(task.Regions.size());

    forrange(i, 0, task.Regions.size()) {
        const FCopyBufferToImage::FRegion& src = task.Regions[i];
        const int3 imageOffset = src.ImageOffset;
        const uint3 imageSize = Max(src.ImageSize, 1u);

        VkBufferImageCopy& dst = regions[i];
        dst.bufferOffset = static_cast<VkDeviceSize>(src.BufferOffset);
        dst.bufferRowLength = src.BufferRowLength;
        dst.bufferImageHeight = src.BufferImageHeight;

        dst.imageSubresource.aspectMask = VkCast(src.ImageLayers.AspectMask, dstImage->PixelFormat());
        dst.imageSubresource.mipLevel = *src.ImageLayers.MipLevel;
        dst.imageSubresource.baseArrayLayer = *src.ImageLayers.BaseLayer;
        dst.imageSubresource.layerCount = src.ImageLayers.LayerCount;
        dst.imageOffset = VkOffset3D{ imageOffset.x, imageOffset.y, imageOffset.z };
        dst.imageExtent = VkExtent3D{ imageSize.x, imageSize.y, imageSize.z };

        AddBuffer_(task.SrcBuffer, EResourceState::TransferSrc, dst, task.DstImage);
        AddImage_(task.DstImage, EResourceState::TransferDst, task.DstLayout, dst.imageSubresource);
    }

    CommitBarriers_();

    vkCmdCopyBufferToImage(
        _vkCommandBuffer,
        srcBuffer->vkBuffer,
        dstImage->vkImage,
        task.DstLayout,
        checked_cast<u32>(regions.size()),
        regions.data() );

    ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumTransferOps++;
    }));
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanCopyImageToBufferTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    const auto srcImage = task.SrcImage->Read();
    const auto dstBuffer = task.DstBuffer->Read();

    FBufferImageCopyRegions regions;
    regions.Resize(task.Regions.size());

    forrange(i, 0, task.Regions.size()) {
        const FCopyBufferToImage::FRegion& src = task.Regions[i];
        const int3 imageOffset = src.ImageOffset;
        const uint3 imageSize = Max(src.ImageSize, 1u);

        VkBufferImageCopy& dst = regions[i];
        dst.bufferOffset = static_cast<VkDeviceSize>(src.BufferOffset);
        dst.bufferRowLength = src.BufferRowLength;
        dst.bufferImageHeight = src.BufferImageHeight;

        dst.imageSubresource.aspectMask = VkCast(src.ImageLayers.AspectMask, srcImage->PixelFormat());
        dst.imageSubresource.mipLevel = *src.ImageLayers.MipLevel;
        dst.imageSubresource.baseArrayLayer = *src.ImageLayers.BaseLayer;
        dst.imageSubresource.layerCount = src.ImageLayers.LayerCount;
        dst.imageOffset = VkOffset3D{ imageOffset.x, imageOffset.y, imageOffset.z };
        dst.imageExtent = VkExtent3D{ imageSize.x, imageSize.y, imageSize.z };

        AddImage_(task.SrcImage, EResourceState::TransferSrc, task.SrcLayout, dst.imageSubresource);
        AddBuffer_(task.DstBuffer, EResourceState::TransferDst, dst, task.SrcImage);
    }

    CommitBarriers_();

    vkCmdCopyImageToBuffer(
        _vkCommandBuffer,
        srcImage->vkImage,
        task.SrcLayout,
        dstBuffer->vkBuffer,
        checked_cast<u32>(regions.size()),
        regions.data() );

    ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumTransferOps++;
    }));
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanBlitImageTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    const auto srcImage = task.SrcImage->Read();
    const auto dstImage = task.DstImage->Read();

    FBlitRegions regions;
    regions.Resize(task.Regions.size());

    forrange(i, 0, task.Regions.size()) {
        const FBlitImage::FRegion& src = task.Regions[i];

        VkImageBlit& dst = regions[i];
        dst.srcSubresource.aspectMask = VkCast(src.SrcSubresource.AspectMask, srcImage->PixelFormat());
        dst.srcSubresource.mipLevel = *src.SrcSubresource.MipLevel;
        dst.srcSubresource.baseArrayLayer = *src.SrcSubresource.BaseLayer;
        dst.srcSubresource.layerCount = src.SrcSubresource.LayerCount;
        dst.srcOffsets[0] = VkOffset3D{ src.SrcOffset0.x, src.SrcOffset0.y, src.SrcOffset0.z };
        dst.srcOffsets[1] = VkOffset3D{ src.SrcOffset1.x, src.SrcOffset1.y, src.SrcOffset1.z };

        dst.dstSubresource.aspectMask = VkCast(src.DstSubresource.AspectMask, srcImage->PixelFormat());
        dst.dstSubresource.mipLevel = *src.DstSubresource.MipLevel;
        dst.dstSubresource.baseArrayLayer = *src.DstSubresource.BaseLayer;
        dst.dstSubresource.layerCount = src.DstSubresource.LayerCount;
        dst.dstOffsets[0] = VkOffset3D{ src.DstOffset0.x, src.SrcOffset0.y, src.SrcOffset0.z };
        dst.dstOffsets[1] = VkOffset3D{ src.DstOffset1.x, src.DstOffset1.y, src.DstOffset1.z };

        AddImage_(task.SrcImage, EResourceState::TransferSrc, task.SrcLayout, dst.srcSubresource);
        AddImage_(task.DstImage, EResourceState::TransferDst, task.DstLayout, dst.dstSubresource);
    }

    CommitBarriers_();

    vkCmdBlitImage(
        _vkCommandBuffer,
        srcImage->vkImage,
        task.SrcLayout,
        dstImage->vkImage,
        task.DstLayout,
        checked_cast<u32>(regions.size()),
        regions.data(),
        task.Filter );

    ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumTransferOps++;
    }));
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanResolveImageTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    const auto srcImage = task.SrcImage->Read();
    const auto dstImage = task.DstImage->Read();

    FResolveRegions regions;
    regions.Resize(task.Regions.size());

    forrange(i, 0, task.Regions.size()) {
        const FResolveImage::FRegion& src = task.Regions[i];
        const uint3 imageSize = Max(src.Extent, 1u);

        VkImageResolve& dst = regions[i];
        dst.srcSubresource.aspectMask = VkCast(src.SrcSubresource.AspectMask, srcImage->PixelFormat());
        dst.srcSubresource.mipLevel = *src.SrcSubresource.MipLevel;
        dst.srcSubresource.baseArrayLayer = *src.SrcSubresource.BaseLayer;
        dst.srcSubresource.layerCount = src.SrcSubresource.LayerCount;
        dst.srcOffset = VkOffset3D{ src.SrcOffset.x, src.SrcOffset.y, src.SrcOffset.z };

        dst.dstSubresource.aspectMask = VkCast(src.DstSubresource.AspectMask, srcImage->PixelFormat());
        dst.dstSubresource.mipLevel = *src.DstSubresource.MipLevel;
        dst.dstSubresource.baseArrayLayer = *src.DstSubresource.BaseLayer;
        dst.dstSubresource.layerCount = src.DstSubresource.LayerCount;
        dst.dstOffset = VkOffset3D{ src.DstOffset.x, src.SrcOffset.y, src.SrcOffset.z };

        dst.extent = VkExtent3D{ imageSize.x, imageSize.y, imageSize.z };

        AddImage_(task.SrcImage, EResourceState::TransferSrc, task.SrcLayout, dst.srcSubresource);
        AddImage_(task.DstImage, EResourceState::TransferDst, task.DstLayout, dst.dstSubresource);
    }

    CommitBarriers_();

    vkCmdResolveImage(
        _vkCommandBuffer,
        srcImage->vkImage,
        task.SrcLayout,
        dstImage->vkImage,
        task.DstLayout,
        checked_cast<u32>(regions.size()),
        regions.data() );

    ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumTransferOps++;
    }));
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanGenerateMipmapsTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    const auto sharedImage = task.Image->Read();
    const uint3 dimension = (sharedImage->Dimensions() / (1u << task.BaseMipLevel));

    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = sharedImage->AspectMask;
    subresourceRange.baseArrayLayer = task.BaseLayer;
    subresourceRange.layerCount = Min(task.LayerCount, sharedImage->ArrayLayers());
    subresourceRange.baseMipLevel = task.BaseMipLevel;
    subresourceRange.levelCount = Min(task.LevelCount, sharedImage->MipmapLevels() - Min(sharedImage->MipmapLevels() - 1, task.BaseMipLevel));

    if (subresourceRange.levelCount == 1)
        return;

    AddImage_(task.Image, EResourceState::TransferSrc, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);
    CommitBarriers_();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = sharedImage->vkImage;

    forrange(i, 1, subresourceRange.levelCount) {
        const int3 srcSize = checked_cast<int>(Max(1u, dimension / (1 << (i - 1))));
        const int3 dstSize = checked_cast<int>(Max(1u, dimension / (1 << i)));

        // transition from undefined to optimal layout

        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.subresourceRange = {
            subresourceRange.aspectMask,
            subresourceRange.baseMipLevel + i, 1,
            subresourceRange.baseArrayLayer, subresourceRange.layerCount };

        vkCmdPipelineBarrier(
            _vkCommandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier );

        VkImageBlit region{};
        region.srcOffsets[0] = { 0, 0, 0 };
        region.srcOffsets[1] = { srcSize.x, srcSize.y, srcSize.z };
        region.srcSubresource = {
            subresourceRange.aspectMask,
            (subresourceRange.baseMipLevel + i - 1),
            subresourceRange.baseArrayLayer,
            subresourceRange.layerCount };
        region.dstOffsets[0] = { 0, 0, 0 };
        region.dstOffsets[1] = { dstSize.x, dstSize.y, dstSize.z };
        region.dstSubresource = {
            subresourceRange.aspectMask,
            (subresourceRange.baseMipLevel + i - 1),
            subresourceRange.baseArrayLayer,
            subresourceRange.layerCount };

        vkCmdBlitImage(
            _vkCommandBuffer,
            sharedImage->vkImage,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            sharedImage->vkImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &region,
            VK_FILTER_LINEAR );

        ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
            rendering.NumTransferOps++;
        }));

        // read after write

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.subresourceRange = {
            subresourceRange.aspectMask,
            (subresourceRange.baseMipLevel + 1),
            1,
            subresourceRange.baseArrayLayer,
            subresourceRange.layerCount };

        vkCmdPipelineBarrier(
            _vkCommandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier );
    }
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanFillBufferTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    AddBuffer_(task.DstBuffer, EResourceState::TransferDst, task.DstOffset, task.Size);
    CommitBarriers_();

    vkCmdFillBuffer(
        _vkCommandBuffer,
        task.DstBuffer->Handle(),
        task.DstOffset,
        task.Size,
        task.Pattern );

    ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumTransferOps++;
    }));
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanClearColorImageTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    const auto dstImage = task.DstImage->Read();

    FImageClearRanges ranges;
    ranges.Resize(task.Ranges.size());

    forrange(i, 0, ranges.size()) {
        const FClearColorImage::FRange& src = task.Ranges[i];

        VkImageSubresourceRange& dst = ranges[i];
        dst.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        dst.baseMipLevel = *src.BaseMipLevel;
        dst.levelCount = src.LevelCount;
        dst.baseArrayLayer = *src.BaseLayer;
        dst.layerCount = src.LayerCount;

        AddImage_(task.DstImage, EResourceState::TransferDst, task.DstLayout, dst);
    }

    CommitBarriers_();

    vkCmdClearColorImage(
        _vkCommandBuffer,
        dstImage->vkImage,
        task.DstLayout,
        &task.ClearValue,
        checked_cast<u32>(ranges.size()),
        ranges.data() );

    ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumTransferOps++;
    }));
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanClearDepthStencilImageTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    const auto dstImage = task.DstImage->Read();

    FImageClearRanges ranges;
    ranges.Resize(task.Ranges.size());

    forrange(i, 0, ranges.size()) {
        const FClearColorImage::FRange& src = task.Ranges[i];

        VkImageSubresourceRange& dst = ranges[i];
        dst.aspectMask = VkCast(src.AspectMask, dstImage->PixelFormat());
        dst.baseMipLevel = *src.BaseMipLevel;
        dst.levelCount = src.LevelCount;
        dst.baseArrayLayer = *src.BaseLayer;
        dst.layerCount = src.LayerCount;

        AddImage_(task.DstImage, EResourceState::TransferDst, task.DstLayout, dst);
    }

    CommitBarriers_();

    vkCmdClearDepthStencilImage(
        _vkCommandBuffer,
        dstImage->vkImage,
        task.DstLayout,
        &task.ClearValue,
        checked_cast<u32>(ranges.size()),
        ranges.data() );

    ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumTransferOps++;
    }));
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanUpdateBufferTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    const auto dstBuffer = task.DstBuffer->Read();

    for (const auto& region : task.Regions) {
        AddBuffer_(task.DstBuffer, EResourceState::TransferDst, region.BufferOffset, region.DataSize);
    }

    CommitBarriers_();

    for (const auto& region : task.Regions) {
        vkCmdUpdateBuffer(
            _vkCommandBuffer,
            dstBuffer->vkBuffer,
            region.BufferOffset,
            region.DataSize,
            region.DataPtr );
    }

    ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumTransferOps += checked_cast<u32>(task.Regions.size());
    }));
}
//----------------------------------------------------------------------------
// Present
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanPresentTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    FRawImageID swapchainImageId;
    VerifyRelease( task.Swapchain->Acquire(&swapchainImageId, *_workerCmd ARGS_IF_RHIDEBUG(_workerCmd->Read()->DebugQueueSync) ));
    Assert(swapchainImageId);

    const FVulkanLocalImage* const pDstImage = ToLocal_(swapchainImageId);
    Assert(pDstImage);
    Assert(pDstImage != task.SrcImage);

    const auto srcImage = task.SrcImage->Read();
    const auto dstImage = pDstImage->Read();
    Assert_NoAssume(dstImage->Desc.Usage & EImageUsage::TransferDst);

    const int3 srcDim = checked_cast<int>(srcImage->Dimensions());
    const int3 dstDim = checked_cast<int>(dstImage->Dimensions());

    const VkFilter filter = (srcImage->Dimensions() == dstImage->Dimensions()
        ? VK_FILTER_NEAREST
        : VK_FILTER_LINEAR );

    VkImageBlit region;
    region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, *task.Mipmap, *task.Layer };
    region.srcOffsets[0] = VkOffset3D{ 0, 0, 0 };
    region.srcOffsets[1] = VkOffset3D{ srcDim.x, srcDim.y, 1 };

    region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.dstOffsets[0] = VkOffset3D{ 0, 0, 0 };
    region.dstOffsets[1] = VkOffset3D{ dstDim.x, dstDim.y, 1 };

    AddImage_(task.SrcImage, EResourceState::TransferSrc, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, region.srcSubresource);
    AddImage_(pDstImage, EResourceState::TransferDst | EResourceState::InvalidateBefore, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, region.dstSubresource);

    CommitBarriers_();

    vkCmdBlitImage(
        _vkCommandBuffer,
        srcImage->vkImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstImage->vkImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region,
        filter );
}
//----------------------------------------------------------------------------
// RayTracing
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanUpdateRayTracingShaderTableTask& task) {
    if (not _enableRayTracingKHR)
        return;

    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    FVulkanPipelineCache::FBufferCopyRegions regions;
    LOG_CHECKVOID(RHI, _workerCmd->Write()->PipelineCache.CreateShaderTable(
        task.ShaderTable,
        &regions,
        *_workerCmd,
        task.Pipeline,
        *task.RTScene->GlobalData(),
        task.RayGenShader,
        task.ShaderGroups,
        task.MaxRecursionDepth ));

    for (const auto& copy : regions) {
        Assert_NoAssume(copy.SrcBuffer->Read()->Desc.Usage & EBufferUsage::TransferSrc);
        Assert_NoAssume(copy.DstBuffer->Read()->Desc.Usage & EBufferUsage::TransferDst);

        AddBuffer_(copy.SrcBuffer, EResourceState::TransferSrc, copy.Region.srcOffset, copy.Region.size);
        AddBuffer_(copy.DstBuffer, EResourceState::TransferDst, copy.Region.dstOffset, copy.Region.size);
    }

    CommitBarriers_();

    for (const auto& copy : regions) {
        vkCmdCopyBuffer(
            _vkCommandBuffer,
            copy.SrcBuffer->Handle(),
            copy.DstBuffer->Handle(),
            1, &copy.Region );
    }

    ONLY_IF_RHIDEBUG(EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumTransferOps += checked_cast<u32>(regions.size());
    }));
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanBuildRayTracingGeometryTask& task) {
    if (not _enableRayTracingKHR)
        return;

    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

#if 0
    AddRTGeometry_(task.RTGeometry, EResourceState::BuildRayTracingStructWrite);
    AddBuffer_(
        task.ScratchBuffer,
        EResourceState::RTASBuildingBufferReadWrite,
        0, VK_WHOLE_SIZE );

    for (const FVulkanLocalBuffer* pLocalBuffer : task.UsableBuffers) {
        Assert(pLocalBuffer);

        // resource state doesn't matter here

        AddBuffer_(pLocalBuffer, EResourceState::TransferSrc, 0, VK_WHOLE_SIZE);
    }

    CommitBarriers_();

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildInfo.geometryCount = checked_cast<u32>(task.Geometries.size());
    buildInfo.pGeometries = task.Geometries.data();
    buildInfo.dstAccelerationStructure = task.RTGeometry->Handle();
    buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
    buildInfo.flags = VkCast(task.RTGeometry->Flags());

    STACKLOCAL_STACK(VkAccelerationStructureBuildRangeInfoKHR, rangeInfos, task.Geometries.size());
    forrange(i, 0, rangeInfos.size()) {
        rangeInfos[i] = VkAccelerationStructureBuildRangeInfoKHR{};
        //rangeInfos[i].primitiveCount = task. #TODO
    }

    vkCmdBuildAccelerationStructuresKHR(_vkCommandBuffer, 1, &buildInfo, &rangeInfo);

#else
    AssertNotImplemented(); // NV to KHR
#endif
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanBuildRayTracingSceneTask& task) {
    UNUSED(task);
    AssertNotImplemented(); // #TODO: NV to KHR
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanTraceRaysTask& task) {
    UNUSED(task);
    AssertNotImplemented(); // #TODO: NV to KHR
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::Visit(const FVulkanCustomTaskTask& task) {
    ONLY_IF_RHIDEBUG( CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    EResourceState stages = _workerCmd->Device().GraphicsShaderStages();

    for (auto& it : task.Images) {
        const auto sharedImg = it.first->Read();
        const FImageViewDesc desc{ sharedImg->Desc };
        AddImage_(it.first, (it.second | stages), EResourceState_ToImageLayout(it.second, sharedImg->AspectMask), desc);
    }

    for (auto& it : task.Buffers) {
        AddBuffer_(it.first, it.second, 0, VK_WHOLE_SIZE);
    }

    CommitBarriers_();

    task.Callback(_workerCmd.get(), _vkCommandBuffer);
}
//----------------------------------------------------------------------------
// AddImage
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::AddImageState_(const FVulkanLocalImage* pLocalImage, FImageState&& rstate) {
    Assert(pLocalImage);
    Assert_NoAssume(not rstate.Range.Empty());

    _pendingResourceBarriers.insert({ pLocalImage, &CommitResourceBarrier_<FVulkanLocalImage> });

#if USE_PPE_RHIDEBUG
    if (Unlikely(_workerCmd->Debugger()))
        _workerCmd->Debugger()->AddUsage(pLocalImage->GlobalData(), rstate);
#endif

    pLocalImage->AddPendingState(std::move(rstate));
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::AddImage_(
    const FVulkanLocalImage* pLocalImage,
    EResourceState state,
    VkImageLayout layout,
    const FImageViewDesc& desc ) {
    Assert(desc.LayerCount > 0);
    Assert(desc.LevelCount > 0);

    AddImageState_(pLocalImage, FImageState{
        state, layout,
        FImageRange{ desc.BaseLayer, desc.LayerCount, desc.BaseLevel, desc.LevelCount },
        (EPixelFormat_HasDepth(desc.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT :
         EPixelFormat_HasStencil(desc.Format) ? VK_IMAGE_ASPECT_STENCIL_BIT :
         VK_IMAGE_ASPECT_COLOR_BIT ),
        _currentTask
    });
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::AddImage_(
    const FVulkanLocalImage* pLocalImage,
    EResourceState state,
    VkImageLayout layout,
    const VkImageSubresourceLayers& subRes ) {
    AddImageState_(pLocalImage, FImageState{
        state, layout,
        FImageRange{ FImageLayer(subRes.baseArrayLayer), subRes.layerCount, FMipmapLevel(subRes.mipLevel), 1 },
        static_cast<VkImageAspectFlagBits>(subRes.aspectMask),
        _currentTask
    });
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::AddImage_(
    const FVulkanLocalImage* pLocalImage,
    EResourceState state,
    VkImageLayout layout,
    const VkImageSubresourceRange& subRes ) {
    AddImageState_(pLocalImage, FImageState{
        state, layout,
        FImageRange{ FImageLayer(subRes.baseArrayLayer), subRes.layerCount, FMipmapLevel(subRes.baseMipLevel), subRes.levelCount },
        static_cast<VkImageAspectFlagBits>(subRes.aspectMask),
        _currentTask
    });
}
//----------------------------------------------------------------------------
// AddBuffer
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::AddBufferState_(const FVulkanLocalBuffer* pLocalBuffer, FBufferState&& rstate) {
    Assert(pLocalBuffer);

    _pendingResourceBarriers.insert({ pLocalBuffer, &CommitResourceBarrier_<FVulkanLocalBuffer> });

#if USE_PPE_RHIDEBUG
    if (Unlikely(_workerCmd->Debugger()))
        _workerCmd->Debugger()->AddUsage(pLocalBuffer->GlobalData(), rstate);
#endif

    pLocalBuffer->AddPendingState(std::move(rstate));
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::AddBuffer_(
    const FVulkanLocalBuffer* pLocalBuffer,
    EResourceState state,
    VkDeviceSize offset, VkDeviceSize size ) {
    Assert(pLocalBuffer);
    Assert(size > 0);

    const VkDeviceSize bufferSize = static_cast<VkDeviceSize>(pLocalBuffer->Read()->SizeInBytes());

    size = Min(bufferSize, (size == VK_WHOLE_SIZE ? bufferSize - offset : offset + size));

    AddBufferState_(pLocalBuffer, FBufferState{ state, offset, size, _currentTask });
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::AddBuffer_(
    const FVulkanLocalBuffer* pLocalBuffer,
    EResourceState state,
    const VkBufferImageCopy& region,
    const FVulkanLocalImage* pLocalImage ) {
    Assert(pLocalImage);

    const auto sharedImg = pLocalImage->Read();

    const u32 bpp = EPixelFormat_BitsPerPixel(
        sharedImg->PixelFormat(),
        RHICast(static_cast<VkImageAspectFlagBits>(region.imageSubresource.aspectMask)) );
    const VkDeviceSize rowPitch = (region.bufferRowLength * bpp) / 8;
    const VkDeviceSize slicePitch = region.bufferImageHeight * rowPitch;
    const u32 dimZ = Max(region.imageSubresource.layerCount, region.imageExtent.depth);

    // one big barrier

    FBufferState wholeState{ state, 0, slicePitch * dimZ, _currentTask };
    wholeState.Range += region.bufferOffset;

    AddBufferState_(pLocalBuffer, std::move(wholeState));
}
//----------------------------------------------------------------------------
// AddRayTracing
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::AddRTGeometry_(const FVulkanRTLocalGeometry* pLocalGeom, EResourceState state) {
    Assert(pLocalGeom);

    _pendingResourceBarriers.insert({ pLocalGeom, &CommitResourceBarrier_<FVulkanRTLocalGeometry> });

    FRTGeometryState rtState{ state, PFrameTask(_currentTask.get()) };

#if USE_PPE_RHIDEBUG
    if (Unlikely(_workerCmd->Debugger()))
        _workerCmd->Debugger()->AddUsage(pLocalGeom->GlobalData(), rtState);
#endif

    pLocalGeom->AddPendingState(std::move(rtState));
}
//----------------------------------------------------------------------------
void FVulkanTaskProcessor::AddRTScene_(const FVulkanRTLocalScene* pLocalScene, EResourceState state) {
    Assert(pLocalScene);

    _pendingResourceBarriers.insert({ pLocalScene, &CommitResourceBarrier_<FVulkanRTLocalScene> });

    FRTSceneState rtState{ state, PFrameTask(_currentTask.get()) };

#if USE_PPE_RHIDEBUG
    if (Unlikely(_workerCmd->Debugger()))
        _workerCmd->Debugger()->AddUsage(pLocalScene->GlobalData(), rtState);
#endif

    pLocalScene->AddPendingState(std::move(rtState));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
