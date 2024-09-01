#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Pipeline/VulkanGraphicsPipeline.h"
#include "Vulkan/Pipeline/VulkanMeshPipeline.h"

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
    IVulkanDrawTask(
        FVulkanCommandBuffer& cmd,
        const _Task& desc, FProcessFunc pass1, FProcessFunc pass2) NOEXCEPT;

    FProcessFunc Pass1{ nullptr };
    FProcessFunc Pass2{ nullptr };

public:
#if USE_PPE_RHIDEBUG
    FConstChar TaskName;
    FColor DebugColor;
    EShaderDebugIndex DebugModeIndex{ Default };
#endif

    void Process1(void* visitor) {
        Assert(Pass1);
        Pass1(visitor, this);
    }

    void Process2(void* visitor) {
        Assert(Pass2);
        Pass2(visitor, this);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Base draw vertices:
//----------------------------------------------------------------------------
namespace details {
class PPE_RHIVULKAN_API FVulkanBaseDrawVerticesTask : public IVulkanDrawTask {
protected:
    using IVulkanDrawTask::FProcessFunc;

    template <typename _Task>
    FVulkanBaseDrawVerticesTask(
        FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const _Task& desc, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;

public:

    using FBufferBinding = TPair<FVertexBufferID, FVertexBufferBinding>;
    using FColorBuffer = TPair<const ERenderTargetID, FColorBufferState>;
    using FResource = FVulkanPipelineResourceSet::FResource;
    using FVertexInput = TPair<const FVertexID, FVertexInput>;

    const TPtrRef<const FVulkanGraphicsPipeline> Pipeline;
    const FDrawDynamicStates DynamicStates;
    const EPrimitiveTopology Topology;
    const bool EnablePrimitiveRestart;

    TMemoryView<u32> DynamicOffsets;
    TMemoryView<const FResource> Resources;

    TMemoryView<const FColorBuffer> ColorBuffers;
    TMemoryView<const FPushConstantData> PushConstants;
    TMemoryView<const FRectangle2u> Scissors;

    TMemoryView<const FBufferBinding> BufferBindings;
    TMemoryView<const FVertexInput> VertexInputs;

    TMemoryView<const TPtrRef<const FVulkanLocalBuffer>> VertexBuffers;
    TMemoryView<const VkDeviceSize> VertexOffsets;
    TMemoryView<const u32> VertexStrides;

    mutable FVulkanDescriptorSets DescriptorSets;

    bool Valid() const NOEXCEPT {
        return (!!Pipeline);
    }
};
} //!details
//----------------------------------------------------------------------------
// FDrawVertices:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawVertices> final : public details::FVulkanBaseDrawVerticesTask {
public:
    const TMemoryView<const FDrawVertices::FDrawCommand> Commands;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawVertices& desc, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
    ~TVulkanDrawTask();
};
//----------------------------------------------------------------------------
// FDrawIndexed:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawIndexed> final : public details::FVulkanBaseDrawVerticesTask {
public:
    const TMemoryView<const FDrawIndexed::FDrawCommand> Commands;

    const TPtrRef<const FVulkanLocalBuffer> IndexBuffer;
    const u32 IndexBufferOffset;
    const EIndexFormat IndexFormat;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawIndexed& desc, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
    ~TVulkanDrawTask();
};
//----------------------------------------------------------------------------
// FDrawVerticesIndirect:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawVerticesIndirect> final : public details::FVulkanBaseDrawVerticesTask {
public:
    const TMemoryView<const FDrawVerticesIndirect::FDrawCommand> Commands;

    const TPtrRef<const FVulkanLocalBuffer> IndirectBuffer;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawVerticesIndirect& desc, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
    ~TVulkanDrawTask();
};
//----------------------------------------------------------------------------
// FDrawIndexedIndirect:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawIndexedIndirect> final : public details::FVulkanBaseDrawVerticesTask {
public:
    const TMemoryView<const FDrawIndexedIndirect::FDrawCommand> Commands;

    const TPtrRef<const FVulkanLocalBuffer> IndirectBuffer;

    const TPtrRef<const FVulkanLocalBuffer> IndexBuffer;
    const u32 IndexBufferOffset;
    const EIndexFormat IndexFormat;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawIndexedIndirect& desc, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
    ~TVulkanDrawTask();
};
//----------------------------------------------------------------------------
// FDrawVerticesIndirectCount:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawVerticesIndirectCount> final : public details::FVulkanBaseDrawVerticesTask {
public:
    const TMemoryView<const FDrawVerticesIndirectCount::FDrawCommand> Commands;

    const TPtrRef<const FVulkanLocalBuffer> IndirectBuffer;
    const TPtrRef<const FVulkanLocalBuffer> CountBuffer;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawVerticesIndirectCount& desc, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
    ~TVulkanDrawTask();
};
//----------------------------------------------------------------------------
// FDrawIndexedIndirectCount:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawIndexedIndirectCount> final : public details::FVulkanBaseDrawVerticesTask {
public:
    const TMemoryView<const FDrawIndexedIndirectCount::FDrawCommand> Commands;

    const TPtrRef<const FVulkanLocalBuffer> IndirectBuffer;
    const TPtrRef<const FVulkanLocalBuffer> CountBuffer;

    const TPtrRef<const FVulkanLocalBuffer> IndexBuffer;
    const u32 IndexBufferOffset;
    const EIndexFormat IndexFormat;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawIndexedIndirectCount& desc, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
    ~TVulkanDrawTask();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef VK_NV_mesh_shader
