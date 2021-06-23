
#include "stdafx.h"

#include "Vulkan/Command/VulkanDrawTask.h"

#include "Vulkan/Command/VulkanCommandBuffer.h"
#include "Vulkan/Command/VulkanFrameTask.h"

namespace PPE {
namespace RHI {
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
:   IVulkanDrawTask(desc, pass1, pass2)
,   Pipeline(cmd.AcquireTransient(desc.Pipeline))
,   PushConstants(desc.PushConstants)
,   VertexInput(desc.VertexInput)
,   ColorBuffers(desc.ColorBuffers)
,   DynamicStates(desc.DynamicStates)
,   Topology(desc.Topology)
,   EnablePrimitiveRestart(desc.EnablePrimitiveRestart)
,   _vertexBufferCount(checked_cast<u32>(desc.VertexBuffers.size()))
,   _scissors(cmd.Allocator().AllocateCopyT(desc.Scissors.MakeView())) {

    IVulkanFrameTask::CopyDescriptorSets(&_resources, &renderPass, cmd, desc.Resources);

    for (const TPair<const FVertexBufferID, FVertexBuffer>& vb : desc.VertexBuffers) {
        const auto it = VertexInput.BufferBindings.find(vb.first);
        Assert(VertexInput.BufferBindings.end() != it);

        FVulkanLocalBuffer* const pLocalBuffer = cmd.ToLocal(vb.second.Id);
        Assert(pLocalBuffer);
        Assert_NoAssume(pLocalBuffer->Read()->Desc.Usage & EBufferUsage::Vertex);

        _vertexBuffers[it->second.Index] = pLocalBuffer;
        _vertexOffsets[it->second.Index] = static_cast<VkDeviceSize>(vb.second.Offset);
        _vertexStrides[it->second.Index] = it->second.Stride;
    }

#if USE_PPE_RHIDEBUG
    if (desc.DebugMode.Mode != Default)
        SetDebugModeIndex(cmd.Batch()->AppendShaderForDebug(_scissors, desc.Name, desc.DebugMode));
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
:   IVulkanDrawTask(desc, pass1, pass2)
,   Pipeline(cmd.AcquireTransient(desc.Pipeline))
,   PushConstants(desc.PushConstants)
,   ColorBuffers(desc.ColorBuffers)
,   DynamicStates(desc.DynamicStates)
,   _scissors(cmd.Allocator().AllocateCopyT(desc.Scissors.MakeView())) {

    IVulkanFrameTask::CopyDescriptorSets(&_resources, &renderPass, cmd, desc.Resources);

#if USE_PPE_RHIDEBUG
    if (desc.DebugMode.Mode != Default)
        SetDebugModeIndex(cmd.Batch()->AppendShaderForDebug(_scissors, desc.Name, desc.DebugMode));
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
,   Commands(desc.Commands) {

}
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawIndexed>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawIndexed& desc,
    FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
:   FVulkanBaseDrawVerticesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(desc.Commands)
,   IndexBuffer(cmd.ToLocal(desc.IndexBuffer))
,   IndexBufferOffset(desc.IndexBufferOffset)
,   IndexFormat(desc.IndexFormat) {
    Assert_NoAssume(IndexBuffer);
    Assert_NoAssume(IndexBuffer->Read()->Desc.Usage & EBufferUsage::Index);
}
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawVerticesIndirect>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawVerticesIndirect& desc,
    FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
:   FVulkanBaseDrawVerticesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(desc.Commands)
,   IndirectBuffer(cmd.ToLocal(desc.IndirectBuffer)) {
    Assert_NoAssume(IndirectBuffer);
    Assert_NoAssume(IndirectBuffer->Read()->Desc.Usage & EBufferUsage::Indirect);
}
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawIndexedIndirect>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawIndexedIndirect& desc,
    FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
:   FVulkanBaseDrawVerticesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(desc.Commands)
,   IndirectBuffer(cmd.ToLocal(desc.IndirectBuffer))
,   IndexBuffer(cmd.ToLocal(desc.IndexBuffer))
,   IndexBufferOffset(desc.IndexBufferOffset)
,   IndexFormat(desc.IndexFormat) {
    Assert_NoAssume(IndirectBuffer);
    Assert_NoAssume(IndirectBuffer->Read()->Desc.Usage & EBufferUsage::Indirect);
    Assert_NoAssume(IndexBuffer);
    Assert_NoAssume(IndexBuffer->Read()->Desc.Usage & EBufferUsage::Index);
}
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawVerticesIndirectCount>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawVerticesIndirectCount& desc,
    FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
:   FVulkanBaseDrawVerticesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(desc.Commands)
,   IndirectBuffer(cmd.ToLocal(desc.IndirectBuffer))
,   CountBuffer(cmd.ToLocal(desc.CountBuffer)) {
    Assert_NoAssume(IndirectBuffer);
    Assert_NoAssume(IndirectBuffer->Read()->Desc.Usage & EBufferUsage::Indirect);
    Assert_NoAssume(CountBuffer);
    Assert_NoAssume(CountBuffer->Read()->Desc.Usage & EBufferUsage::Index);
}
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawIndexedIndirectCount>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawIndexedIndirectCount& desc,
    FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
:   FVulkanBaseDrawVerticesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(desc.Commands)
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
,   Commands(desc.Commands) {

}
//----------------------------------------------------------------------------
TVulkanDrawTask<FDrawMeshesIndirect>::TVulkanDrawTask(
    FVulkanLogicalRenderPass& renderPass,
    FVulkanCommandBuffer& cmd,
    const FDrawMeshesIndirect& desc,
    FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT
:   FVulkanBaseDrawMeshesTask(renderPass, cmd, desc, pass1, pass2)
,   Commands(desc.Commands)
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
,   Commands(desc.Commands)
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
:   IVulkanDrawTask(desc, pass1, pass2)
,   Callback(desc.Callback)
,   UserParam(desc.UserParam)
,   Images(cmd.Allocator().AllocateCopyT(desc.Images.MakeView(), [&cmd](auto src) {
        return TPair<const FVulkanLocalImage*, EResourceState>(cmd.ToLocal(src.first), src.second);
    }))
,   Buffers(cmd.Allocator().AllocateCopyT(desc.Buffers.MakeView(), [&cmd](auto src) {
        return TPair<const FVulkanLocalBuffer*, EResourceState>(cmd.ToLocal(src.first), src.second);
    })) {

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
