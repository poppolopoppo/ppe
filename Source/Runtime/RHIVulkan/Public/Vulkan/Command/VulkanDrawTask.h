#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/DrawTask.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API IVulkanDrawTask {
protected:
    using FProcessFunc = void (*)(void* visitor, void* data);

    template <typename _Task>
    IVulkanDrawTask(const _Task& task, FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT
    :   _pass1(pass1), _pass2(pass2)
#if USE_PPE_RHIDEBUG
    ,   _taskName(task.TaskName), _debugColor(task.DebugColor)
#endif
    {}

public:
    void Process1(void* visitor);
    void Process2(void* visitor);

#if USE_PPE_RHIDEBUG
    FStringView TaskName() const { return _taskName; }
    const FRgba8u DebugColor() const { return _debugColor; }

    EShaderDebugIndex DebugModeIndex() const { return _debugModeIndex; }
    void SetDebugModeIndex(EShaderDebugIndex id) { _debugModeIndex = id; }
#endif

private:
    FProcessFunc _pass1{ nullptr };
    FProcessFunc _pass2{ nullptr };

#if USE_PPE_RHIDEBUG
    FTaskName _taskName;
    FRgba8u _debugColor{ FRgba8u::One };
    EShaderDebugIndex _debugModeIndex{ Default };
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Base draw vertices:
//----------------------------------------------------------------------------
namespace details {
class FVulkanBaseDrawVerticesTask : public IVulkanDrawTask {
protected:
    using IVulkanDrawTask::FProcessFunc;

    template <typename _Task>
    FVulkanBaseDrawVerticesTask(
        FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const _Task& task, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;

public:
    using FVertexBuffers = TStaticArray<const FVulkanLocalBuffer*, MaxVertexBuffers>;
    using FVertexOffsets = TStaticArray<VkDeviceSize, MaxVertexBuffers>;
    using FVertexStrides = TStaticArray<u32, MaxVertexBuffers>;

    SCVulkanGraphicsPipeline Pipeline;
    FPushConstantDatas PushConstants;

    FVertexInputState VertexInput;

    FColorBuffers ColorBuffers;
    FDrawDynamicStates DynamicStates;

    const EPrimitiveTopology Topology;
    const bool EnablePrimitiveRestart;

    FVulkanDescriptorSets DescriptorSets;

    bool Valid() const { return (!!Pipeline); }

    const FVulkanPipelineResourceSet& Resources() const { return _resources; }
    const TMemoryView<const FRectangleI>& Scissors() const { return _scissors; }

    TMemoryView<const FVulkanLocalBuffer* const> VertexBuffers() const { return _vertexBuffers.MakeConstView().CutBeforeConst(_vertexBufferCount); }
    TMemoryView<const VkDeviceSize> VertexOffsets() const { return _vertexOffsets.MakeConstView().CutBeforeConst(_vertexBufferCount); }
    TMemoryView<const u32> VertexStrides() const { return _vertexStrides.MakeConstView().CutBeforeConst(_vertexBufferCount); }

private:
    FVulkanPipelineResourceSet _resources;

    FVertexBuffers _vertexBuffers;
    FVertexOffsets _vertexOffsets;
    FVertexStrides _vertexStrides;
    u32 _vertexBufferCount{ 0 };

    TMemoryView<const FRectangleI> _scissors;
};
} //!details
//----------------------------------------------------------------------------
// FDrawVertices:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawVertices> final : public details::FVulkanBaseDrawVerticesTask {
public:
    const FDrawVertices::FDrawCommands Commands;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawVertices& task, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
};
//----------------------------------------------------------------------------
// FDrawIndexed:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawIndexed> final : public details::FVulkanBaseDrawVerticesTask {
public:
    const FDrawIndexed::FDrawCommands Commands;

    const FVulkanLocalBuffer* const IndexBuffer;
    const u32 IndexBufferOffset;
    const EIndexFormat IndexFormat;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawIndexed& task, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
};
//----------------------------------------------------------------------------
// FDrawVerticesIndirect:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawVerticesIndirect> final : public details::FVulkanBaseDrawVerticesTask {
public:
    const FDrawVerticesIndirect::FDrawCommands Commands;

    const FVulkanLocalBuffer* const IndirectBuffer;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawVerticesIndirect& task, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
};
//----------------------------------------------------------------------------
// FDrawIndexedIndirect:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawIndexedIndirect> final : public details::FVulkanBaseDrawVerticesTask {
public:
    const FDrawIndexedIndirect::FDrawCommands Commands;

    const FVulkanLocalBuffer* const IndirectBuffer;

    const FVulkanLocalBuffer* const IndexBuffer;
    const u32 IndexBufferOffset;
    const EIndexFormat IndexFormat;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawIndexedIndirect& task, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
};
//----------------------------------------------------------------------------
// FDrawVerticesIndirectCount:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawVerticesIndirectCount> final : public details::FVulkanBaseDrawVerticesTask {
    public:
    const FDrawVerticesIndirectCount::FDrawCommands Commands;

    const FVulkanLocalBuffer* const IndirectBuffer;
    const FVulkanLocalBuffer* const CountBuffer;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawVerticesIndirectCount& task, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
};
//----------------------------------------------------------------------------
// FDrawIndexedIndirectCount:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawIndexedIndirectCount> final : public details::FVulkanBaseDrawVerticesTask {
    public:
    const FDrawIndexedIndirectCount::FDrawCommands Commands;

    const FVulkanLocalBuffer* const IndirectBuffer;
    const FVulkanLocalBuffer* const CountBuffer;

    const FVulkanLocalBuffer* const IndexBuffer;
    const u32 IndexBufferOffset;
    const EIndexFormat IndexFormat;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawIndexedIndirectCount& task, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Base draw meshes:
