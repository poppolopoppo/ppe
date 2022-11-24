// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RHI/DrawTask.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FDrawVertices
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FDrawVertices::FDrawVertices() NOEXCEPT : TDrawVerticesDesc<FDrawVertices>{ "DrawVertices", FDebugColorScheme::Get().Draw } {}
#else
FDrawVertices::FDrawVertices() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FDrawVertices::~FDrawVertices() = default;
//----------------------------------------------------------------------------
FDrawVertices& FDrawVertices::Draw(u32 vertexCount, u32 instanceCount/* = 1 */, u32 firstVertex/* = 0 */, u32 firstInstance/* = 0 */) {
    Assert(vertexCount > 0);
    Emplace_Back(Commands, vertexCount, instanceCount, firstVertex, firstInstance);
    return (*this);
}
//----------------------------------------------------------------------------
// FDrawIndexed
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FDrawIndexed::FDrawIndexed() NOEXCEPT : TDrawVerticesDesc<FDrawIndexed>{ "DrawIndexed", FDebugColorScheme::Get().Draw } {}
#else
FDrawIndexed::FDrawIndexed() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FDrawIndexed::~FDrawIndexed() = default;
//----------------------------------------------------------------------------
FDrawIndexed& FDrawIndexed::SetIndexBuffer(FRawBufferID buffer, u32 offset/* = 0 */, EIndexFormat fmt/* = Default */) {
    Assert(buffer);
    IndexBuffer = buffer;
    IndexBufferOffset = offset;
    IndexFormat = fmt;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawIndexed& FDrawIndexed::Draw(u32 indexCount, u32 instanceCount/* = 1 */, u32 firstIndex/* = 0 */, i32 vertexOffset/* = 0 */, u32 firstInstance/* = 0 */) {
    Assert(indexCount > 0);
    Emplace_Back(Commands, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    return (*this);
}
//----------------------------------------------------------------------------
// FDrawVerticesIndirect
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FDrawVerticesIndirect::FDrawVerticesIndirect() NOEXCEPT : TDrawVerticesDesc<FDrawVerticesIndirect>{ "DrawVerticesIndirect", FDebugColorScheme::Get().Draw } {}
#else
FDrawVerticesIndirect::FDrawVerticesIndirect() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FDrawVerticesIndirect::~FDrawVerticesIndirect() = default;
//----------------------------------------------------------------------------
FDrawVerticesIndirect& FDrawVerticesIndirect::SetIndirectBuffer(FRawBufferID buffer) {
    Assert(buffer);
    IndirectBuffer = buffer;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawVerticesIndirect& FDrawVerticesIndirect::Draw(u32 drawCount, u32 indirectBufferOffset/* = 0 */, u32 indirectBufferStride/* = sizeof(FIndirectCommand) */) {
    Assert(drawCount > 0);
    Emplace_Back(Commands, indirectBufferOffset, drawCount, indirectBufferStride);
    return (*this);
}
//----------------------------------------------------------------------------
// FDrawIndexedIndirect
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FDrawIndexedIndirect::FDrawIndexedIndirect() NOEXCEPT : TDrawVerticesDesc<FDrawIndexedIndirect>{ "DrawIndexedIndirect", FDebugColorScheme::Get().Draw } {}
#else
FDrawIndexedIndirect::FDrawIndexedIndirect() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FDrawIndexedIndirect::~FDrawIndexedIndirect() = default;
//----------------------------------------------------------------------------
FDrawIndexedIndirect& FDrawIndexedIndirect::SetIndexBuffer(FRawBufferID buffer, u32 offset/* = 0 */, EIndexFormat fmt/* = Default */) {
    Assert(buffer);
    IndexBuffer = buffer;
    IndexBufferOffset = offset;
    IndexFormat = fmt;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawIndexedIndirect& FDrawIndexedIndirect::SetIndirectBuffer(FRawBufferID buffer) {
    Assert(buffer);
    IndirectBuffer = buffer;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawIndexedIndirect& FDrawIndexedIndirect::Draw(u32 drawCount, u32 indirectBufferOffset/* = 0 */, u32 stride/* = sizeof(FIndirectCommand) */) {
    Assert(drawCount > 0);
    Emplace_Back(Commands, indirectBufferOffset, drawCount, stride);
    return (*this);
}
//----------------------------------------------------------------------------
// FDrawVerticesIndirectCount
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FDrawVerticesIndirectCount::FDrawVerticesIndirectCount() NOEXCEPT : TDrawVerticesDesc<FDrawVerticesIndirectCount>{ "DrawVerticesIndirectCount", FDebugColorScheme::Get().Draw } {}
#else
FDrawVerticesIndirectCount::FDrawVerticesIndirectCount() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FDrawVerticesIndirectCount::~FDrawVerticesIndirectCount() = default;
//----------------------------------------------------------------------------
FDrawVerticesIndirectCount& FDrawVerticesIndirectCount::SetIndirectBuffer(FRawBufferID buffer) {
    Assert(buffer);
    IndirectBuffer = buffer;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawVerticesIndirectCount& FDrawVerticesIndirectCount::SetCountBuffer(FRawBufferID buffer) {
    Assert(buffer);
    CountBuffer = buffer;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawVerticesIndirectCount& FDrawVerticesIndirectCount::Draw(u32 maxDrawCount, u32 indirectBufferOffset/* = 0 */, u32 countBufferOffset/* = 0 */, u32 indirectBufferStride/* = sizeof(FIndirectCommand) */) {
    Assert(maxDrawCount > 0);
    Emplace_Back(Commands, indirectBufferOffset, countBufferOffset, maxDrawCount, indirectBufferStride);
    return (*this);
}
//----------------------------------------------------------------------------
// FDrawIndexedIndirectCount
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FDrawIndexedIndirectCount::FDrawIndexedIndirectCount() NOEXCEPT : TDrawVerticesDesc<FDrawIndexedIndirectCount>{ "DrawIndexedIndirectCount", FDebugColorScheme::Get().Draw } {}
#else
FDrawIndexedIndirectCount::FDrawIndexedIndirectCount() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FDrawIndexedIndirectCount::~FDrawIndexedIndirectCount() = default;
//----------------------------------------------------------------------------
FDrawIndexedIndirectCount& FDrawIndexedIndirectCount::SetIndexBuffer(FRawBufferID buffer, u32 offset/* = 0 */, EIndexFormat fmt/* = Default */) {
    Assert(buffer);
    IndexBuffer = buffer;
    IndexBufferOffset = offset;
    IndexFormat = fmt;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawIndexedIndirectCount& FDrawIndexedIndirectCount::SetIndirectBuffer(FRawBufferID buffer) {
    Assert(buffer);
    IndirectBuffer = buffer;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawIndexedIndirectCount& FDrawIndexedIndirectCount::SetCountBuffer(FRawBufferID buffer) {
    Assert(buffer);
    CountBuffer = buffer;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawIndexedIndirectCount& FDrawIndexedIndirectCount::Draw(u32 maxDrawCount, u32 indirectBufferOffset/* = 0 */, u32 countBufferOffset/* = 0 */, u32 indirectBufferStride/* = sizeof(FIndirectCommand) */) {
    Assert(maxDrawCount > 0);
    Emplace_Back(Commands, indirectBufferOffset, countBufferOffset, maxDrawCount, indirectBufferStride);
    return (*this);
}
//----------------------------------------------------------------------------
// FDrawMeshes
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FDrawMeshes::FDrawMeshes() NOEXCEPT : TDrawCallDesc<FDrawMeshes>{ "DrawMeshes", FDebugColorScheme::Get().DrawMeshes } {}
#else
FDrawMeshes::FDrawMeshes() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FDrawMeshes::~FDrawMeshes() = default;
//----------------------------------------------------------------------------
FDrawMeshes& FDrawMeshes::SetPipeline(FRawMPipelineID value) {
    Assert(value);
    Pipeline = value;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawMeshes& FDrawMeshes::Draw(u32 meshCount, u32 firstMesh/* = 0 */) {
    Assert(meshCount > 0);
    Emplace_Back(Commands, meshCount, firstMesh);
    return (*this);
}
//----------------------------------------------------------------------------
// FDrawMeshesIndirect
//----------------------------------------------------------------------------

#if USE_PPE_RHITASKNAME
FDrawMeshesIndirect::FDrawMeshesIndirect() NOEXCEPT : TDrawCallDesc<FDrawMeshesIndirect>{ "DrawMeshesIndirect", FDebugColorScheme::Get().DrawMeshes } {}
#else
FDrawMeshesIndirect::FDrawMeshesIndirect() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FDrawMeshesIndirect::~FDrawMeshesIndirect() = default;
//----------------------------------------------------------------------------
FDrawMeshesIndirect& FDrawMeshesIndirect::SetPipeline(FRawMPipelineID value) {
    Assert(value);
    Pipeline = value;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawMeshesIndirect& FDrawMeshesIndirect::SetIndirectBuffer(FRawBufferID buffer) {
    Assert(buffer);
    IndirectBuffer = buffer;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawMeshesIndirect& FDrawMeshesIndirect::Draw(u32 drawCount, u32 indirectBufferOffset/* = 0 */, u32 stride/* = sizeof(FIndirectCommand) */) {
    Assert(drawCount > 0);
    Emplace_Back(Commands, drawCount, indirectBufferOffset, stride);
    return (*this);
}
//----------------------------------------------------------------------------
// FDrawMeshesIndirectCount
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FDrawMeshesIndirectCount::FDrawMeshesIndirectCount() NOEXCEPT : TDrawCallDesc<FDrawMeshesIndirectCount>{ "DrawMeshesIndirectCount", FDebugColorScheme::Get().DrawMeshes } {}
#else
FDrawMeshesIndirectCount::FDrawMeshesIndirectCount() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FDrawMeshesIndirectCount::~FDrawMeshesIndirectCount() = default;
//----------------------------------------------------------------------------
FDrawMeshesIndirectCount& FDrawMeshesIndirectCount::SetPipeline(FRawMPipelineID value) {
    Assert(value);
    Pipeline = value;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawMeshesIndirectCount& FDrawMeshesIndirectCount::SetIndirectBuffer(FRawBufferID buffer) {
    Assert(buffer);
    IndirectBuffer = buffer;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawMeshesIndirectCount& FDrawMeshesIndirectCount::SetCountBuffer(FRawBufferID buffer) {
    Assert(buffer);
    CountBuffer = buffer;
    return (*this);
}
//----------------------------------------------------------------------------
FDrawMeshesIndirectCount& FDrawMeshesIndirectCount::Draw(u32 maxDrawCount, u32 indirectBufferOffset/* = 0 */, u32 countBufferOffset/* = 0 */, u32 indexBufferStride/* = sizeof(FIndirectCommand) */) {
    Assert(maxDrawCount > 0);
    Emplace_Back(Commands, indirectBufferOffset, countBufferOffset, maxDrawCount, indexBufferStride);
    return (*this);
}
//----------------------------------------------------------------------------
// FCustomDraw
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FCustomDraw::FCustomDraw() NOEXCEPT : TDrawTaskDesc<FCustomDraw>{ "CustomDraw", FDebugColorScheme::Get().CustomDraw } {}
#else
FCustomDraw::FCustomDraw() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FCustomDraw::~FCustomDraw() = default;
//----------------------------------------------------------------------------
FCustomDraw::FCustomDraw(FCallback&& rcallback, void* userParam/* = nullptr */) NOEXCEPT : FCustomDraw() {
    Callback = std::move(rcallback);
    UserParam = userParam;
}
//----------------------------------------------------------------------------
FCustomDraw& FCustomDraw::AddImage(FRawImageID image, EResourceState state/* = EResourceState::ShaderSample */) {
    Images.Push(image, state);
    return (*this);
}
//----------------------------------------------------------------------------
FCustomDraw& FCustomDraw::AddBuffer(FRawBufferID buffer, EResourceState state/* = EResourceState::ShaderSample */) {
    Buffers.Push(buffer, state);
    return (*this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