//----------------------------------------------------------------------------
// Base draw meshes:
//----------------------------------------------------------------------------
namespace details {
class PPE_RHIVULKAN_API FVulkanBaseDrawMeshesTask : public IVulkanDrawTask {
protected:
    using IVulkanDrawTask::FProcessFunc;

    template <typename _Task>
    FVulkanBaseDrawMeshesTask(
        FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const _Task& desc, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;

public:
    using FColorBuffer = TPair<const ERenderTargetID, FColorBufferState>;
    using FResource = FVulkanPipelineResourceSet::FResource;

    const TPtrRef<const FVulkanMeshPipeline> Pipeline;
    FDrawDynamicStates DynamicStates;

    TMemoryView<u32> DynamicOffsets;
    TMemoryView<const FResource> Resources;

    TMemoryView<const FColorBuffer> ColorBuffers;
    TMemoryView<const FPushConstantData> PushConstants;
    TMemoryView<const FRectangle2u> Scissors;

    mutable FVulkanDescriptorSets DescriptorSets;

    bool Valid() const { return (!!Pipeline); }
};
} //!details
//----------------------------------------------------------------------------
// FDrawMeshes:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawMeshes> final : public details::FVulkanBaseDrawMeshesTask {
public:
    const TMemoryView<const FDrawMeshes::FDrawCommand> Commands;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawMeshes& desc, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
    ~TVulkanDrawTask();
};
//----------------------------------------------------------------------------
// FDrawMeshesIndirect:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawMeshesIndirect> final : public details::FVulkanBaseDrawMeshesTask {
public:
    const TMemoryView<const FDrawMeshesIndirect::FDrawCommand> Commands;

    const TPtrRef<const FVulkanLocalBuffer> IndirectBuffer;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawMeshesIndirect& desc, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
    ~TVulkanDrawTask();
};
//----------------------------------------------------------------------------
// FDrawMeshesIndirect:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FDrawMeshesIndirectCount> final : public details::FVulkanBaseDrawMeshesTask {
public:
    const TMemoryView<const FDrawMeshesIndirectCount::FDrawCommand> Commands;

    const TPtrRef<const FVulkanLocalBuffer> IndirectBuffer;
    const TPtrRef<const FVulkanLocalBuffer> CountBuffer;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FDrawMeshesIndirectCount& desc, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
    ~TVulkanDrawTask();
};
//----------------------------------------------------------------------------
#endif //!VK_NV_mesh_shader
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FCustomDraw:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanDrawTask<FCustomDraw> final : public IVulkanDrawTask {
public:
    using FCallback = FCustomDraw::FCallback;

    using FImage = TPair<TPtrRef<const FVulkanLocalImage>, EResourceState>;
    using FBuffer = TPair<TPtrRef<const FVulkanLocalBuffer>, EResourceState>;

    const FCallback Callback;
    void* const UserParam;

    const TMemoryView<const FImage> Images;
    const TMemoryView<const FBuffer> Buffers;

    TVulkanDrawTask(FVulkanLogicalRenderPass& renderPass, FVulkanCommandBuffer& cmd,
        const FCustomDraw& desc, FProcessFunc pass1, FProcessFunc pass2 ) NOEXCEPT;
    ~TVulkanDrawTask();

    bool Valid() const { return Callback.Valid(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
