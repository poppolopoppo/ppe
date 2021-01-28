#pragma once

#include "RHI_fwd.h"

#include "RHI/FrameDebug.h"
#include "RHI/PipelineResources.h"
#include "RHI/ResourceTypes.h"
#include "RHI/RenderState.h"
#include "RHI/VertexInputState.h"

#include "Maths/ScalarRectangle.h"
#include "Misc/Function.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename _Self>
struct TDrawTaskDesc {
    using self_type = _Self;

    TDrawTaskDesc() = default;

#if USE_PPE_RHITASKNAME
    FTaskName Name;
    FRgba8u DebugColor;

    TDrawTaskDesc(FStringView name, FRgba8u color) : Name(name), DebugColor(color) {}

    self_type& SetName(FStringView value) { Name.Assign(value); return static_cast<self_type&>(*this); }
    self_type& SetDebugColor(FRgba8u value) { DebugColor = value; return static_cast<self_type&>(*this); }
#endif
};
} //!details
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
struct FGraphicsShaderDebugMode {
    EShaderDebugMode Mode{ Default };
    EShaderStages Stages{ Default };
    short2 FragCoord{ TNumericLimits<short2>::MinValue() };
};
#endif
//----------------------------------------------------------------------------
// Draw calls
//----------------------------------------------------------------------------
namespace details {
template <typename _Task>
struct TDrawCallDesc : TDrawTaskDesc<_Task> {

    FPipelineResourceSet Resources;
    FPushConstantDatas PushConstants;
    FScissors Scissors;
    FColorBuffers ColorBuffers;
    FDrawDynamicStates DynamicStates;

#if USE_PPE_RHIDEBUG
    using FDebugMode = FGraphicsShaderDebugMode;
    FDebugMode DebugMode;
#endif

    TDrawCallDesc() = default;
#if USE_PPE_RHITASKNAME
    TDrawCallDesc(FStringView name, FRgba8u color) : TDrawTaskDesc<_Task>(name, color) {}
#endif

    _Task& AddResources(const FDescriptorSetID& id, const FPipelineResources* res);

    _Task& AddScissor(const FRectangleI& clip);

    _Task& AddColorBuffer(ERenderTargetID id, const FColorBufferState& cb);
    _Task& AddColorBuffer(ERenderTargetID id, EColorMask colorMask = EColorMask::All );
    _Task& AddColorBuffer(ERenderTargetID id,
        EBlendFactor srcBlend, EBlendFactor dstBlend, EBlendOp blendOp, EColorMask colorMask = EColorMask::All );
    _Task& AddColorBuffer(ERenderTargetID id,
        EBlendFactor srcBlendColor, EBlendFactor srcBlendAlpha,
        EBlendFactor dstBlendColor, EBlendFactor dstBlendAlpha,
        EBlendOp blendOpColor, EBlendOp blendOpAlpha,
        EColorMask colorMask = EColorMask::All );

    template <typename T>
    _Task& AddPushConstant(const FPushConstantID& id, const T& value) { return AddPushConstant(id, &value, sizeof(value)); }
    _Task& AddPushConstant(const FPushConstantID& id, const void* p, size_t size);

#define DEF_DYNAMICSTATE_SET(ID, TYPE, NAME, SUFF) \
    _Task& CONCAT(Set, NAME)(TYPE value) { DynamicStates.CONCAT(Set, NAME)(value); return static_cast<_Task&>(*this); }
    PPE_RHI_EACH_DYNAMICSTATE(DEF_DYNAMICSTATE_SET)
#undef DEF_DYNAMICSTATE_SET

#if USE_PPE_RHIDEBUG
    _Task& EnableShaderDebugTrace(EShaderStages stages);
    _Task& EnableFragmentDebugTrace(int x, int y);
#endif

#if USE_PPE_RHIPROFILING
    _Task& EnableShaderProfiling(EShaderStages stages);
    _Task& EnableFragmentProfiling(int x, int y);
#endif

};
} //!details
//----------------------------------------------------------------------------
namespace details {
template <typename _Task>
struct TDrawVerticesDesc : TDrawCallDesc<_Task> {
    using FVertexBuffers = TFixedSizeHashMap<FVertexBufferID, FVertexBuffer, MaxVertexBuffers>;

