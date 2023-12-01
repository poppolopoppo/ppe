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
    FLinearColor DebugColor;

    TDrawTaskDesc(FConstChar name, FLinearColor color) NOEXCEPT : Name(name), DebugColor(color) {}

    self_type& SetName(const FStringView& value) { Name.Assign(value); return static_cast<self_type&>(*this); }
    self_type& SetDebugColor(const FLinearColor& value) { DebugColor = value; return static_cast<self_type&>(*this); }

#else
    // tolerant API to simply client integration:
    CONSTEXPR self_type& SetName(const FStringView&) { return static_cast<self_type&>(*this); }
    CONSTEXPR self_type& SetDebugColor(const FLinearColor&) { return static_cast<self_type&>(*this); }

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
    TDrawCallDesc(FConstChar name, const FLinearColor& color) NOEXCEPT : TDrawTaskDesc<_Task>(name, color) {}
#endif

    _Task& AddResources(FDescriptorSetID&& rid, PCPipelineResources&& rres);
    _Task& AddResources(const FDescriptorSetID& id, const PCPipelineResources& res);

    _Task& AddScissor(const FRectangleU& clip);

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

#if USE_PPE_RHIDEBUG
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

    bool EnablePrimitiveRestart{ false };

    FVertexInputState VertexInput;
    FVertexBuffers VertexBuffers;

    TDrawVerticesDesc() = default;
#if USE_PPE_RHITASKNAME
    TDrawVerticesDesc(FConstChar name, const FLinearColor& color) NOEXCEPT : TDrawCallDesc<_Task>(name, color) {}
#endif

    _Task& SetTopology(EPrimitiveTopology value) { Topology = value; return static_cast<_Task&>(*this); }
    _Task& SetPipeline(FRawGPipelineID value) { Assert(value); Pipeline = value; return static_cast<_Task&>(*this); }

    _Task& SetVertexInput(const FVertexInputState& value) { VertexInput = value; return static_cast<_Task&>(*this); }
    _Task& SetEnablePrimitiveRestart(bool value) { EnablePrimitiveRestart = value; return static_cast<_Task&>(*this); }

    _Task& AddVertexBuffer(const FVertexBufferID& id, FRawBufferID buffer, size_t offset = 0) {
        //Assert(id); // support one anonymous vertex buffer
        Assert(buffer);
        VertexBuffers.Emplace_Overwrite(id, FVertexBuffer{ buffer, offset });
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

    PPE_RHI_API FDrawVertices() NOEXCEPT;
    PPE_RHI_API ~FDrawVertices();

    PPE_RHI_API FDrawVertices& Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0);
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

    PPE_RHI_API FDrawIndexed() NOEXCEPT;
    PPE_RHI_API ~FDrawIndexed();

    PPE_RHI_API FDrawIndexed& SetIndexBuffer(FRawBufferID buffer, u32 offset = 0, EIndexFormat fmt = Default);
    PPE_RHI_API FDrawIndexed& Draw(u32 indexCount, u32 instanceCount = 1, u32 firstIndex = 0, i32 vertexOffset = 0, u32 firstInstance = 0);
};
//----------------------------------------------------------------------------
// FDrawVerticesIndirect
//----------------------------------------------------------------------------
struct FDrawVerticesIndirect : details::TDrawVerticesDesc<FDrawVerticesIndirect> {
    struct FDrawCommand {
        u32 IndirectBufferOffset{ 0 };
        u32 DrawCount{ 0 };
        u32 IndirectBufferStride{ UMax };
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

    PPE_RHI_API FDrawVerticesIndirect() NOEXCEPT;
    PPE_RHI_API ~FDrawVerticesIndirect();

    PPE_RHI_API FDrawVerticesIndirect& SetIndirectBuffer(FRawBufferID buffer);
    PPE_RHI_API FDrawVerticesIndirect& Draw(u32 drawCount, u32 indirectBufferOffset = 0, u32 indirectBufferStride = sizeof(FIndirectCommand));
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

    PPE_RHI_API FDrawIndexedIndirect() NOEXCEPT;
    PPE_RHI_API ~FDrawIndexedIndirect();