//----------------------------------------------------------------------------
namespace details {
class FVulkanBaseDrawMeshesTask : public IVulkanDrawTask {
protected:
    using IVulkanDrawTask::FProcessFunc;

    template <typename _Task>
    FVulkanBaseDrawMeshesTask(
        FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const _Task& task, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;

public:
    SCVulkanMeshPipeline Pipeline;
    FPushConstantDatas PushConstants;

    FColorBuffers ColorBuffers;
    FDrawDynamicStates DynamicStates;

    FVulkanDescriptorSets DescriptorSets;

    bool Valid() const { return (!!Pipeline); }

    const FVulkanPipelineResourceSet& Resources() const { return _resources; }
    const TMemoryView<const FRectangleI>& Scissors() const { return _scissors; }

private:
    FVulkanPipelineResourceSet _resources;
    TMemoryView<const FRectangleI> _scissors;
};
} //!details
//----------------------------------------------------------------------------
// FDrawMeshes:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawMeshes> final : public details::FVulkanBaseDrawMeshesTask {
public:
    const FDrawMeshes::FDrawCommands Commands;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawMeshes& task, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
};
//----------------------------------------------------------------------------
// FDrawMeshesIndirect:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawMeshesIndirect> final : public details::FVulkanBaseDrawMeshesTask {
public:
    const FDrawMeshesIndirect::FDrawCommands Commands;

    const FVulkanLocalBuffer* const IndirectBuffer;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawMeshesIndirect& task, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
};
//----------------------------------------------------------------------------
// FDrawMeshesIndirect:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawMeshesIndirectCount> final : public details::FVulkanBaseDrawMeshesTask {
    public:
    const FDrawMeshesIndirectCount::FDrawCommands Commands;

    const FVulkanLocalBuffer* const IndirectBuffer;
    const FVulkanLocalBuffer* const CountBuffer;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawMeshesIndirectCount& task, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FCustomDraw:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FCustomDraw> final : public IVulkanDrawTask {
public:
    using FCallback = FCustomDraw::FCallback;

    using FImages = TMemoryView<const TPair<const FVulkanLocalImage*, EResourceState>>;
    using FBuffers = TMemoryView<const TPair<const FVulkanLocalBuffer*, EResourceState>>;

    const FCallback Callback;
    void* const CallbackParam;

    const FImages Images;
    const FBuffers Buffers;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FCustomDraw& task, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;

    bool Valid() const { return (!!Callback); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
