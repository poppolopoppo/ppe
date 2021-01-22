#pragma once

#include "ResourceTypes.h"
#include "ShaderEnums.h"
#include "RHI/DrawTask-inl.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _Task>
_Task& TDrawCallDesc<_Task>::AddResources(const FDescriptorSetID& id, const FPipelineResources* res) {
    Assert(id);
    Assert(res);
    Resources.Add_Overwrite(id, res);
    return static_cast<_Task&>(*this);
}
//----------------------------------------------------------------------------
template <typename _Task>
_Task& TDrawCallDesc<_Task>::AddScissor(const FRectangleI& clip) {
    Assert(clip.HasPositiveExtents());
    Emplace_Back(Scissors, clip);
    return static_cast<_Task&>(*this);
}
//----------------------------------------------------------------------------
template <typename _Task>
_Task& TDrawCallDesc<_Task>::AddColorBuffer(ERenderTargetID id, const FColorBufferState& cb) {
    ColorBuffers.Add_Overwrite(id, cb);
    return static_cast<_Task&>(*this);
}
//----------------------------------------------------------------------------
template <typename _Task>
_Task& TDrawCallDesc<_Task>::AddColorBuffer(ERenderTargetID id, EColorMask colorMask/* = EColorMask::All */) {
    FColorBufferState cb;
    cb.ColorMask = colorMask;
    ColorBuffers.Add_Overwrite(id, cb);
    return static_cast<_Task&>(*this);
}
//----------------------------------------------------------------------------
template <typename _Task>
_Task& TDrawCallDesc<_Task>::AddColorBuffer(ERenderTargetID id, EBlendFactor srcBlend, EBlendFactor dstBlend, EBlendOp blendOp, EColorMask colorMask/* = EColorMask::All */) {
    return AddColorBuffer(id,
        srcBlend, srcBlend,
        dstBlend, dstBlend,
        blendOp, blendOp,
        colorMask );
}
//----------------------------------------------------------------------------
template <typename _Task>
_Task& TDrawCallDesc<_Task>::AddColorBuffer(ERenderTargetID id,
    EBlendFactor srcBlendColor, EBlendFactor srcBlendAlpha,
    EBlendFactor dstBlendColor, EBlendFactor dstBlendAlpha,
    EBlendOp blendOpColor, EBlendOp blendOpAlpha,
    EColorMask colorMask/* = EColorMask::All */) {
    FColorBufferState cb;
    cb.EnableAlphaBlending = true;
    cb.SrcBlendFactor =  { srcBlendColor, srcBlendAlpha };
    cb.DstBlendFactor =  { dstBlendColor, dstBlendAlpha };
    cb.BlendOp = { blendOpColor, blendOpAlpha };
    cb.ColorMask = colorMask;
    ColorBuffers.Add_Overwrite(id, cb);
    return static_cast<_Task&>(*this);
}
//----------------------------------------------------------------------------
template <typename _Task>
_Task& TDrawCallDesc<_Task>::AddPushConstant(const FPushConstantID& id, const void* p, size_t size) {
    PushConstants.Push(id, p, size);
    return static_cast<_Task&>(*this);
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
template <typename _Task>
_Task& TDrawCallDesc<_Task>::EnableShaderDebugTrace(EShaderStages stages) {
    DebugMode.Mode = EShaderDebugMode::Trace;
    DebugMode.Stages = DebugMode.Stages | stages;
    return static_cast<_Task&>(*this);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
template <typename _Task>
_Task& TDrawCallDesc<_Task>::EnableFragmentDebugTrace(int x, int y) {
    DebugMode.Mode = EShaderDebugMode::Trace;
    DebugMode.Stages = DebugMode.Stages | EShaderStages::Fragment;
    DebugMode.FragCoord = { checked_cast<i16>(x), checked_cast<i16>(y) };
    return static_cast<_Task&>(*this);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIPROFILING
template <typename _Task>
_Task& TDrawCallDesc<_Task>::EnableShaderProfiling(EShaderStages stages) {
    DebugMode.Mode = EShaderDebugMode::Profiling;
    DebugMode.Stages = DebugMode.Stages | stages;
    return static_cast<_Task&>(*this);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIPROFILING
template <typename _Task>
_Task& TDrawCallDesc<_Task>::EnableFragmentProfiling(int x, int y) {
    DebugMode.Mode = EShaderDebugMode::Profiling;
    DebugMode.Stages = DebugMode.Stages | EShaderStages::Fragment;
    DebugMode.FragCoord = { checked_cast<i16>(x), checked_cast<i16>(y) };
    return static_cast<_Task&>(*this);
}
#endif
//----------------------------------------------------------------------------
}  // !namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
