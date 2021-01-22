#pragma once

#include "RHI_fwd.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIMOBILE
CONSTEXPR u32   MaxVertexBuffers                            =   8;
CONSTEXPR u32   MaxVertexAttribs                            =  16;
CONSTEXPR u32   MaxColorBuffers                             =   4;
CONSTEXPR u32   MaxViewports                                =   1;
CONSTEXPR u32   MaxDescriptorSets                           =   4;
CONSTEXPR u32   MaxBufferDynamicOffsets                     =  12; // 8 UBO + 4 SSBO
CONSTEXPR u32   MaxElementsInUnsizedDesc                    =   1; // if used extension GL_EXT_nonuniform_qualifier
CONSTEXPR u32   GPUPageSizeInMb                             =  64;
#else
CONSTEXPR u32   MaxVertexBuffers                            =   8;
CONSTEXPR u32   MaxVertexAttribs                            =  16;
CONSTEXPR u32   MaxColorBuffers                             =   8;
CONSTEXPR u32   MaxViewports                                =  16;
CONSTEXPR u32   MaxDescriptorSets                           =   8;
CONSTEXPR u32   MaxBufferDynamicOffsets                     =  16;
CONSTEXPR u32   MaxElementsInUnsizedDesc                    =  64; // if used extension GL_EXT_nonuniform_qualifier
CONSTEXPR u32   GPUPageSizeInMb                             = 256;
#endif
//----------------------------------------------------------------------------
CONSTEXPR u32   MaxRenderPassSubpasses                      =   8;
CONSTEXPR u32   MaxPushConstantsCount                       =   8;
CONSTEXPR u32   MaxPushConstantsSize                        = 128; // bytes
CONSTEXPR u32   MaxSpecializationConstants                  =   8;
CONSTEXPR u32   MaxQueueFamilies                            =  32;
//----------------------------------------------------------------------------
CONSTEXPR u32   MaxComputeCommands                          =  16;
CONSTEXPR u32   MaxDrawCommands                             =  16;
CONSTEXPR u32   MaxResourceStates                           =   8;
//----------------------------------------------------------------------------
CONSTEXPR u32   MaxTaskDependencies                         =   8;
CONSTEXPR u32   MaxCopyRegions                              =   8;
CONSTEXPR u32   MaxClearRanges                              =   8;
CONSTEXPR u32   MaxBlitRegions                              =   8;
CONSTEXPR u32   MaxResolveRegions                           =   8;
//----------------------------------------------------------------------------
CONSTEXPR bool  EnableShaderDebugging                       = USE_PPE_RHIDEBUG;
CONSTEXPR u32   DebugDescriptorSet                          = MaxDescriptorSets - 1;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
