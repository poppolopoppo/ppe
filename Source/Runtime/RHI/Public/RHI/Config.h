#pragma once

#include "RHI_fwd.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIMOBILE
inline CONSTEXPR u32   MaxUniforms                        =   8;
inline CONSTEXPR u32   MaxVertexBuffers                   =   8;
inline CONSTEXPR u32   MaxVertexAttribs                   =  16;
inline CONSTEXPR u32   MaxColorBuffers                    =   4;
inline CONSTEXPR u32   MaxViewports                       =   1;
inline CONSTEXPR u32   MaxDescriptorSets                  =   4;
inline CONSTEXPR u32   MaxBufferDynamicOffsets            =  12; // 8 UBO + 4 SSBO
inline CONSTEXPR u32   MaxElementsInUnsizedDesc           =   1; // if used extension GL_EXT_nonuniform_qualifier
inline CONSTEXPR u32   GPUPageSizeInMb                    =  64;
#else
inline CONSTEXPR u32   MaxUniforms                        =   8;
inline CONSTEXPR u32   MaxVertexBuffers                   =   8;
inline CONSTEXPR u32   MaxVertexAttribs                   =  16;
inline CONSTEXPR u32   MaxColorBuffers                    =   8;
inline CONSTEXPR u32   MaxViewports                       =  16;
inline CONSTEXPR u32   MaxDescriptorSets                  =   8;
inline CONSTEXPR u32   MaxBufferDynamicOffsets            =  16;
inline CONSTEXPR u32   MaxElementsInUnsizedDesc           =  64; // if used extension GL_EXT_nonuniform_qualifier
inline CONSTEXPR u32   GPUPageSizeInMb                    = 256;
#endif
//----------------------------------------------------------------------------
inline CONSTEXPR u32   MaxRenderPassSubpasses             =   8;
inline CONSTEXPR u32   MaxPushConstantsCount              =   8;
inline CONSTEXPR u32   MaxPushConstantsSize               = 128; // bytes
inline CONSTEXPR u32   MaxSpecializationConstants         =   8;
inline CONSTEXPR u32   MaxQueueFamilies                   =  32;
//----------------------------------------------------------------------------
inline CONSTEXPR u32   MaxComputeCommands                 =  16;
inline CONSTEXPR u32   MaxDrawCommands                    =  16;
inline CONSTEXPR u32   MaxResourceStates                  =   8;
//----------------------------------------------------------------------------
inline CONSTEXPR u32   MaxTaskDependencies                =   8;
inline CONSTEXPR u32   MaxCopyRegions                     =   8;
inline CONSTEXPR u32   MaxClearRanges                     =   8;
inline CONSTEXPR u32   MaxBlitRegions                     =   8;
inline CONSTEXPR u32   MaxResolveRegions                  =   8;
//----------------------------------------------------------------------------
inline CONSTEXPR bool  EnableShaderDebugging              = USE_PPE_RHIDEBUG;
#if USE_PPE_RHIDEBUG
inline CONSTEXPR u32   DebugDescriptorSet                 = MaxDescriptorSets - 1;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