    STATIC_CONST_INTEGRAL(u16, RestartIndex16, UMax); // restart the assembly of primitives
    STATIC_CONST_INTEGRAL(u32, RestartIndex32, UMax); //

    FRawGPipelineID Pipeline;
    EPrimitiveTopology Topology{ Default };

    FVertexInputState VertexInput;
    FVertexBuffers VertexBuffers;

    bool EnablePrimitiveRestart{ false };

    TDrawVerticesDesc() = default;
#if USE_PPE_RHITASKNAME
    TDrawVerticesDesc(FStringView name, FRgba8u color) : TDrawCallDesc<_Task>(name, color) {}
#endif

    _Task& SetTopology(EPrimitiveTopology value) { Topology = value; return static_cast<_Task&>(*this); }
    _Task& SetPipeline(FRawGPipelineID value) { Assert(value); Pipeline = value; return static_cast<_Task&>(*this); }

    _Task& SetVertexInput(const FVertexInputState& value) { VertexInput = value; return static_cast<_Task&>(*this); }
    _Task& SetEnablePrimitiveRestart(bool value) { EnablePrimitiveRestart = value; return static_cast<_Task&>(*this); }

    _Task& AddVertexBuffer(const FVertexBufferID& id, FRawBufferID buffer, u32 offset = 0) {
        Assert(buffer);
        VertexBuffers.Add_Overwrite(id, FVertexBuffer{ buffer, offset });
        return static_cast<_Task&>(*this);
    }

};
} //!details
//----------------------------------------------------------------------------
// FDrawVertices
//----------------------------------------------------------------------------
struct FDrawVertices final : details::TDrawVerticesDesc<FDrawVertices> {
    struct FDrawCommand {
        u32 VertexCount{ 0 };
        u32 InstanceCount{ 1 };
        u32 FirstVertex{ 0 };
        u32 FirstInstance{ 0 };
    };

    using FDrawCommands = TFixedSizeStack<FDrawCommand, MaxDrawCommands>;

    FDrawCommands Commands;

#if USE_PPE_RHITASKNAME
    FDrawVertices() : TDrawVerticesDesc<FDrawVertices>{ "DrawVertices", FDebugColorScheme::Get().Draw } {}
#endif

    FDrawVertices& Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0) {
        Assert(vertexCount > 0);
        Emplace_Back(Commands, vertexCount, instanceCount, firstVertex, firstInstance);
        return (*this);
    }
};
//----------------------------------------------------------------------------
// FDrawIndexed
//----------------------------------------------------------------------------
struct FDrawIndexed : details::TDrawVerticesDesc<FDrawIndexed> {
    struct FDrawCommand {
        u32 IndexCount{ 0 };
        u32 InstanceCount{ 1 };
        u32 FirstIndex{ 0 };
        i32 VertexOffset{ 0 };
        u32 FirstInstance{ 0 };
    };

    using FDrawCommands = TFixedSizeStack<FDrawCommand, MaxDrawCommands>;

    FRawBufferID IndexBuffer;
    u32 IndexBufferOffset{ 0 };
    EIndexFormat IndexFormat{ Default };
    FDrawCommands Commands;

#if USE_PPE_RHITASKNAME
    FDrawIndexed() : TDrawVerticesDesc<FDrawIndexed>{ "DrawIndexed", FDebugColorScheme::Get().Draw } {}
#endif

    FDrawIndexed& SetIndexBuffer(FRawBufferID buffer, u32 offset = 0, EIndexFormat fmt = Default) {
        Assert(buffer);
        IndexBuffer = buffer;
        IndexBufferOffset = offset;
        IndexFormat = fmt;
        return (*this);
    }

