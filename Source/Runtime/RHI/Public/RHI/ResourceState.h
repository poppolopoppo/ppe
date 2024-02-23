#pragma once

#include "RHI_fwd.h"

#include "Meta/Enum.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EResourceAccess : u32 {
    Unknown = 0,

    // memory access
    ShaderStorage,                      // uniform buffer, storage buffer, image storage
    Uniform,                            // uniform buffer only
    ShaderSample,                       // texture only
    InputAttachment,                    // same as ShaderRead
    Transfer,                           // copy buffer/image
    ColorAttachment,                    // color render target
    DepthStencilAttachment,             // depth/stencil write/test
    Host,                               // resource mapping
    Present,                            // image only
    IndirectBuffer,
    IndexBuffer,
    VertexBuffer,
    ConditionalRendering,
    CommandProcess,
    ShadingRateImage,
    BuildRayTracingAS,                  // build/update acceleration structure for ray tracing
    RTASBuildingBuffer,                 // vertex, index, ..., scratch buffer that used when build/update acceleration structure

    AccessLast,
};
//----------------------------------------------------------------------------
enum class EResourceShaderStages : u32 {
    Unknown = 0,

    // shader stages
    VertexShader                       = 1u << 0,
    TessControlShader                  = 1u << 1,
    TessEvaluationShader               = 1u << 2,
    GeometryShader                     = 1u << 3,
    FragmentShader                     = 1u << 4,
    ComputeShader                      = 1u << 5,
    MeshTaskShader                     = 1u << 6,
    MeshShader                         = 1u << 7,
    RayTracingShader                   = 1u << 8,  // AnyHitShader, ClosestHitShader, MissShader, and other
};
ENUM_FLAGS(EResourceShaderStages);
//----------------------------------------------------------------------------
enum class EResourceFlags : u32 {
    Unknown = 0,

    // flags
    BufferDynamicOffset                = 1u << 0,

    // for ColorAttachment, DepthStencilAttachment
    InvalidateBefore                   = 1u << 1,  // discard image content before executing command
    InvalidateAfter                    = 1u << 2,  // discard image content after command execution

    // for DepthStencilAttachment
    EarlyFragmentTests                 = 1u << 3,
    LateFragmentTests                  = 1u << 4,

    // read/write access
    Read                               = 1u << 5,
    Write                              = 1u << 6,
};
ENUM_FLAGS(EResourceFlags);
//----------------------------------------------------------------------------
inline CONSTEXPR EResourceFlags EResourceFlags_ReadWrite{
    EResourceFlags::Read |
    EResourceFlags::Write };
//----------------------------------------------------------------------------
inline CONSTEXPR EResourceShaderStages EResourceFlags_AllGraphics{
    EResourceShaderStages::VertexShader |
    EResourceShaderStages::TessControlShader |
    EResourceShaderStages::TessEvaluationShader |
    EResourceShaderStages::GeometryShader |
    EResourceShaderStages::FragmentShader |
    EResourceShaderStages::MeshTaskShader |
    EResourceShaderStages::MeshShader };
//----------------------------------------------------------------------------
struct EResourceState {
    EResourceFlags          Flags           : 15;
    EResourceAccess         MemoryAccess    : 8;
    EResourceShaderStages   ShaderStages    : 9;

    CONSTEXPR u32 Ord() const {
        return ((u32(Flags) << 0u) |
                (u32(MemoryAccess) << 15u) |
                (u32(ShaderStages) << 23u));
    }

    CONSTEXPR EResourceState() : EResourceState(Default, Default, Default)
    {}

    CONSTEXPR EResourceState(EResourceFlags flags, EResourceAccess memoryAccess, EResourceShaderStages shaderStages) {
        Flags = flags;
        MemoryAccess = memoryAccess;
        ShaderStages = shaderStages;
    }

    CONSTEXPR EResourceState OnlyState() const {
        return { Meta::EnumAnd(Flags, EResourceFlags_ReadWrite), MemoryAccess, Default };
    }

