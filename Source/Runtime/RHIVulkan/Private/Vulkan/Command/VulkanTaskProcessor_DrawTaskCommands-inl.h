#pragma once

#include "Vulkan/Command/VulkanTaskProcessor.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T>
using if_has_PushConstants_ = decltype(std::declval<const T&>().PushConstants);
template <typename T>
using if_has_DynamicStates_ = decltype(std::declval<const T&>().DynamicStates);
template <typename T>
using if_has_Scissors_ = decltype(std::declval<const T&>().Scissors());
template <typename T>
using if_has_IndexBuffer_ = decltype(std::declval<const T&>().IndexBuffer);
template <typename T>
using if_has_VertexBuffers_ = decltype(std::declval<const T&>().VertexBuffers());
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FVulkanTaskProcessor::FDrawTaskCommands::FDrawTaskCommands(
    FVulkanTaskProcessor& processor,
    const TVulkanFrameTask<FSubmitRenderPass>* pSubmit,
    VkCommandBuffer vkCommandBuffer ) NOEXCEPT
:   _processor(processor)
,   _submitRef(pSubmit)
,   _vkCommandBuffer(vkCommandBuffer) {
    Assert_NoAssume(_submitRef);
    Assert_NoAssume(_vkCommandBuffer != VK_NULL_HANDLE);
}
//----------------------------------------------------------------------------
template <typename _Desc>
void FVulkanTaskProcessor::FDrawTaskCommands::BindTaskPipeline_(const TVulkanDrawTask<_Desc>& task) {
    ONLY_IF_RHIDEBUG( _processor.CmdDebugMarker_(task.TaskName(), task.DebugColor()) );

    const FVulkanLogicalRenderPass& logicalRenderPass = *_submitRef->LogicalPass();
    const FVulkanPipelineLayout* pPplnLayout = nullptr;
    VerifyRelease( _processor.BindPipeline_(&pPplnLayout, logicalRenderPass, task) );

    BindPipelineResources_(*pPplnLayout, task);

    IF_CONSTEXPR(Meta::has_defined_v<details::if_has_PushConstants_, _Desc>) {
        _processor.PushContants_(*pPplnLayout, task.PushConstants);
    }
    IF_CONSTEXPR(Meta::has_defined_v<details::if_has_DynamicStates_, _Desc>) {
        _processor.SetDynamicStates_(task.DynamicStates);
    }
    IF_CONSTEXPR(Meta::has_defined_v<details::if_has_Scissors_, _Desc>) {
        _processor.SetScissor_(logicalRenderPass, task.Scissors());
    }
    IF_CONSTEXPR(Meta::has_defined_v<details::if_has_IndexBuffer_, _Desc>) {
        _processor.BindIndexBuffer_(
            task.IndexBuffer->Handle(),
            static_cast<VkDeviceSize>(task.IndexBufferOffset),
            VkCast(task.IndexFormat) );
    }
    IF_CONSTEXPR(Meta::has_defined_v<details::if_has_VertexBuffers_, _Desc>) {
        BindVertexBuffers_(task.VertexBuffers(), task.VertexOffsets());
    }
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskCommands::BindVertexBuffers_(
    TMemoryView<const FVulkanLocalBuffer* const> vertexBuffers,
    TMemoryView<const VkDeviceSize> vertexOffsets ) const {
    if (vertexBuffers.empty()) {
        Assert_NoAssume(vertexOffsets.empty());
        return;
    }

    TFixedSizeStack<VkBuffer, MaxVertexBuffers> buffers;
    for (const FVulkanLocalBuffer* pLocalBuffer : vertexBuffers) {
        buffers.Push(pLocalBuffer->Handle());
    }

    _processor.vkCmdBindVertexBuffers(
        _vkCommandBuffer,
        0, checked_cast<u32>(buffers.size()),
        buffers.data(), vertexOffsets.data() );

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([](FFrameStatistics::FRendering& rendering) {
       rendering.NumVertexBufferBindings++;
    }));
}
//----------------------------------------------------------------------------
template <typename _DrawTask>
void FVulkanTaskProcessor::FDrawTaskCommands::BindPipelineResources_(const FVulkanPipelineLayout& layout, const _DrawTask& task) const {
    if (not task.DescriptorSets.empty()) {
        _processor.vkCmdBindDescriptorSets(
            _vkCommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            layout.Handle(),
            layout.Read()->FirstDescriptorSet,
            checked_cast<u32>(task.DescriptorSets.size()),
            task.DescriptorSets.data(),
            checked_cast<u32>(task.Resources().DynamicOffsets.size()),
            task.Resources().DynamicOffsets.data() );

        ONLY_IF_RHIDEBUG(_processor.EditStatistics_([](FFrameStatistics::FRendering& rendering) {
           rendering.NumDescriptorBinds++;
        }));
    }

#if USE_PPE_RHIDEBUG
    if (task.DebugModeIndex() != Default) {
        VkDescriptorSet vkDescriptorSet;
        u32 dynamicOffset, binding;
        if (_processor._workerCmd->Batch()->FindDescriptorSetForDebug(
            &binding, &vkDescriptorSet, &dynamicOffset,
            task.DebugModeIndex() )) {

            _processor.vkCmdBindDescriptorSets(
                _vkCommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                layout.Handle(),
                binding,
                1, &vkDescriptorSet,
                1, &dynamicOffset );

            ONLY_IF_RHIDEBUG(_processor.EditStatistics_([](FFrameStatistics::FRendering& rendering) {
               rendering.NumDescriptorBinds++;
            }));
        }
    }
#endif
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskCommands::Visit(const TVulkanDrawTask<FDrawVertices>& task) {
    BindTaskPipeline_(task);

    for (const auto& cmd : task.Commands) {
        _processor.vkCmdDraw(
            _vkCommandBuffer,
            cmd.VertexCount,
            cmd.InstanceCount,
            cmd.FirstVertex,
            cmd.FirstInstance );

        ONLY_IF_RHIDEBUG(_processor.EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
            rendering.NumDrawCalls++;
            rendering.NumVertexCount += static_cast<u64>(cmd.VertexCount) * cmd.InstanceCount;
            rendering.NumPrimitiveCount += EPrimitiveTopology_PrimitiveCount(
                task.Topology,
                static_cast<u64>(cmd.VertexCount) * cmd.InstanceCount,
                task.Pipeline->Read()->PatchControlPoints );
        }));
    }
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskCommands::Visit(const TVulkanDrawTask<FDrawIndexed>& task) {
    BindTaskPipeline_(task);

    for (const auto& cmd : task.Commands) {
        _processor.vkCmdDrawIndexed(
            _vkCommandBuffer,
            cmd.IndexCount,
            cmd.InstanceCount,
            cmd.FirstIndex,
            cmd.VertexOffset,
            cmd.FirstInstance );

        ONLY_IF_RHIDEBUG(_processor.EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
            rendering.NumDrawCalls++;
            rendering.NumVertexCount += static_cast<u64>(cmd.IndexCount) * cmd.InstanceCount;
            rendering.NumPrimitiveCount += EPrimitiveTopology_PrimitiveCount(
                task.Topology,
                static_cast<u64>(cmd.IndexCount) * cmd.InstanceCount,
                task.Pipeline->Read()->PatchControlPoints );
        }));
    }
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskCommands::Visit(const TVulkanDrawTask<FDrawVerticesIndirect>& task) {
    BindTaskPipeline_(task);

    for (const auto& cmd : task.Commands) {
        Assert_NoAssume(cmd.DrawCount <= _processor._maxDrawIndirectCount);

        _processor.vkCmdDrawIndirect(
            _vkCommandBuffer,
            task.IndirectBuffer->Handle(),
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset),
            cmd.DrawCount,
            cmd.IndirectBufferStride );

        ONLY_IF_RHIDEBUG(_processor.EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
            rendering.NumIndirectDrawCalls += cmd.DrawCount;
        }));
    }
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskCommands::Visit(const TVulkanDrawTask<FDrawIndexedIndirect>& task) {
    BindTaskPipeline_(task);

    for (const auto& cmd : task.Commands) {
        Assert_NoAssume(cmd.DrawCount <= _processor._maxDrawIndirectCount);

        _processor.vkCmdDrawIndexedIndirect(
            _vkCommandBuffer,
            task.IndirectBuffer->Handle(),
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset),
            cmd.DrawCount,
            cmd.IndirectBufferStride );

        ONLY_IF_RHIDEBUG(_processor.EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
            rendering.NumIndirectDrawCalls += cmd.DrawCount;
        }));
    }
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskCommands::Visit(const TVulkanDrawTask<FDrawVerticesIndirectCount>& task) {
    if (not _processor._enableDrawIndirectCount)
        return;

    BindTaskPipeline_(task);

    for (const auto& cmd : task.Commands) {
        Assert_NoAssume(cmd.MaxDrawCount <= _processor._maxDrawIndirectCount);

        _processor.vkCmdDrawIndirectCountKHR(
            _vkCommandBuffer,
            task.IndirectBuffer->Handle(),
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset),
            task.CountBuffer->Handle(),
            static_cast<VkDeviceSize>(cmd.CountBufferOffset),
            cmd.MaxDrawCount,
            cmd.IndirectBufferStride );
    }

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumIndirectDrawCalls += checked_cast<u32>(task.Commands.size());
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskCommands::Visit(const TVulkanDrawTask<FDrawIndexedIndirectCount>& task) {
    if (not _processor._enableDrawIndirectCount)
        return;

    BindTaskPipeline_(task);

    for (const auto& cmd : task.Commands) {
        Assert_NoAssume(cmd.MaxDrawCount <= _processor._maxDrawIndirectCount);

        _processor.vkCmdDrawIndexedIndirectCountKHR(
            _vkCommandBuffer,
            task.IndirectBuffer->Handle(),
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset),
            task.CountBuffer->Handle(),
            static_cast<VkDeviceSize>(cmd.CountBufferOffset),
            cmd.MaxDrawCount,
            cmd.IndirectBufferStride );
    }

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumIndirectDrawCalls += checked_cast<u32>(task.Commands.size());
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskCommands::Visit(const TVulkanDrawTask<FDrawMeshes>& task) {
    if (not _processor._enableMeshShaderNV)
        return;

    BindTaskPipeline_(task);

    for (const auto& cmd : task.Commands) {
        Assert_NoAssume(cmd.TaskCount <= _processor._maxDrawMeshTaskCount);

        _processor.vkCmdDrawMeshTasksNV(
            _vkCommandBuffer,
            cmd.TaskCount,
            cmd.FirstTask );
    }

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumDrawCalls += checked_cast<u32>(task.Commands.size());
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskCommands::Visit(const TVulkanDrawTask<FDrawMeshesIndirect>& task) {
    if (not _processor._enableMeshShaderNV)
        return;

    BindTaskPipeline_(task);

    for (const auto& cmd : task.Commands) {
        Assert_NoAssume(cmd.DrawCount <= _processor._maxDrawMeshTaskCount);

        _processor.vkCmdDrawMeshTasksIndirectNV(
            _vkCommandBuffer,
            task.IndirectBuffer->Handle(),
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset),
            cmd.DrawCount,
            cmd.IndirectBufferStride );

        ONLY_IF_RHIDEBUG(_processor.EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
            rendering.NumIndirectDrawCalls += cmd.DrawCount;
        }));
    }
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskCommands::Visit(const TVulkanDrawTask<FDrawMeshesIndirectCount>& task) {
    if (not _processor._enableMeshShaderNV)
        return;

    BindTaskPipeline_(task);

    for (const auto& cmd : task.Commands) {
        Assert_NoAssume(cmd.MaxDrawCount <= _processor._maxDrawIndirectCount);

        _processor.vkCmdDrawMeshTasksIndirectCountNV(
            _vkCommandBuffer,
            task.IndirectBuffer->Handle(),
            static_cast<VkDeviceSize>(cmd.IndirectBufferOffset),
            task.CountBuffer->Handle(),
            static_cast<VkDeviceSize>(cmd.CountBufferOffset),
            cmd.MaxDrawCount,
            cmd.IndirectBufferStride );
    }

    ONLY_IF_RHIDEBUG(_processor.EditStatistics_([&](FFrameStatistics::FRendering& rendering) {
        rendering.NumIndirectDrawCalls += checked_cast<u32>(task.Commands.size());
    }));
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FDrawTaskCommands::Visit(const TVulkanDrawTask<FCustomDraw>& task) {
    FDrawContext ctx{ _processor, *_submitRef->LogicalPass() };

    task.Callback(task.UserParam, ctx);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