    FDrawIndexed& Draw(u32 indexCount, u32 instanceCount = 1, u32 firstIndex = 0, i32 vertexOffset = 0, u32 firstInstance = 0) {
        Assert(indexCount > 0);
        Emplace_Back(Commands, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        return (*this);
    }
};
//----------------------------------------------------------------------------
// FDrawVerticesIndirect
//----------------------------------------------------------------------------
struct FDrawVerticesIndirect : details::TDrawVerticesDesc<FDrawVerticesIndirect> {
    struct FDrawCommand {
        u32 IndirectBufferOffset{ 0 };
        u32 DrawCount{ 0 };
        u32 Stride{ UMax };
    };

    using FDrawCommands = TFixedSizeStack<FDrawCommand, MaxDrawCommands>;

    struct FIndirectCommand {
        u32 VertexCount{ 0 };
        u32 InstanceCount{ 0 };
        u32 FirstVertex{ 0 };
        u32 FirstInstance{ 0 };
    };
    STATIC_ASSERT(sizeof(FIndirectCommand) == 16);

    FDrawCommands Commands;
    FRawBufferID IndirectBuffer;

#if USE_PPE_RHITASKNAME
    FDrawVerticesIndirect() : TDrawVerticesDesc<FDrawVerticesIndirect>{ "DrawVerticesIndirect", FDebugColorScheme::Get().Draw } {}
#endif

    FDrawVerticesIndirect& SetIndirectBuffer(FRawBufferID buffer) {
        Assert(buffer);
        IndirectBuffer = buffer;
        return (*this);
    }

    FDrawVerticesIndirect& Draw(u32 drawCount, u32 indirectBufferOffset = 0, u32 stride = sizeof(FIndirectCommand)) {
        Assert(drawCount > 0);
        Emplace_Back(Commands, indirectBufferOffset, drawCount, stride);
        return (*this);
    }
};
//----------------------------------------------------------------------------
// FDrawIndexedIndirect
//----------------------------------------------------------------------------
struct FDrawIndexedIndirect : details::TDrawVerticesDesc<FDrawIndexedIndirect> {
    using FDrawCommand = FDrawVerticesIndirect::FDrawCommand;
    using FDrawCommands = FDrawVerticesIndirect::FDrawCommands;

    struct FIndirectCommand {
        u32 IndexCount{ 0 };
        u32 InstanceCount{ 1 };
        u32 FirstIndex{ 0 };
        i32 VertexOffset{ 0 };
        u32 FirstInstance{ 0 };
    };
    STATIC_ASSERT(sizeof(FIndirectCommand) == 20);

    FRawBufferID IndexBuffer;
    u32 IndexBufferOffset{ 0 };
    EIndexFormat IndexFormat{ Default };

    FDrawCommands Commands;
    FRawBufferID IndirectBuffer;

#if USE_PPE_RHITASKNAME
    FDrawIndexedIndirect() : TDrawVerticesDesc<FDrawIndexedIndirect>{ "DrawIndexedIndirect", FDebugColorScheme::Get().Draw } {}
#endif

    FDrawIndexedIndirect& SetIndexBuffer(FRawBufferID buffer, u32 offset = 0, EIndexFormat fmt = Default) {
        Assert(buffer);
        IndexBuffer = buffer;
        IndexBufferOffset = offset;
        IndexFormat = fmt;
        return (*this);
    }

    FDrawIndexedIndirect& SetIndirectBuffer(FRawBufferID buffer) {
        Assert(buffer);
        IndirectBuffer = buffer;
        return (*this);
    }

    FDrawIndexedIndirect& Draw(u32 drawCount, u32 indirectBufferOffset = 0, u32 stride = sizeof(FIndirectCommand)) {
        Assert(drawCount > 0);
        Emplace_Back(Commands, indirectBufferOffset, drawCount, stride);
        return (*this);
    }
};
//----------------------------------------------------------------------------
// FDrawMeshes
//----------------------------------------------------------------------------
struct FDrawMeshes final : details::TDrawCallDesc<FDrawMeshes> {
    struct FDrawCommand {
        u32 MeshCount{ 0 };
        u32 FirstMesh{ 0 };
    };

