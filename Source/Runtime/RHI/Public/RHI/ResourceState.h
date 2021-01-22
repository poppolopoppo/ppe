#pragma once

#include "RHI_fwd.h"

#include "Meta/Enum.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EResourceState : u32 {
    Unknown = 0,

    // memory access
    _Access_ShaderStorage,              // uniform buffer, storage buffer, image storage
    _Access_Uniform,                    // uniform buffer only
    _Access_ShaderSample,               // texture only
    _Access_InputAttachment,            // same as ShaderRead
    _Access_Transfer,                   // copy buffer/image
    _Access_ColorAttachment,            // color render target
    _Access_DepthStencilAttachment,     // depth/stencil write/test
    _Access_Host,                       // resource mapping
    _Access_Present,                    // image only
    _Access_IndirectBuffer,
    _Access_IndexBuffer,
    _Access_VertexBuffer,
    _Access_ConditionalRendering,
    _Access_CommandProcess,
    _Access_ShadingRateImage,
    _Access_BuildRayTracingAS,          // build/update acceleration structure for ray tracing
    _Access_RTASBuildingBuffer,         // vertex, index, ..., scratch buffer that used when build/update acceleration structure
    _AccessLast,
    _AccessMask                         = (1u << 8) - 1,

    // shader stages
    _VertexShader                       = 1u << 8,
    _TessControlShader                  = 1u << 9,
    _TessEvaluationShader               = 1u << 10,
    _GeometryShader                     = 1u << 11,
    _FragmentShader                     = 1u << 12,
    _ComputeShader                      = 1u << 13,
    _MeshTaskShader                     = 1u << 14,
    _MeshShader                         = 1u << 15,
    _RayTracingShader                   = 1u << 16,  // AnyHitShader, ClosestHitShader, MissShader, and other
    _AllGraphics                        = _VertexShader | _TessControlShader | _TessEvaluationShader |
                                          _GeometryShader | _FragmentShader | _MeshTaskShader | _MeshShader,
    _ShaderMask                         = _AllGraphics | _ComputeShader | _RayTracingShader,

    // flags
    // unused range: 17..24
    _BufferDynamicOffset                = 1u << 25,

    // for ColorAttachment, DepthStencilAttachment
    InvalidateBefore                    = 1u << 26,  // discard image content before executing command
    InvalidateAfter                     = 1u << 27,  // discard image content after command execution

    // for DepthStencilAttachment
    EarlyFragmentTests                  = 1u << 28,
    LateFragmentTests                   = 1u << 29,

    // read/write access
    _Read                               = 1u << 30,
    _Write                              = 1u << 31,
    _ReadWrite                          = _Read | _Write,

    // default states
    _StateMask                          = _AccessMask | _ReadWrite,

    ShaderRead                          = _Access_ShaderStorage | _Read,
    ShaderWrite                         = _Access_ShaderStorage | _Write,
    ShaderReadWrite                     = _Access_ShaderStorage | _ReadWrite,

    UniformRead                         = _Access_Uniform | _Read,

    ShaderSample                        = _Access_ShaderSample | _Read,
    InputAttachment                     = _Access_InputAttachment | _Read,

    TransferSrc                         = _Access_Transfer | _Read,
    TransferDst                         = _Access_Transfer | _Write,

    ColorAttachmentRead                 = _Access_ColorAttachment | _Read,
    ColorAttachmentWrite                = _Access_ColorAttachment | _Write,
    ColorAttachmentReadWrite            = _Access_ColorAttachment | _ReadWrite,

    DepthStencilAttachmentRead          = _Access_DepthStencilAttachment | _Read,
    DepthStencilAttachmentWrite         = _Access_DepthStencilAttachment | _Write,
    DepthStencilAttachmentReadWrite     = _Access_DepthStencilAttachment | _ReadWrite,

    HostRead                            = _Access_Host | _Read,
    HostWrite                           = _Access_Host | _Write,
    HostReadWrite                       = _Access_Host | _ReadWrite,

    PresentImage                        = _Access_Present | _Read,

    IndirectBuffer                      = _Access_IndirectBuffer | _Read,
    IndexBuffer                         = _Access_IndexBuffer | _Read,
    VertexBuffer                        = _Access_VertexBuffer | _Read,

    BuildRayTracingStructRead           = _Access_BuildRayTracingAS | _Read,
    BuildRayTracingStructWrite          = _Access_BuildRayTracingAS | _Write,
    BuildRayTracingStructReadWrite      = _Access_BuildRayTracingAS | _Read | _Write,

    RTASBuildingBufferRead              = _Access_RTASBuildingBuffer | _Read,
    RTASBuildingBufferReadWrite         = _Access_RTASBuildingBuffer | _Read | _Write,

    RayTracingShaderRead                = ShaderRead | _RayTracingShader,

    ShadingRateImageRead                = _Access_ShadingRateImage | _Read,
};
//----------------------------------------------------------------------------
STATIC_ASSERT(u32(EResourceState::_AccessLast) < u32(EResourceState::_AccessMask));
//----------------------------------------------------------------------------
ENUM_FLAGS(EResourceState);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