    PPE_RHI_API FDrawIndexedIndirect& SetIndexBuffer(FRawBufferID buffer, u32 offset = 0, EIndexFormat fmt = Default);
    PPE_RHI_API FDrawIndexedIndirect& SetIndirectBuffer(FRawBufferID buffer);
    PPE_RHI_API FDrawIndexedIndirect& Draw(u32 drawCount, u32 indirectBufferOffset = 0, u32 stride = sizeof(FIndirectCommand));
};
//----------------------------------------------------------------------------
// FDrawVerticesIndirectCount
//----------------------------------------------------------------------------
struct FDrawVerticesIndirectCount : details::TDrawVerticesDesc<FDrawVerticesIndirectCount> {
    struct FDrawCommand {
        u32 IndirectBufferOffset{ 0 };
        u32 CountBufferOffset{ 0 };
        u32 MaxDrawCount{ 0 };
        u32 IndirectBufferStride{ UMax };
    };

    using FDrawCommands = TFixedSizeStack<FDrawCommand, MaxDrawCommands>;
    using FIndirectCommand = FDrawVerticesIndirect::FIndirectCommand;

    FDrawCommands Commands;
    FRawBufferID IndirectBuffer; // contains array of 'DrawIndirectCommand'
    FRawBufferID CountBuffer; // contains single 'uint' value

    PPE_RHI_API FDrawVerticesIndirectCount() NOEXCEPT;
    PPE_RHI_API ~FDrawVerticesIndirectCount();

    PPE_RHI_API FDrawVerticesIndirectCount& SetIndirectBuffer(FRawBufferID buffer);
    PPE_RHI_API FDrawVerticesIndirectCount& SetCountBuffer(FRawBufferID buffer);
    PPE_RHI_API FDrawVerticesIndirectCount& Draw(u32 maxDrawCount, u32 indirectBufferOffset = 0, u32 countBufferOffset = 0, u32 indirectBufferStride = sizeof(FIndirectCommand));
};
//----------------------------------------------------------------------------
// FDrawIndexedIndirectCount
//----------------------------------------------------------------------------
struct FDrawIndexedIndirectCount : details::TDrawVerticesDesc<FDrawIndexedIndirectCount> {
    using FDrawCommand = FDrawVerticesIndirectCount::FDrawCommand;
    using FDrawCommands = FDrawVerticesIndirectCount::FDrawCommands;
    using FIndirectCommand = FDrawIndexedIndirect::FIndirectCommand;

    FRawBufferID IndexBuffer;
    u32 IndexBufferOffset{ 0 };
    EIndexFormat IndexFormat{ Default };

    FDrawCommands Commands;
    FRawBufferID IndirectBuffer; // contains array of 'DrawIndirectCommand'
    FRawBufferID CountBuffer; // contains single 'uint' value

    PPE_RHI_API FDrawIndexedIndirectCount() NOEXCEPT;
    PPE_RHI_API ~FDrawIndexedIndirectCount();

    PPE_RHI_API FDrawIndexedIndirectCount& SetIndexBuffer(FRawBufferID buffer, u32 offset = 0, EIndexFormat fmt = Default);
    PPE_RHI_API FDrawIndexedIndirectCount& SetIndirectBuffer(FRawBufferID buffer);
    PPE_RHI_API FDrawIndexedIndirectCount& SetCountBuffer(FRawBufferID buffer);
    PPE_RHI_API FDrawIndexedIndirectCount& Draw(u32 maxDrawCount, u32 indirectBufferOffset = 0, u32 countBufferOffset = 0, u32 indirectBufferStride = sizeof(FIndirectCommand));
};
//----------------------------------------------------------------------------
// FDrawMeshes
//----------------------------------------------------------------------------
struct FDrawMeshes final : details::TDrawCallDesc<FDrawMeshes> {
    struct FDrawCommand {
        u32 TaskCount{ 0 };
        u32 FirstTask{ 0 };
    };

    using FDrawCommands = TFixedSizeStack<FDrawCommand, MaxDrawCommands>;

    FRawMPipelineID Pipeline;
    FDrawCommands Commands;

    PPE_RHI_API FDrawMeshes() NOEXCEPT;
    PPE_RHI_API ~FDrawMeshes();

