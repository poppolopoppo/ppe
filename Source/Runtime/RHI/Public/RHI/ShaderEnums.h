#pragma once

#include "RHI_fwd.h"

#include "Meta/Enum.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EShaderType : u32 {
    Vertex,
    TessControl,
    TessEvaluation,
    Geometry,
    Fragment,
    Compute,

    MeshTask,
    Mesh,

    RayGen,
    RayAnyHit,
    RayClosestHit,
    RayMiss,
    RayIntersection,
    RayCallable,

    _Count,
    Unknown		= ~0u,
};
//----------------------------------------------------------------------------
enum class EShaderStages : u32 {
    Vertex          = 1 << u32(EShaderType::Vertex),
    TessControl     = 1 << u32(EShaderType::TessControl),
    TessEvaluation  = 1 << u32(EShaderType::TessEvaluation),
    Geometry        = 1 << u32(EShaderType::Geometry),
    Fragment        = 1 << u32(EShaderType::Fragment),
    Compute         = 1 << u32(EShaderType::Compute),
    MeshTask        = 1 << u32(EShaderType::MeshTask),
    Mesh            = 1 << u32(EShaderType::Mesh),
    RayGen          = 1 << u32(EShaderType::RayGen),
    RayAnyHit       = 1 << u32(EShaderType::RayAnyHit),
    RayClosestHit   = 1 << u32(EShaderType::RayClosestHit),
    RayMiss         = 1 << u32(EShaderType::RayMiss),
    RayIntersection = 1 << u32(EShaderType::RayIntersection),
    RayCallable     = 1 << u32(EShaderType::RayCallable),
    _Last,

    All             = ((_Last-1) << 1) - 1,
    AllGraphics     = Vertex | TessControl | TessEvaluation | Geometry | MeshTask | Mesh | Fragment,
    AllRayTracing   = RayGen | RayAnyHit | RayClosestHit | RayMiss | RayIntersection | RayCallable,
    Unknown         = 0,
};
ENUM_FLAGS(EShaderStages);
//----------------------------------------------------------------------------
enum class EShaderAccess : u32 {
    ReadOnly,
    WriteOnly,
    WriteDiscard, // (optimization) same as WriteOnly, but previous data will be discarded
    ReadWrite,
};
//----------------------------------------------------------------------------
enum class EShaderLangFormat : u32 {
    // api
    _ApiOffset          = 0,
    _ApiMask            = 0x7 << _ApiOffset,
    Vulkan              = 1 << _ApiOffset,
    OpenGL              = 2 << _ApiOffset,
    OpenGLES            = 3 << _ApiOffset,
    DirectX             = 4 << _ApiOffset,
    // reserved 5..7

    // version
    _VersionOffset      = 3,
    _VersionMask        = 0x3FF << _VersionOffset,
    Vulkan_100          = (100 << _VersionOffset) | Vulkan,
    Vulkan_110          = (110 << _VersionOffset) | Vulkan,
    Vulkan_120          = (120 << _VersionOffset) | Vulkan,
    OpenGL_450          = (450 << _VersionOffset) | OpenGL,
    OpenGL_460          = (460 << _VersionOffset) | OpenGL,
    OpenGLES_200        = (200 << _VersionOffset) | OpenGLES,
    OpenGLES_320        = (320 << _VersionOffset) | OpenGLES,
    DirectX_11          = (110 << _VersionOffset) | DirectX,
    DirectX_12          = (120 << _VersionOffset) | DirectX,

    // storage
    _StorageOffset      = 13,
    _StorageMask        = 0x3 << _StorageOffset,
    Source              = 1 << _StorageOffset,
    Binary              = 2 << _StorageOffset,                  // compiled program (HLSL bytecode, SPIRV)
    Executable          = 3 << _StorageOffset,                  // compiled program (exe, dll, so, ...)

    // format
    _FormatOffset       = 15,
    _FormatMask         = 0x7 << _FormatOffset,
    HighLevel           = (1 << _FormatOffset) | Source,        // GLSL, HLSL, CL
    SPIRV               = (2 << _FormatOffset) | Binary,
    ShaderModule        = (3 << _FormatOffset) | Executable,    // vkShaderModule, GLuint, ...
    // reserved 4..7

    // independent flags
    _FlagsOffset        = 18,
    _FlagsMask          = 0xFF << _FlagsOffset,
    EnableDebugTrace    = 1 << (_FlagsOffset + 0),  // writes trace only for selected shader invocation (may be very slow)
    EnableProfiling     = 1 << (_FlagsOffset + 1),  // writes shader function execution time for selected invocation.
    EnableTimeMap       = 1 << (_FlagsOffset + 2),  // writes summarized shader invocation times per pixel.
    _DebugModeMask      = EnableDebugTrace | EnableProfiling | EnableTimeMap,

    //HasInputAttachment= 1 << (_FlagsOffset + 3),	// if shader contains input attachment then may be generated 2 shaders:
                                                    // 1. keep input attachments and add flag 'HasInputAttachment'.
                                                    // 2. replaces attachments by sampler2D and texelFetch function.

    // available bits: 26..31

    Unknown             = 0,

    // combined masks
    _StorageFormatMask      = _StorageMask | _FormatMask,
    _ApiStorageFormatMask   = _ApiMask | _StorageMask | _FormatMask,
    _ApiVersionMask         = _ApiMask | _VersionMask,
    _VersionModeFlagsMask   = _VersionMask | _FlagsMask,

    // default
    GLSL_450        = OpenGL_450 | HighLevel,
    GLSL_460        = OpenGL_460 | HighLevel,
    HLSL_11         = DirectX_11 | HighLevel,
    HLSL_12         = DirectX_12 | HighLevel,
    VKSL_100        = Vulkan_100 | HighLevel,
    VKSL_110        = Vulkan_110 | HighLevel,
    VKSL_120        = Vulkan_120 | HighLevel,
    SPIRV_100       = Vulkan_100 | SPIRV,
    SPIRV_110       = Vulkan_110 | SPIRV,           // SPIRV 1.3
    SPIRV_120       = Vulkan_120 | SPIRV,           // SPIRV 1.4
    VkShader_100    = Vulkan_100 | ShaderModule,
    VkShader_110    = Vulkan_110 | ShaderModule,
    VkShader_120    = Vulkan_120 | ShaderModule,
};
ENUM_FLAGS(EShaderLangFormat);
//----------------------------------------------------------------------------
enum class EShaderDebugMode : u32 {
    None            = 0,
#if USE_PPE_RHIDEBUG
    Trace,
    Profiling,
    Timemap,
    //Asserts,
    //View,
    //InstructionCounter,
#endif
    _Count,
    Unknown	        = None,
};
//----------------------------------------------------------------------------
inline CONSTEXPR bool operator ==(FPackedDebugMode lhs, FPackedDebugMode rhs) {
    return (lhs.Stages == rhs.Stages && lhs.Mode == rhs.Mode);
}
inline CONSTEXPR bool operator !=(FPackedDebugMode lhs, FPackedDebugMode rhs) {
    return (not operator ==(lhs, rhs));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
