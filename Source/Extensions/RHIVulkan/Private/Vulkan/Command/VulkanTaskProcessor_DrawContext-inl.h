#pragma once

#include "Vulkan/Command/VulkanTaskProcessor.h"

#include "Vulkan/Pipeline/VulkanPipelineCache.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FVulkanTaskProcessor::FDrawContext::FDrawContext(
    FVulkanTaskProcessor& processor,
    const FVulkanLogicalRenderPass& logicalRenderPass ) NOEXCEPT
:   _processor(processor)
,   _logicalRenderPass(logicalRenderPass)
,   _changed(ALL_BITS) {

    _renderState.Blend = logicalRenderPass.BlendState();
    _renderState.Depth = logicalRenderPass.DepthState();
    _renderState.Stencil = logicalRenderPass.StencilState();
    _renderState.Rasterization = logicalRenderPass.RasterizationState();
    _renderState.Multisample = logicalRenderPass.MultisampleState();
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::Reset() {
    _gPipelineRef.reset();
    _mPipelineRef.reset();
    _pipelineLayoutRef.reset();

    _dynamicStates = Default;
    _vertexInput = Default;
    _changed = ALL_BITS;

    _renderState.Blend = _logicalRenderPass.BlendState();
    _renderState.Depth = _logicalRenderPass.DepthState();
    _renderState.Stencil = _logicalRenderPass.StencilState();
    _renderState.Rasterization = _logicalRenderPass.RasterizationState();
    _renderState.Multisample = _logicalRenderPass.MultisampleState();

    _processor._indexBuffer = VK_NULL_HANDLE;
    _processor._graphicsPipeline = Default;
    _processor._isDefaultScissor = false;
    _processor._perPassStatesUpdated = false;
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::BindPipeline(FRawGPipelineID id, EPipelineDynamicState dynamicState) {
    Assert(id);

    _gPipelineRef = _processor.Resource_(id);
    _mPipelineRef.reset();
    _dynamicStates = dynamicState;
    _changed |= ALL_BITS;

    Assert(!!_gPipelineRef);
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::BindPipeline(FRawMPipelineID id, EPipelineDynamicState dynamicState) {
    Assert(id);

    _gPipelineRef.reset();
    _mPipelineRef = _processor.Resource_(id);
    _dynamicStates = dynamicState;
    _changed |= ALL_BITS;

    Assert(!!_mPipelineRef);
}
//----------------------------------------------------------------------------
inline bool FVulkanTaskProcessor::FDrawContext::BindPipeline_(u32 mask) {
    bool success = true;

    mask = (_changed & mask);

    if (_gPipelineRef && !!(mask & GRAPHICS_BIT)) {
        _changed ^= GRAPHICS_BIT;

        VkPipeline vkPipeline;
        if (_processor._workerCmd->Write()->PipelineCache.CreatePipelineInstance(
                &vkPipeline, _pipelineLayoutRef.ref(),
                *_processor._workerCmd, _logicalRenderPass, *_gPipelineRef,
                _vertexInput, _renderState, _dynamicStates ARGS_IF_RHIDEBUG(Default) )) {

            _processor.BindPipelinePerPassStates_(_logicalRenderPass, vkPipeline);
            _processor.SetScissor_(_logicalRenderPass, Default);
        }
        else {
            RHI_LOG(Warning, L"failed to create graphics pipeline instance for <{0}>", _gPipelineRef->DebugName());
            success = false;
        }
    }

    if (_mPipelineRef && !!(mask & MESH_BIT)) {
        _changed ^= MESH_BIT;

        VkPipeline vkPipeline;
        if (_processor._workerCmd->Write()->PipelineCache.CreatePipelineInstance(
                &vkPipeline, _pipelineLayoutRef.ref(),
                *_processor._workerCmd, _logicalRenderPass, *_mPipelineRef,
                _renderState, _dynamicStates ARGS_IF_RHIDEBUG(Default) )) {

            _processor.BindPipelinePerPassStates_(_logicalRenderPass, vkPipeline);
            _processor.SetScissor_(_logicalRenderPass, Default);
        }
        else {
            RHI_LOG(Warning, L"failed to create mesh pipeline instance for <{0}>", _gPipelineRef->DebugName());
            success = false;
        }
    }

    Assert_NoAssume(not success or _pipelineLayoutRef);
    return success;
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::BindResources(const FDescriptorSetID& id, const FPipelineResources& res) {
    Assert(id);

    if (not BindPipeline_(ALL_BITS) )
        return;

    const FVulkanPipelineResources* const pPipelineResources = _processor._workerCmd->CreateDescriptorSet(res);
    Assert(pPipelineResources);

    const VkDescriptorSet vkDescriptorSet = pPipelineResources->Handle();
    const TMemoryView<const u32> dynamicOffsets = res.DynamicOffsets();

    u32 binding;
    FRawDescriptorSetLayoutID dsLayoutId;
    LOG_CHECKVOID(RHI, _pipelineLayoutRef->DescriptorSetLayout(&binding, &dsLayoutId, id) );

    _processor.vkCmdBindDescriptorSets(
        _processor._vkCommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayoutRef->Handle(),
        binding,
        1, &vkDescriptorSet,
        checked_cast<u32>(dynamicOffsets.size()), dynamicOffsets.data() );

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([](FFrameStatistics::FRendering& rendering) {
        rendering.NumDescriptorBinds++;
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::PushConstants(const FPushConstantID& id, const void* data, size_t size) {
    Assert(id);
    Assert(data);
    Assert(size);

    if (not BindPipeline_(ALL_BITS) )
        return;

    const auto pplnLayout = _pipelineLayoutRef->Read();
    const auto it = pplnLayout->PushConstants.find(id);
    LOG_CHECKVOID(RHI, pplnLayout->PushConstants.end() != it);
    Assert(static_cast<u32>(it->second.Size) == size);

    _processor.vkCmdPushConstants(
        _processor._vkCommandBuffer,
        pplnLayout->Layout,
        VkCast(it->second.StageFlags),
        it->second.Offset,
        checked_cast<u32>(size),
        data );

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([](FFrameStatistics::FRendering& rendering) {
        rendering.NumPushConstants++;
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::BindVertexAttribs(const FVertexInputState& vertexInput) {
    _vertexInput = vertexInput;
    _changed |= GRAPHICS_BIT;
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::BindVertexBuffer(const FVertexBufferID& id, FRawBufferID buffer, size_t offset) {
    Assert(id);
    Assert(buffer);

    const FVulkanBuffer* const pBuffer = _processor.Resource_(buffer);

    const auto it = _vertexInput.BufferBindings.find(id);

    if (pBuffer && _vertexInput.BufferBindings.end() != it) {
        const VkBuffer vkBuffer = pBuffer->Handle();
        const VkDeviceSize vkOffset = static_cast<VkDeviceSize>(offset);

        _processor.vkCmdBindVertexBuffers(
            _processor._vkCommandBuffer,
            it->second.Index,
            1, &vkBuffer, &vkOffset );

        ONLY_IF_RHIDEBUG(_processor.EditStatistics_([](FFrameStatistics::FRendering& rendering) {
            rendering.NumVertexBufferBindings++;
        }));
    }
    else {
        RHI_LOG(Warning, L"invalid vertex buffer binding: [{0}] -> {1} ({2})", id, buffer, _pipelineLayoutRef->DebugName());
    }
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::BindIndexBuffer(FRawBufferID buffer, size_t offset, EIndexFormat fmt) {
    Assert(buffer);

    if (const FVulkanBuffer* const pBuffer = _processor.Resource_(buffer)) {
        _processor.BindIndexBuffer_(pBuffer->Handle(), checked_cast<VkDeviceSize>(offset), VkCast(fmt));
    }
    else {
        RHI_LOG(Warning, L"invalid index buffer binding: {0} ({1})", buffer, _pipelineLayoutRef->DebugName());
    }
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::SetColorBuffer(ERenderTargetID id, const FColorBufferState& value) {
    Assert(static_cast<size_t>(id) < _renderState.Blend.Buffers.size());

    _renderState.Blend.Buffers[static_cast<u32>(id)] = value;
    _changed |= ALL_BITS;
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::SetLogicOp(ELogicOp value) {
    _renderState.Blend.LogicOp = value;
    _changed |= ALL_BITS;
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::SetBlendColor(const FLinearColor& value) {
    _renderState.Blend.BlendColor = value;
    _changed |= ALL_BITS;
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::SetStencilBuffer(const FStencilBufferState& value) {
    _renderState.Stencil = value;
    _changed |= ALL_BITS;
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::SetDepthBuffer(const FDepthBufferState& value) {
    _renderState.Depth = value;
    _changed |= ALL_BITS;
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::SetInputAssembly(const FInputAssemblyState& value) {
    _renderState.InputAssembly = value;
    _changed |= GRAPHICS_BIT;
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::SetRasterization(const FRasterizationState& value) {
    _renderState.Rasterization = value;
    _changed |= ALL_BITS;
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::SetMultisample(const FMultisampleState& value) {
    _renderState.Multisample = value;
    _changed |= ALL_BITS;
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::SetStencilCompareMask(u32 value) {
    LOG_CHECKVOID(RHI, BindPipeline_(ALL_BITS));

    _processor.vkCmdSetStencilCompareMask(
        _processor._vkCommandBuffer,
        VK_STENCIL_FRONT_AND_BACK,
        value );
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::SetStencilWriteMask(u32 value) {
    LOG_CHECKVOID(RHI, BindPipeline_(ALL_BITS));

    _processor.vkCmdSetStencilWriteMask(
        _processor._vkCommandBuffer,
        VK_STENCIL_FRONT_AND_BACK,
        value );
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::SetStencilReference(u32 value) {
    LOG_CHECKVOID(RHI, BindPipeline_(ALL_BITS));

    _processor.vkCmdSetStencilReference(
        _processor._vkCommandBuffer,
        VK_STENCIL_FRONT_AND_BACK,
        value );
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::SetShadingRatePalette(u32 viewportIndex, TMemoryView<const EShadingRatePalette> value) {
    LOG_CHECKVOID(RHI, BindPipeline_(ALL_BITS));

    STACKLOCAL_POD_ARRAY(VkShadingRatePaletteEntryNV, entries, Max(value.size(), 1_size_t));
    entries[0] = VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_PIXEL_NV;

    forrange(i, 0, value.size())
        entries[i] = VkCast(value[i]);

    VkShadingRatePaletteNV palette{};
    palette.shadingRatePaletteEntryCount = checked_cast<u32>(entries.size());
    palette.pShadingRatePaletteEntries = entries.data();

    _processor.vkCmdSetViewportShadingRatePaletteNV(
        _processor._vkCommandBuffer,
        viewportIndex, 1, &palette );
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::BindShadingRateImage(FRawImageID value, FImageLayer layer, FMipmapLevel level) {
    Assert(value);

    const FVulkanImage* const pImage = _processor.Resource_(value);
    LOG_CHECKVOID(RHI, pImage);

    FImageViewDesc desc;
    desc.View = EImageView_2D;
    desc.Format = EPixelFormat::R8u;
    desc.BaseLevel = level;
    desc.BaseLayer = layer;
    desc.AspectMask = EImageAspect::Color;

    const VkImageView vkImageView = pImage->MakeView(
        _processor._workerCmd->Device(), desc );
    Assert(VK_NULL_HANDLE != vkImageView);

    _processor.BindShadingRateImage_(vkImageView);
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::DrawVertices(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) {
    Assert_NoAssume(_gPipelineRef);

    LOG_CHECKVOID(RHI, BindPipeline_(GRAPHICS_BIT));

    _processor.vkCmdDraw(
        _processor._vkCommandBuffer,
        vertexCount, instanceCount, firstVertex, firstInstance );

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([=](FFrameStatistics::FRendering& rendering) {
        rendering.NumDrawCalls++;
        rendering.NumVertexCount += static_cast<u64>(vertexCount) * instanceCount;
        rendering.NumPrimitiveCount += EPrimitiveTopology_PrimitiveCount(
            _renderState.InputAssembly.Topology,
            vertexCount * instanceCount,
            _gPipelineRef->Read()->PatchControlPoints );
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance) {
    Assert_NoAssume(_gPipelineRef);

    LOG_CHECKVOID(RHI, BindPipeline_(GRAPHICS_BIT));

    _processor.vkCmdDrawIndexed(
        _processor._vkCommandBuffer,
        indexCount, instanceCount, firstIndex, vertexOffset, firstInstance );

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([=](FFrameStatistics::FRendering& rendering) {
        rendering.NumDrawCalls++;
        rendering.NumVertexCount += static_cast<u64>(indexCount) * instanceCount;
        rendering.NumPrimitiveCount += EPrimitiveTopology_PrimitiveCount(
            _renderState.InputAssembly.Topology,
            indexCount * instanceCount,
            _gPipelineRef->Read()->PatchControlPoints );
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::DrawVerticesIndirect(FRawBufferID indirectBuffer, size_t indirectBufferOffset, u32 drawCount, u32 indirectBufferStride) {
    Assert(indirectBuffer);
    Assert_NoAssume(_gPipelineRef);
    Assert_NoAssume(drawCount <= _processor._maxDrawIndirectCount);

    const FVulkanBuffer* const pIndirectBuffer = _processor.Resource_(indirectBuffer);
    LOG_CHECKVOID(RHI, pIndirectBuffer);

    LOG_CHECKVOID(RHI, BindPipeline_(GRAPHICS_BIT));

    _processor.vkCmdDrawIndirect(
        _processor._vkCommandBuffer,
        pIndirectBuffer->Handle(),
        checked_cast<VkDeviceSize>(indirectBufferOffset),
        drawCount, indirectBufferStride );

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([=](FFrameStatistics::FRendering& rendering) {
        rendering.NumIndirectDrawCalls += drawCount;
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::DrawIndexedIndirect(FRawBufferID indirectBuffer, size_t indirectBufferOffset, u32 drawCount, u32 indirectBufferStride) {
    Assert(indirectBuffer);
    Assert_NoAssume(_gPipelineRef);
    Assert_NoAssume(drawCount <= _processor._maxDrawIndirectCount);

    const FVulkanBuffer* const pIndirectBuffer = _processor.Resource_(indirectBuffer);
    LOG_CHECKVOID(RHI, pIndirectBuffer);

    LOG_CHECKVOID(RHI, BindPipeline_(GRAPHICS_BIT));

    _processor.vkCmdDrawIndexedIndirect(
        _processor._vkCommandBuffer,
        pIndirectBuffer->Handle(),
        checked_cast<VkDeviceSize>(indirectBufferOffset),
        drawCount, indirectBufferStride );

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([=](FFrameStatistics::FRendering& rendering) {
        rendering.NumIndirectDrawCalls += drawCount;
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::DrawVerticesIndirectCount(FRawBufferID indirectBuffer, size_t indirectBufferOffset, FRawBufferID countBuffer, size_t countBufferOffset, u32 maxDrawCount, u32 indirectBufferStride) {
    Assert(indirectBuffer);
    Assert(countBuffer);
    Assert_NoAssume(_gPipelineRef);
    Assert_NoAssume(_processor._enableDrawIndirectCount);
    Assert_NoAssume(maxDrawCount <= _processor._maxDrawIndirectCount);

    const FVulkanBuffer* const pIndirectBuffer = _processor.Resource_(indirectBuffer);
    LOG_CHECKVOID(RHI, pIndirectBuffer);

    const FVulkanBuffer* const pCountBuffer = _processor.Resource_(countBuffer);
    LOG_CHECKVOID(RHI, pCountBuffer);

    LOG_CHECKVOID(RHI, BindPipeline_(GRAPHICS_BIT));

    _processor.vkCmdDrawIndirectCountKHR(
        _processor._vkCommandBuffer,
        pIndirectBuffer->Handle(),
        checked_cast<VkDeviceSize>(indirectBufferOffset),
        pCountBuffer->Handle(),
        checked_cast<VkDeviceSize>(countBufferOffset),
        maxDrawCount, indirectBufferStride );

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([=](FFrameStatistics::FRendering& rendering) {
        rendering.NumIndirectDrawCalls += maxDrawCount;
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::DrawIndexedIndirectCount(FRawBufferID indirectBuffer, size_t indirectBufferOffset, FRawBufferID countBuffer, size_t countBufferOffset, u32 maxDrawCount, u32 indirectBufferStride) {
    Assert(indirectBuffer);
    Assert(countBuffer);
    Assert_NoAssume(_gPipelineRef);
    Assert_NoAssume(_processor._enableDrawIndirectCount);
    Assert_NoAssume(maxDrawCount <= _processor._maxDrawIndirectCount);

    const FVulkanBuffer* const pIndirectBuffer = _processor.Resource_(indirectBuffer);
    LOG_CHECKVOID(RHI, pIndirectBuffer);

    const FVulkanBuffer* const pCountBuffer = _processor.Resource_(countBuffer);
    LOG_CHECKVOID(RHI, pCountBuffer);

    LOG_CHECKVOID(RHI, BindPipeline_(GRAPHICS_BIT));

    _processor.vkCmdDrawIndirectCountKHR(
        _processor._vkCommandBuffer,
        pIndirectBuffer->Handle(),
        checked_cast<VkDeviceSize>(indirectBufferOffset),
        pCountBuffer->Handle(),
        checked_cast<VkDeviceSize>(countBufferOffset),
        maxDrawCount, indirectBufferStride );

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([=](FFrameStatistics::FRendering& rendering) {
        rendering.NumIndirectDrawCalls += maxDrawCount;
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::DrawMeshes(u32 taskCount, u32 firstTask) {
    Assert_NoAssume(_mPipelineRef);
    Assert_NoAssume(_processor._enableMeshShaderNV);
    Assert_NoAssume(taskCount <= _processor._maxDrawMeshTaskCount);

    LOG_CHECKVOID(RHI, BindPipeline_(MESH_BIT));

    _processor.vkCmdDrawMeshTasksNV(
        _processor._vkCommandBuffer,
        taskCount, firstTask );

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([=](FFrameStatistics::FRendering& rendering) {
        rendering.NumDrawCalls++;
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::DrawMeshesIndirect(FRawBufferID indirectBuffer, size_t indirectBufferOffset, u32 drawCount, u32 indirectBufferStride) {
    Assert(indirectBuffer);
    Assert_NoAssume(_mPipelineRef);
    Assert_NoAssume(_processor._enableMeshShaderNV);
    Assert_NoAssume(drawCount <= _processor._maxDrawIndirectCount);

    const FVulkanBuffer* const pIndirectBuffer = _processor.Resource_(indirectBuffer);
    LOG_CHECKVOID(RHI, pIndirectBuffer);

    LOG_CHECKVOID(RHI, BindPipeline_(MESH_BIT));

    _processor.vkCmdDrawMeshTasksIndirectNV(
        _processor._vkCommandBuffer,
        pIndirectBuffer->Handle(),
        checked_cast<VkDeviceSize>(indirectBufferOffset),
        drawCount,
        indirectBufferStride );

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([=](FFrameStatistics::FRendering& rendering) {
        rendering.NumIndirectDrawCalls += drawCount;
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawContext::DrawMeshesIndirectCount(FRawBufferID indirectBuffer, size_t indirectBufferOffset, FRawBufferID countBuffer, size_t countBufferOffset, u32 maxDrawCount, u32 indirectBufferStride) {
    Assert(indirectBuffer);
    Assert(countBuffer);
    Assert_NoAssume(_mPipelineRef);
    Assert_NoAssume(_processor._enableMeshShaderNV);
    Assert_NoAssume(maxDrawCount <= _processor._maxDrawIndirectCount);

    const FVulkanBuffer* const pIndirectBuffer = _processor.Resource_(indirectBuffer);
    LOG_CHECKVOID(RHI, pIndirectBuffer);

    const FVulkanBuffer* const pCountBuffer = _processor.Resource_(countBuffer);
    LOG_CHECKVOID(RHI, pCountBuffer);

    LOG_CHECKVOID(RHI, BindPipeline_(MESH_BIT));

    _processor.vkCmdDrawMeshTasksIndirectCountNV(
        _processor._vkCommandBuffer,
        pIndirectBuffer->Handle(),
        checked_cast<VkDeviceSize>(indirectBufferOffset),
        pCountBuffer->Handle(),
        checked_cast<VkDeviceSize>(countBufferOffset),
        maxDrawCount,
        indirectBufferStride );

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([=](FFrameStatistics::FRendering& rendering) {
        rendering.NumIndirectDrawCalls += maxDrawCount;
    }));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