    friend hash_t hash_value(EResourceState state) NOEXCEPT {
        return hash_as_pod(state);
    }

    CONSTEXPR EResourceState operator +(EResourceFlags flag) const { return {Flags + flag, MemoryAccess, ShaderStages }; }
    CONSTEXPR EResourceState operator -(EResourceFlags flag) const { return {Flags - flag, MemoryAccess, ShaderStages }; }
    CONSTEXPR EResourceState operator |(EResourceFlags flag) const { return {Flags | flag, MemoryAccess, ShaderStages }; }

    CONSTEXPR EResourceState& operator +=(EResourceFlags flag) { Flags = (Flags + flag); return (*this); }
    CONSTEXPR EResourceState& operator -=(EResourceFlags flag) { Flags = (Flags - flag); return (*this); }
    CONSTEXPR EResourceState& operator |=(EResourceFlags flag) { Flags = (Flags | flag); return (*this); }

    CONSTEXPR bool operator &(EResourceFlags flag) const { return (Flags & flag); }
    CONSTEXPR bool operator ^(EResourceFlags flag) const { return (Flags ^ flag); }

    CONSTEXPR EResourceState operator +(EResourceShaderStages stage) const { return {Flags, MemoryAccess, ShaderStages + stage}; }
    CONSTEXPR EResourceState operator -(EResourceShaderStages stage) const { return {Flags, MemoryAccess, ShaderStages - stage}; }
    CONSTEXPR EResourceState operator |(EResourceShaderStages stage) const { return {Flags, MemoryAccess, ShaderStages | stage}; }

    CONSTEXPR EResourceState& operator +=(EResourceShaderStages stage) { ShaderStages = (ShaderStages + stage); return (*this); }
    CONSTEXPR EResourceState& operator -=(EResourceShaderStages stage) { ShaderStages = (ShaderStages - stage); return (*this); }
    CONSTEXPR EResourceState& operator |=(EResourceShaderStages stage) { ShaderStages = (ShaderStages | stage); return (*this); }

    CONSTEXPR bool operator &(EResourceShaderStages stage) const { return (ShaderStages & stage); }
    CONSTEXPR bool operator ^(EResourceShaderStages stage) const { return (ShaderStages ^ stage); }

    CONSTEXPR bool operator ==(EResourceState o) const { return (Flags == o.Flags && MemoryAccess == o.MemoryAccess && ShaderStages == o.ShaderStages); }
    CONSTEXPR bool operator !=(EResourceState o) const { return (not operator ==(o)); }

