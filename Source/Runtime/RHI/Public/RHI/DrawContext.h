#pragma once

#include "RHI_fwd.h"

#include "Color/Color_fwd.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IDrawContext : public FRefCountable {
public: // interface
    virtual ~IDrawContext() = default;

    // returns API-specific data.
    // this data can be used to set states and write draw commands directly to the vulkan command buffer.
    virtual void* ExternalContext() const = 0;

    // reset current states to default states:
    //    - render states copied from logical render pass
    //    - resets all pipelines and descriptor sets
    //    - resets vertex and index buffers
    // use this if you set states by raw API-specific function call and want to continue using context api.
    virtual void Reset() = 0;

    // pipelines
    virtual void BindPipeline(FRawGPipelineID id, EPipelineDynamicState dynamicState = EPipelineDynamicState::Default) = 0;
    virtual void BindPipeline(FRawMPipelineID id, EPipelineDynamicState dynamicState = EPipelineDynamicState::Default) = 0;

    // resources (descriptor sets)
    virtual void BindResources(const FDescriptorSetID& id, const FPipelineResources& res) = 0;
    virtual void PushConstants(const FPushConstantID& id, const void* data, size_t dataSize) = 0;
    virtual void BindShadingRateImage(FRawImageID value, FImageLayer layer = Default, FMipmapLevel level = Default) = 0;

    // vertex attributes and index buffer
    virtual void BindVertexAttribs(const FVertexInputState&) = 0;
    virtual void BindVertexBuffer(const FVertexBufferID&id, FRawBufferID vbuf, size_t offset) = 0;
    virtual void BindIndexBuffer(FRawBufferID buffer, size_t offset, EIndexFormat fmt) = 0;

    // render states
    virtual void SetColorBuffer(ERenderTargetID id, const FColorBufferState& value) = 0;
    virtual void SetLogicOp(ELogicOp value) = 0;
    virtual void SetBlendColor(const FLinearColor& value) = 0;
    virtual void SetStencilBuffer(const FStencilBufferState& value) = 0;
    virtual void SetDepthBuffer(const FDepthBufferState& value) = 0;
    virtual void SetInputAssembly(const FInputAssemblyState& value) = 0;
    virtual void SetRasterization(const FRasterizationState& value) = 0;
    virtual void SetMultisample(const FMultisampleState& value) = 0;

    // dynamic states
    virtual void SetStencilCompareMask(u32 value) = 0;
    virtual void SetStencilWriteMask(u32 value) = 0;
    virtual void SetStencilReference(u32 value) = 0;
    virtual void SetShadingRatePalette(u32 viewportIndex, TMemoryView<EShadingRatePalette> value) = 0;

    // draw commands
    virtual void DrawVertices(
        u32 vertexCount,
        u32 instanceCount = 1,
        u32 firstVertex = 0,
        u32 firstInstance = 0) = 0;

    virtual void DrawIndexed(
        u32 indexCount,
        u32 instanceCount = 1,
        u32 firstIndex = 0,
        i32 vertexOffset = 0,
        u32 firstInstance = 0) = 0;

    virtual void DrawVerticesIndirect(
        FRawBufferID indirectBuffer,
        size_t indirectBufferOffset,
        u32 drawCount,
        size_t stride = 0) = 0;

    virtual void DrawIndexedIndirect(
        FRawBufferID indirectBuffer,
        size_t indirectBufferOffset,
        u32 drawCount,
        size_t stride = 0) = 0;

    virtual void DrawMeshes(u32 taskCount, u32 firstTask = 0) = 0;

    virtual void DrawMeshesIndirect(
        FRawBufferID indirectBuffer,
        size_t indirectBufferOffset,
        u32 drawCount,
        size_t stride = 0) = 0;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