    PPE_RHI_API FDrawMeshes& SetPipeline(FRawMPipelineID value);
    PPE_RHI_API FDrawMeshes& Draw(u32 meshCount, u32 firstMesh = 0);
};
//----------------------------------------------------------------------------
// FDrawMeshesIndirect
//----------------------------------------------------------------------------
struct FDrawMeshesIndirect final : details::TDrawCallDesc<FDrawMeshesIndirect> {
    using FDrawCommand = FDrawVerticesIndirect::FDrawCommand;
    using FDrawCommands = FDrawVerticesIndirect::FDrawCommands;

    struct FIndirectCommand {
        u32 TaskCount{ 0 };
        u32 FirstTask{ 0 };
    };
    STATIC_ASSERT(sizeof(FIndirectCommand) == 8);

    FRawMPipelineID Pipeline;
    FDrawCommands Commands;
    FRawBufferID IndirectBuffer;

    PPE_RHI_API FDrawMeshesIndirect() NOEXCEPT;
    PPE_RHI_API ~FDrawMeshesIndirect();

    PPE_RHI_API FDrawMeshesIndirect& SetPipeline(FRawMPipelineID value);
    PPE_RHI_API FDrawMeshesIndirect& SetIndirectBuffer(FRawBufferID buffer);
    PPE_RHI_API FDrawMeshesIndirect& Draw(u32 drawCount, u32 indirectBufferOffset = 0, u32 stride = sizeof(FIndirectCommand));
};
//----------------------------------------------------------------------------
// FDrawMeshesIndirectCount
//----------------------------------------------------------------------------
struct FDrawMeshesIndirectCount final : details::TDrawCallDesc<FDrawMeshesIndirectCount> {
    using FDrawCommand = FDrawVerticesIndirectCount::FDrawCommand;
    using FDrawCommands = FDrawVerticesIndirectCount::FDrawCommands;
    using FIndirectCommand =  FDrawMeshesIndirect::FIndirectCommand;

    FRawMPipelineID Pipeline;
    FDrawCommands Commands;
    FRawBufferID IndirectBuffer; // contains array of 'DrawIndirectCommand'
    FRawBufferID CountBuffer; // contains single 'uint' value

    PPE_RHI_API FDrawMeshesIndirectCount() NOEXCEPT;
    PPE_RHI_API ~FDrawMeshesIndirectCount();

    PPE_RHI_API FDrawMeshesIndirectCount& SetPipeline(FRawMPipelineID value);
    PPE_RHI_API FDrawMeshesIndirectCount& SetIndirectBuffer(FRawBufferID buffer);
    PPE_RHI_API FDrawMeshesIndirectCount& SetCountBuffer(FRawBufferID buffer);
    PPE_RHI_API FDrawMeshesIndirectCount& Draw(u32 maxDrawCount, u32 indirectBufferOffset = 0, u32 countBufferOffset = 0, u32 indexBufferStride = sizeof(FIndirectCommand));
};
//----------------------------------------------------------------------------
// FCustomDraw
//----------------------------------------------------------------------------
struct FCustomDraw final : details::TDrawTaskDesc<FCustomDraw> {
    using FCallback = TFunction<void(void*, IDrawContext&)>;
    using FImages = TFixedSizeStack<TPair<FRawImageID, EResourceState>, MaxResourceStates>;
    using FBuffers = TFixedSizeStack<TPair<FRawBufferID, EResourceState>, MaxResourceStates>;

    FCallback Callback;
    void* UserParam{ nullptr };

    FImages Images;
    FBuffers Buffers;

    PPE_RHI_API FCustomDraw() NOEXCEPT;
    PPE_RHI_API explicit FCustomDraw(FCallback&& rcallback, void* userParam = nullptr) NOEXCEPT;
    PPE_RHI_API ~FCustomDraw();

    PPE_RHI_API FCustomDraw& AddImage(FRawImageID image, EResourceState state = EResourceState::ShaderSample);
    PPE_RHI_API FCustomDraw& AddBuffer(FRawBufferID buffer, EResourceState state = EResourceState::ShaderSample);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#include "RHI/DrawTask-inl.h"
