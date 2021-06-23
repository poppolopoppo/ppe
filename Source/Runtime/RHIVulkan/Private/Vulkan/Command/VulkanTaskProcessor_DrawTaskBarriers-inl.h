#pragma once

#include "Vulkan/Command/VulkanTaskProcessor.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FVulkanTaskProcessor::FDrawTaskBarriers::FDrawTaskBarriers(
    FVulkanTaskProcessor& processor,
    const FVulkanLogicalRenderPass& logicalRenderPass ) NOEXCEPT
:   _processor(processor)
,   _logicalRenderPass(logicalRenderPass)
,   _enableEarlyFragmentTests(false)
,   _enableLateFragmentTests(false)
,   _enableDepthWrite(logicalRenderPass.DepthState().EnableDepthWrite)
,   _enableStencilWrite(false)
,   _enableRasterizerDiscard(logicalRenderPass.RasterizationState().EnableDiscard)
,   _enableCompatibleFragmentOutput(true) {

    // invalidate fragment output

    for (FPipelineDesc::FFragmentOutput& frag : _fragmentOutputs) {
        frag.Index = UMax;
        frag.Type = EFragmentOutput::Unknown;
    }

    const FStencilBufferState& stencil = logicalRenderPass.StencilState();
    _enableStencilWrite |= (stencil.EnabledStencilTests
        ? ( (EStencilOp::Keep != stencil.Front.PassOp) |
            (EStencilOp::Keep != stencil.Front.FailOp) |
            (EStencilOp::Keep != stencil.Front.DepthFailOp) |
            (EStencilOp::Keep != stencil.Back.PassOp) |
            (EStencilOp::Keep != stencil.Back.FailOp) |
            (EStencilOp::Keep != stencil.Back.DepthFailOp) )
        : false );
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskBarriers::Visit(const TVulkanDrawTask<FDrawVertices>& task) {
    const auto ppln = task.Pipeline->Read();

    // update descriptor sets and add pipeline barriers
    ExtractDescriptorSets(*ppln->BaseLayoutId, task);

    // add vertex buffers

    forrange(i, 0, task.VertexBuffers().size()) {
        const VkDeviceSize vbOffset{ task.VertexOffsets()[i] };
        const VkDeviceSize vbStride{ task.VertexStrides()[i] };

        for (const auto& cmd : task.Commands) {
            const VkDeviceSize cmdOffset{ vbOffset + vbStride * cmd.FirstVertex };
            const VkDeviceSize cmdSize{ vbStride * cmd.VertexCount };

            _processor.AddBuffer_(
                task.VertexBuffers()[i],
                EResourceState::VertexBuffer,
                cmdOffset, cmdSize );
        }
    }

    MergePipeline(task.DynamicStates, *task.Pipeline->Read());
}

//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskBarriers::Visit(const TVulkanDrawTask<FDrawIndexed>& task) {
    const auto ppln = task.Pipeline->Read();

    // update descriptor sets and add pipeline barriers
    ExtractDescriptorSets(*ppln->BaseLayoutId, task);

    // add vertex buffers

    forrange(i, 0, task.VertexBuffers().size()) {
        _processor.AddBuffer_(
            task.VertexBuffers()[i],
            EResourceState::VertexBuffer,
            task.VertexOffsets()[i],
            VK_WHOLE_SIZE );
    }

    // add index buffer

    const VkDeviceSize ibStride{ EIndexFormat_SizeOf(task.IndexFormat) };
    const VkDeviceSize ibOffset{ task.IndexBufferOffset };

    for (const auto& cmd : task.Commands) {
        const VkDeviceSize cmdSize{ ibStride * cmd.IndexCount };

        _processor.AddBuffer_(
            task.IndexBuffer,
            EResourceState::IndexBuffer,
            ibOffset, cmdSize );
    }

    MergePipeline(task.DynamicStates, *task.Pipeline->Read());
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskBarriers::Visit(const TVulkanDrawTask<FDrawVerticesIndirect>& task) {
    const auto ppln = task.Pipeline->Read();

    // update descriptor sets and add pipeline barriers
    ExtractDescriptorSets(*ppln->BaseLayoutId, task);

    // add vertex buffers

    forrange(i, 0, task.VertexBuffers().size()) {
        _processor.AddBuffer_(
            task.VertexBuffers()[i],
            EResourceState::VertexBuffer,
            task.VertexOffsets()[i],
            VK_WHOLE_SIZE );
    }

    // add indirect buffer

    for (const auto& cmd : task.Commands) {
        _processor.AddBuffer_(
            task.IndirectBuffer,
            EResourceState::IndirectBuffer,
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset),
            static_cast<VkDeviceSize>(cmd.IndirectBufferStride) * cmd.DrawCount );
    }

    MergePipeline(task.DynamicStates, *task.Pipeline->Read());
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskBarriers::Visit(const TVulkanDrawTask<FDrawIndexedIndirect>& task) {
    const auto ppln = task.Pipeline->Read();

    // update descriptor sets and add pipeline barriers
    ExtractDescriptorSets(*ppln->BaseLayoutId, task);

    // add vertex buffers

    forrange(i, 0, task.VertexBuffers().size()) {
        _processor.AddBuffer_(
            task.VertexBuffers()[i],
            EResourceState::VertexBuffer,
            task.VertexOffsets()[i],
            VK_WHOLE_SIZE );
    }

    // add index buffer

    _processor.AddBuffer_(
        task.IndexBuffer,
        EResourceState::IndexBuffer,
        static_cast<VkDeviceSize>(task.IndexBufferOffset),
        VK_WHOLE_SIZE );

    // add indirect buffer

    for (const auto& cmd : task.Commands) {
        _processor.AddBuffer_(
            task.IndirectBuffer,
            EResourceState::IndirectBuffer,
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset),
            static_cast<VkDeviceSize>(cmd.IndirectBufferStride) * cmd.DrawCount );
    }

    MergePipeline(task.DynamicStates, *task.Pipeline->Read());
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskBarriers::Visit(const TVulkanDrawTask<FDrawVerticesIndirectCount>& task) {
    const auto ppln = task.Pipeline->Read();

    // update descriptor sets and add pipeline barriers
    ExtractDescriptorSets(*ppln->BaseLayoutId, task);

    // add vertex buffers

    forrange(i, 0, task.VertexBuffers().size()) {
        _processor.AddBuffer_(
            task.VertexBuffers()[i],
            EResourceState::VertexBuffer,
            task.VertexOffsets()[i],
            VK_WHOLE_SIZE );
    }

    // add indirect buffer

    for (const auto& cmd : task.Commands) {
        _processor.AddBuffer_(
            task.IndirectBuffer,
            EResourceState::IndirectBuffer,
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset),
            static_cast<VkDeviceSize>(cmd.IndirectBufferStride) * cmd.MaxDrawCount );
        _processor.AddBuffer_(
            task.CountBuffer,
            EResourceState::IndirectBuffer,
            static_cast<VkDeviceSize>(cmd.CountBufferOffset),
            sizeof(u32) );
    }

    MergePipeline(task.DynamicStates, *task.Pipeline->Read());
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskBarriers::Visit(const TVulkanDrawTask<FDrawIndexedIndirectCount>& task) {
    const auto ppln = task.Pipeline->Read();

    // update descriptor sets and add pipeline barriers
    ExtractDescriptorSets(*ppln->BaseLayoutId, task);

    // add vertex buffers

    forrange(i, 0, task.VertexBuffers().size()) {
        _processor.AddBuffer_(
            task.VertexBuffers()[i],
            EResourceState::VertexBuffer,
            task.VertexOffsets()[i],
            VK_WHOLE_SIZE );
    }

    // add index buffer

    _processor.AddBuffer_(
        task.IndexBuffer,
        EResourceState::IndexBuffer,
        static_cast<VkDeviceSize>(task.IndexBufferOffset),
        VK_WHOLE_SIZE );

    // add indirect buffer

    for (const auto& cmd : task.Commands) {
        _processor.AddBuffer_(
            task.IndirectBuffer,
            EResourceState::IndirectBuffer,
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset),
            static_cast<VkDeviceSize>(cmd.IndirectBufferStride) * cmd.MaxDrawCount );
        _processor.AddBuffer_(
            task.CountBuffer,
            EResourceState::IndirectBuffer,
            static_cast<VkDeviceSize>(cmd.CountBufferOffset),
            sizeof(u32) );
    }

    MergePipeline(task.DynamicStates, *task.Pipeline->Read());
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskBarriers::Visit(const TVulkanDrawTask<FDrawMeshes>& task) {
    const auto ppln = task.Pipeline->Read();

    // update descriptor sets and add pipeline barriers
    ExtractDescriptorSets(*ppln->BaseLayoutId, task);

    MergePipeline(task.DynamicStates, *task.Pipeline->Read());
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskBarriers::Visit(const TVulkanDrawTask<FDrawMeshesIndirect>& task) {
    const auto ppln = task.Pipeline->Read();

    // update descriptor sets and add pipeline barriers
    ExtractDescriptorSets(*ppln->BaseLayoutId, task);

    for (const auto& cmd : task.Commands) {
        _processor.AddBuffer_(
            task.IndirectBuffer,
            EResourceState::IndirectBuffer,
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset),
            static_cast<VkDeviceSize>(cmd.IndirectBufferStride) * cmd.DrawCount );
    }

    MergePipeline(task.DynamicStates, *task.Pipeline->Read());
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskBarriers::Visit(const TVulkanDrawTask<FDrawMeshesIndirectCount>& task) {
    const auto ppln = task.Pipeline->Read();

    // update descriptor sets and add pipeline barriers
    ExtractDescriptorSets(*ppln->BaseLayoutId, task);

    for (const auto& cmd : task.Commands) {
        _processor.AddBuffer_(
            task.IndirectBuffer,
            EResourceState::IndirectBuffer,
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset),
            static_cast<VkDeviceSize>(cmd.IndirectBufferStride) * cmd.MaxDrawCount );
        _processor.AddBuffer_(
            task.CountBuffer,
            EResourceState::IndirectBuffer,
            static_cast<VkDeviceSize>(cmd.CountBufferOffset),
            sizeof(u32) );
    }



    MergePipeline(task.DynamicStates, *task.Pipeline->Read());
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskBarriers::Visit(const TVulkanDrawTask<FCustomDraw>& task) {
    const EResourceState stages = _processor._workerCmd->Device().GraphicsShaderStages();

    for (const auto& it : task.Images) {
        const auto sharedImg = it.first->Read();
        const FImageViewDesc desc{ sharedImg->Desc };

        _processor.AddImage_(
            it.first, (it.second | stages),
            EResourceState_ToImageLayout(it.second, sharedImg->AspectMask),
            desc );
    }

    for (const auto& it : task.Buffers) {
        _processor.AddBuffer_(
            it.first, it.second,
            0, VK_WHOLE_SIZE );
    }
}
//----------------------------------------------------------------------------
template <typename _DrawTask>
void FVulkanTaskProcessor::FDrawTaskBarriers::ExtractDescriptorSets(const FRawPipelineLayoutID& layoutId, const _DrawTask& task) {
    Assert(layoutId);

    _processor.ExtractDescriptorSets_(
        &task.DescriptorSets,
        *_processor.Resource_(layoutId), task.Resources() );
}
//----------------------------------------------------------------------------
template <typename _Pipeline>
void FVulkanTaskProcessor::FDrawTaskBarriers::MergePipeline(const FDrawDynamicStates& ds, const _Pipeline& pipeline) NOEXCEPT {
    STATIC_ASSERT(
        std::is_same_v<_Pipeline, FVulkanGraphicsPipeline::FInternalPipeline> or
        std::is_same_v<_Pipeline, FVulkanMeshPipeline::FInternalPipeline> );


    if (pipeline.EarlyFragmentTests)
        _enableEarlyFragmentTests = true;
    else
        _enableLateFragmentTests = false;

    _enableDepthWrite |= (ds.HasEnableDepthWrite() & ds.EnableDepthWrite());

    _enableStencilWrite |= ((ds.HasEnableStencilTest() & _logicalRenderPass.StencilState().EnabledStencilTests)
        ?  ((ds.HasStencilPassOp() && (ds.StencilPassOp() != EStencilOp::Keep)) |
            (ds.HasStencilFailOp() && (ds.StencilFailOp() != EStencilOp::Keep)) |
            (ds.HasStencilDepthFailOp() && (ds.StencilDepthFailOp() != EStencilOp::Keep)) )
        : false );

    _enableRasterizerDiscard &= (not _logicalRenderPass.RasterizationState().EnableDiscard);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
