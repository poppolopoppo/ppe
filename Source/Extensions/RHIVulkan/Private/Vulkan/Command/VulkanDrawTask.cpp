// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Command/VulkanDrawTask.h"

#include "Vulkan/Command/VulkanCommandBuffer.h"
#include "Vulkan/Command/VulkanFrameTask.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Task>
IVulkanDrawTask::IVulkanDrawTask(
    FVulkanCommandBuffer& cmd,
    const _Task& desc, FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
:   Pass1(pass1), Pass2(pass2)
ARGS_IF_RHIDEBUG(TaskName(cmd.EmbedString(desc.Name.Str())), DebugColor(desc.DebugColor.Quantize(EGammaSpace::sRGB))) {
    Unused(cmd, desc);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _Task>
FVulkanBaseDrawVerticesTask::FVulkanBaseDrawVerticesTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const _Task& desc,
    FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT
:   IVulkanDrawTask(cmd, desc, pass1, pass2)
,   Pipeline(cmd.AcquireTransient(desc.Pipeline))

,   DynamicStates(desc.DynamicStates)
,   Topology(desc.Topology)
,   EnablePrimitiveRestart(desc.EnablePrimitiveRestart)

,   ColorBuffers(cmd.EmbedCopy(desc.ColorBuffers))
,   PushConstants(cmd.EmbedCopy(desc.PushConstants.MakeView()))
,   Scissors(cmd.EmbedCopy(desc.Scissors.MakeView()))

,   BufferBindings(cmd.EmbedCopy(desc.VertexInput.BufferBindings.MakeView()))
,   VertexInputs(cmd.EmbedCopy(desc.VertexInput.Vertices))

,   VertexBuffers(cmd.EmbedView<TPtrRef<const FVulkanLocalBuffer>>(desc.VertexBuffers.size()))
,   VertexOffsets(cmd.EmbedView<VkDeviceSize>(desc.VertexBuffers.size()))
,   VertexStrides(cmd.EmbedView<u32>(desc.VertexBuffers.size()))
{
    FVulkanPipelineResourceSet resources;
    IVulkanFrameTask::CopyDescriptorSets(&resources, &renderPass, cmd, desc.Resources);

    DynamicOffsets = cmd.EmbedCopy(resources.DynamicOffsets.MakeView());
    Resources = cmd.EmbedCopy(resources.Resources.MakeView());

    const auto mutableVertexBuffers = RemoveConstView(VertexBuffers);
    const auto mutableVertexOffsets = RemoveConstView(VertexOffsets);
    const auto mutableVertexStrides = RemoveConstView(VertexStrides);

    Broadcast(mutableVertexBuffers, TPtrRef<const FVulkanLocalBuffer>{});
    Broadcast(mutableVertexOffsets, 0);
    Broadcast(mutableVertexStrides, 0);

    for (const TPair<const FVertexBufferID, FVertexBuffer>& vb : desc.VertexBuffers) {
        const auto it = std::find_if(BufferBindings.begin(), BufferBindings.end(),
            [vertexBufferId{vb.first}](const FBufferBinding& it){
               return (it.first == vertexBufferId);
            });
        AssertRelease(BufferBindings.end() != it);

        FVulkanLocalBuffer* const pLocalBuffer = cmd.ToLocal(vb.second.Id);
        Assert(pLocalBuffer);
        Assert_NoAssume(pLocalBuffer->Read()->Desc.Usage & EBufferUsage::Vertex);

        mutableVertexBuffers[it->second.Index] = pLocalBuffer;
        mutableVertexOffsets[it->second.Index] = checked_cast<VkDeviceSize>(vb.second.Offset);
        mutableVertexStrides[it->second.Index] = it->second.Stride;
    }

#if USE_PPE_RHIDEBUG
    if (desc.DebugMode.Mode != Default)
        DebugModeIndex = cmd.Batch()->AppendShaderForDebug(Scissors, desc.Name, desc.DebugMode);
#endif
}
//----------------------------------------------------------------------------
#ifdef VK_NV_mesh_shader
template <typename _Task>
FVulkanBaseDrawMeshesTask::FVulkanBaseDrawMeshesTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const _Task& desc,
    FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
:   IVulkanDrawTask(cmd, desc, pass1, pass2)
,   Pipeline(cmd.AcquireTransient(desc.Pipeline))
,   DynamicStates(desc.DynamicStates)
,   ColorBuffers(cmd.EmbedCopy(desc.ColorBuffers))
,   PushConstants(cmd.EmbedCopy(desc.PushConstants.MakeView()))
,   Scissors(cmd.EmbedCopy(desc.Scissors.MakeView()))
 {
    FVulkanPipelineResourceSet resources;
    IVulkanFrameTask::CopyDescriptorSets(&resources, &renderPass, cmd, desc.Resources);

    DynamicOffsets = cmd.EmbedCopy(resources.DynamicOffsets.MakeView());
    Resources = cmd.EmbedCopy(resources.Resources.MakeView());

#if USE_PPE_RHIDEBUG
    if (desc.DebugMode.Mode != Default)
        DebugModeIndex = cmd.Batch()->AppendShaderForDebug(Scissors, desc.Name, desc.DebugMode);
#endif
}
#endif
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Draw vertices tasks
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawVertices>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawVertices& desc,
    FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
:   FVulkanBaseDrawVerticesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(cmd.EmbedCopy(desc.Commands.MakeView())) {

}
TVulkanDrawTask<FDrawVertices>::~TVulkanDrawTask() = default;
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawIndexed>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawIndexed& desc,
    FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
:   FVulkanBaseDrawVerticesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(cmd.EmbedCopy(desc.Commands.MakeView()))
,   IndexBuffer(cmd.ToLocal(desc.IndexBuffer))
,   IndexBufferOffset(desc.IndexBufferOffset)
,   IndexFormat(desc.IndexFormat) {
    Assert_NoAssume(IndexBuffer);
    Assert_NoAssume(IndexBuffer->Read()->Desc.Usage & EBufferUsage::Index);
}
TVulkanDrawTask<FDrawIndexed>::~TVulkanDrawTask() = default;
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawVerticesIndirect>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawVerticesIndirect& desc,
    FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
:   FVulkanBaseDrawVerticesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(cmd.EmbedCopy(desc.Commands.MakeView()))
,   IndirectBuffer(cmd.ToLocal(desc.IndirectBuffer)) {
    Assert_NoAssume(IndirectBuffer);
    Assert_NoAssume(IndirectBuffer->Read()->Desc.Usage & EBufferUsage::Indirect);
}
TVulkanDrawTask<FDrawVerticesIndirect>::~TVulkanDrawTask() = default;
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawIndexedIndirect>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawIndexedIndirect& desc,
    FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
:   FVulkanBaseDrawVerticesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(cmd.EmbedCopy(desc.Commands.MakeView()))
,   IndirectBuffer(cmd.ToLocal(desc.IndirectBuffer))
,   IndexBuffer(cmd.ToLocal(desc.IndexBuffer))
,   IndexBufferOffset(desc.IndexBufferOffset)
,   IndexFormat(desc.IndexFormat) {
    Assert_NoAssume(IndirectBuffer);
    Assert_NoAssume(IndirectBuffer->Read()->Desc.Usage & EBufferUsage::Indirect);
    Assert_NoAssume(IndexBuffer);
    Assert_NoAssume(IndexBuffer->Read()->Desc.Usage & EBufferUsage::Index);
}
TVulkanDrawTask<FDrawIndexedIndirect>::~TVulkanDrawTask() = default;
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawVerticesIndirectCount>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawVerticesIndirectCount& desc,
    FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
:   FVulkanBaseDrawVerticesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(cmd.EmbedCopy(desc.Commands.MakeView()))
,   IndirectBuffer(cmd.ToLocal(desc.IndirectBuffer))
,   CountBuffer(cmd.ToLocal(desc.CountBuffer)) {
    Assert_NoAssume(IndirectBuffer);
    Assert_NoAssume(IndirectBuffer->Read()->Desc.Usage & EBufferUsage::Indirect);
    Assert_NoAssume(CountBuffer);
    Assert_NoAssume(CountBuffer->Read()->Desc.Usage & EBufferUsage::Index);
}
TVulkanDrawTask<FDrawVerticesIndirectCount>::~TVulkanDrawTask() = default;
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawIndexedIndirectCount>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawIndexedIndirectCount& desc,
    FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
:   FVulkanBaseDrawVerticesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(cmd.EmbedCopy(desc.Commands.MakeView()))
,   IndirectBuffer(cmd.ToLocal(desc.IndirectBuffer))
,   CountBuffer(cmd.ToLocal(desc.CountBuffer))
,   IndexBuffer(cmd.ToLocal(desc.IndexBuffer))
,   IndexBufferOffset(desc.IndexBufferOffset)
,   IndexFormat(desc.IndexFormat) {
    Assert_NoAssume(IndirectBuffer);
    Assert_NoAssume(IndirectBuffer->Read()->Desc.Usage & EBufferUsage::Indirect);
    Assert_NoAssume(CountBuffer);
    Assert_NoAssume(CountBuffer->Read()->Desc.Usage & EBufferUsage::Index);
    Assert_NoAssume(IndexBuffer);
    Assert_NoAssume(IndexBuffer->Read()->Desc.Usage & EBufferUsage::Index);
}
TVulkanDrawTask<FDrawIndexedIndirectCount>::~TVulkanDrawTask() = default;
//----------------------------------------------------------------------------
// Draw meshes tasks
//----------------------------------------------------------------------------
#ifdef VK_NV_mesh_shader
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawMeshes>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawMeshes& desc,
    FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT
:   FVulkanBaseDrawMeshesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(cmd.EmbedCopy(desc.Commands.MakeView())) {

}
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawMeshesIndirect>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawMeshesIndirect& desc,
    FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT
:   FVulkanBaseDrawMeshesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(cmd.EmbedCopy(desc.Commands.MakeView()))
,   IndirectBuffer(cmd.ToLocal(desc.IndirectBuffer)) {
    Assert_NoAssume(IndirectBuffer);
    Assert_NoAssume(IndirectBuffer->Read()->Desc.Usage & EBufferUsage::Indirect);
}
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawMeshesIndirectCount>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawMeshesIndirectCount& desc,
    FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT
:   FVulkanBaseDrawMeshesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(cmd.EmbedCopy(desc.Commands.MakeView()))
,   IndirectBuffer(cmd.ToLocal(desc.IndirectBuffer))
,   CountBuffer(cmd.ToLocal(desc.CountBuffer)) {
    Assert_NoAssume(IndirectBuffer);
    Assert_NoAssume(IndirectBuffer->Read()->Desc.Usage & EBufferUsage::Indirect);
    Assert_NoAssume(CountBuffer);
    Assert_NoAssume(CountBuffer->Read()->Desc.Usage & EBufferUsage::Index);
}
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
// Custom
//----------------------------------------------------------------------------
TVulkanDrawTask<FCustomDraw>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& ,
    FVulkanCommandBuffer& cmd,
    const FCustomDraw& desc,
    FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT
:   IVulkanDrawTask(cmd, desc, pass1, pass2)
,   Callback(desc.Callback)
,   UserParam(desc.UserParam)
,   Images(cmd.EmbedCopy(desc.Images.MakeView().Map([&cmd](auto src) {
        return FImage(cmd.ToLocal(src.first), src.second);
    })))
,   Buffers(cmd.EmbedCopy(desc.Buffers.MakeView().Map([&cmd](auto src) {
        return FBuffer(cmd.ToLocal(src.first), src.second);
    })))
{}
TVulkanDrawTask<FCustomDraw>::~TVulkanDrawTask() = default;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