    CONSTEXPR bool operator &(EResourceState o) const { return ((Flags & o.Flags) && (ShaderStages & o.ShaderStages) && (MemoryAccess == o.MemoryAccess)); }
    CONSTEXPR bool operator ^(EResourceState o) const { return ((Flags ^ o.Flags) && (ShaderStages ^ o.ShaderStages) && (MemoryAccess == o.MemoryAccess)); }

};
PPE_ASSUME_TYPE_AS_POD(EResourceState);
STATIC_ASSERT(Meta::TCheckSameSize<EResourceState, u32>::value);
//----------------------------------------------------------------------------
inline CONSTEXPR EResourceState EResourceState_Unknown                             { Default, Default, Default };
inline CONSTEXPR EResourceState EResourceState_ShaderRead                          { EResourceFlags::Read, EResourceAccess::ShaderStorage, Default };
inline CONSTEXPR EResourceState EResourceState_ShaderWrite                         { EResourceFlags::Write, EResourceAccess::ShaderStorage, Default };
inline CONSTEXPR EResourceState EResourceState_ShaderReadWrite                     { EResourceFlags_ReadWrite, EResourceAccess::ShaderStorage, Default };
inline CONSTEXPR EResourceState EResourceState_UniformRead                         { EResourceFlags::Read, EResourceAccess::Uniform, Default };
inline CONSTEXPR EResourceState EResourceState_ShaderSample                        { EResourceFlags::Read, EResourceAccess::ShaderSample, Default };
inline CONSTEXPR EResourceState EResourceState_InputAttachment                     { EResourceFlags::Read, EResourceAccess::InputAttachment, Default };
inline CONSTEXPR EResourceState EResourceState_TransferSrc                         { EResourceFlags::Read, EResourceAccess::Transfer, Default };
inline CONSTEXPR EResourceState EResourceState_TransferDst                         { EResourceFlags::Write, EResourceAccess::Transfer, Default };
inline CONSTEXPR EResourceState EResourceState_ColorAttachmentRead                 { EResourceFlags::Read, EResourceAccess::ColorAttachment, Default };
inline CONSTEXPR EResourceState EResourceState_ColorAttachmentWrite                { EResourceFlags::Write, EResourceAccess::ColorAttachment, Default };
inline CONSTEXPR EResourceState EResourceState_ColorAttachmentReadWrite            { EResourceFlags_ReadWrite, EResourceAccess::ColorAttachment, Default };
inline CONSTEXPR EResourceState EResourceState_DepthStencilAttachmentRead          { EResourceFlags::Read, EResourceAccess::DepthStencilAttachment, Default };
inline CONSTEXPR EResourceState EResourceState_DepthStencilAttachmentWrite         { EResourceFlags::Write, EResourceAccess::DepthStencilAttachment, Default };
inline CONSTEXPR EResourceState EResourceState_DepthStencilAttachmentReadWrite     { EResourceFlags_ReadWrite, EResourceAccess::DepthStencilAttachment, Default };
inline CONSTEXPR EResourceState EResourceState_HostRead                            { EResourceFlags::Read, EResourceAccess::Host, Default };
inline CONSTEXPR EResourceState EResourceState_HostWrite                           { EResourceFlags::Write, EResourceAccess::Host, Default };
inline CONSTEXPR EResourceState EResourceState_HostReadWrite                       { EResourceFlags_ReadWrite, EResourceAccess::Host, Default };
inline CONSTEXPR EResourceState EResourceState_PresentImage                        { EResourceFlags::Read, EResourceAccess::Present, Default };
inline CONSTEXPR EResourceState EResourceState_IndirectBuffer                      { EResourceFlags::Read, EResourceAccess::IndirectBuffer, Default };
inline CONSTEXPR EResourceState EResourceState_IndexBuffer                         { EResourceFlags::Read, EResourceAccess::IndexBuffer, Default };
inline CONSTEXPR EResourceState EResourceState_VertexBuffer                        { EResourceFlags::Read, EResourceAccess::VertexBuffer, Default };
inline CONSTEXPR EResourceState EResourceState_BuildRayTracingStructRead           { EResourceFlags::Read, EResourceAccess::BuildRayTracingAS, Default };
inline CONSTEXPR EResourceState EResourceState_BuildRayTracingStructWrite          { EResourceFlags::Write, EResourceAccess::BuildRayTracingAS, Default };
inline CONSTEXPR EResourceState EResourceState_BuildRayTracingStructReadWrite      { EResourceFlags_ReadWrite, EResourceAccess::BuildRayTracingAS, Default };
inline CONSTEXPR EResourceState EResourceState_RTASBuildingBufferRead              { EResourceFlags::Read, EResourceAccess::RTASBuildingBuffer, Default };
inline CONSTEXPR EResourceState EResourceState_RTASBuildingBufferReadWrite         { EResourceFlags_ReadWrite, EResourceAccess::RTASBuildingBuffer, Default };
inline CONSTEXPR EResourceState EResourceState_ShadingRateImageRead                { EResourceFlags::Read, EResourceAccess::ShadingRateImage, Default };
inline CONSTEXPR EResourceState EResourceState_RayTracingShaderRead                { EResourceFlags::Read, EResourceAccess::ShaderStorage, EResourceShaderStages::RayTracingShader };
inline CONSTEXPR EResourceState EResourceState_RayTracingShaderStorage             { Default, EResourceAccess::ShaderStorage, EResourceShaderStages::RayTracingShader };
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