    using FDrawCommands = TFixedSizeStack<FDrawCommand, MaxDrawCommands>;

    FRawMPipelineID Pipeline;
    FDrawCommands Commands;

#if USE_PPE_RHITASKNAME
    FDrawMeshes() : TDrawCallDesc<FDrawMeshes>{ "DrawMeshes", FDebugColorScheme::Get().DrawMeshes } {}
#endif

    FDrawMeshes& SetPipeline(FRawMPipelineID value) {
        Assert(value);
        Pipeline = value;
        return (*this);
    }

    FDrawMeshes& Draw(u32 meshCount, u32 firstMesh = 0) {
        Assert(meshCount > 0);
        Emplace_Back(Commands, meshCount, firstMesh);
        return (*this);
    }
};
//----------------------------------------------------------------------------
// FDrawMeshesIndirect
//----------------------------------------------------------------------------
struct FDrawMeshesIndirect final : details::TDrawCallDesc<FDrawMeshesIndirect> {
    using FDrawCommand = FDrawVerticesIndirect::FDrawCommand;
    using FDrawCommands = TFixedSizeStack<FDrawCommand, MaxDrawCommands>;

    struct FIndirectCommand {
        u32 TaskCount{ 0 };
        u32 FirstTask{ 0 };
    };
    STATIC_ASSERT(sizeof(FIndirectCommand) == 8);

    FRawMPipelineID Pipeline;
    FDrawCommands Commands;
    FRawBufferID IndirectBuffer;

#if USE_PPE_RHITASKNAME
    FDrawMeshesIndirect() : TDrawCallDesc<FDrawMeshesIndirect>{ "DrawMeshesIndirect", FDebugColorScheme::Get().DrawMeshes } {}
#endif

    FDrawMeshesIndirect& SetPipeline(FRawMPipelineID value) {
        Assert(value);
        Pipeline = value;
        return (*this);
    }

    FDrawMeshesIndirect& SetIndirectBuffer(FRawBufferID buffer) {
        Assert(buffer);
        IndirectBuffer = buffer;
        return (*this);
    }

    FDrawMeshesIndirect& Draw(u32 drawCount, u32 indirectBufferOffset = 0, u32 stride = sizeof(FIndirectCommand)) {
        Assert(drawCount > 0);
        Emplace_Back(Commands, drawCount, indirectBufferOffset, stride);
        return (*this);
    }
};
//----------------------------------------------------------------------------
// FCustomDraw
//----------------------------------------------------------------------------
struct FCustomDraw final : details::TDrawTaskDesc<FCustomDraw> {
    using FCallback = TFunction<void(void*, IDrawContext&)>;
    using FImages = TFixedSizeHashMap<FRawImageID, EResourceState, MaxResourceStates>;
    using FBuffers = TFixedSizeHashMap<FRawBufferID, EResourceState, MaxResourceStates>;

    FCallback Callback;
    FImages Images;
    FBuffers Buffers;

#if USE_PPE_RHITASKNAME
    FCustomDraw() = default;
#else
    FCustomDraw() : TDrawTaskDesc<FCustomDraw>{ "CustomDraw", FDebugColorScheme::Get().CustomDraw } {}
#endif

    explicit FCustomDraw(FCallback&& rcallback) : FCustomDraw() {
        Callback = std::move(rcallback);
    }

    FCustomDraw& AddImage(FRawImageID image, EResourceState state = EResourceState::ShaderSample) {
        Images.Add_Overwrite(image, state);
        return (*this);
    }

    FCustomDraw& AddBuffer(FRawBufferID buffer, EResourceState state = EResourceState::ShaderSample) {
        Buffers.Add_Overwrite(buffer, state);
        return (*this);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#include "RHI/DrawTask-inl.h"
