#pragma once

#include "RHI_fwd.h"

#include "IO/TextWriter.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define PPE_RHI_ENUMTOSTRING_DECL(_Enum) \
    PPE_RHI_API FTextWriter& operator <<(FTextWriter& oss, _Enum value); \
    PPE_RHI_API FWTextWriter& operator <<(FWTextWriter& oss, _Enum value)
//----------------------------------------------------------------------------
PPE_RHI_ENUMTOSTRING_DECL(EQueueType);
PPE_RHI_ENUMTOSTRING_DECL(EQueueUsage);
PPE_RHI_ENUMTOSTRING_DECL(EMemoryType);
PPE_RHI_ENUMTOSTRING_DECL(EBufferUsage);
PPE_RHI_ENUMTOSTRING_DECL(EImageDim);
PPE_RHI_ENUMTOSTRING_DECL(EImageView);
PPE_RHI_ENUMTOSTRING_DECL(EImageFlags);
PPE_RHI_ENUMTOSTRING_DECL(EImageUsage);
PPE_RHI_ENUMTOSTRING_DECL(EImageAspect);
PPE_RHI_ENUMTOSTRING_DECL(EImageSampler);
PPE_RHI_ENUMTOSTRING_DECL(EAttachmentStoreOp);
PPE_RHI_ENUMTOSTRING_DECL(EShadingRatePalette);
PPE_RHI_ENUMTOSTRING_DECL(EPixelFormat);
PPE_RHI_ENUMTOSTRING_DECL(EColorSpace);
PPE_RHI_ENUMTOSTRING_DECL(EFragmentOutput);
#if USE_PPE_RHIDEBUG
PPE_RHI_ENUMTOSTRING_DECL(EDebugFlags);
#endif
PPE_RHI_ENUMTOSTRING_DECL(EBlendFactor);
PPE_RHI_ENUMTOSTRING_DECL(EBlendOp);
PPE_RHI_ENUMTOSTRING_DECL(ELogicOp);
PPE_RHI_ENUMTOSTRING_DECL(EColorMask);
PPE_RHI_ENUMTOSTRING_DECL(ECompareOp);
PPE_RHI_ENUMTOSTRING_DECL(EStencilOp);
PPE_RHI_ENUMTOSTRING_DECL(EPolygonMode);
PPE_RHI_ENUMTOSTRING_DECL(EPrimitiveTopology);
PPE_RHI_ENUMTOSTRING_DECL(ECullMode);
PPE_RHI_ENUMTOSTRING_DECL(EPipelineDynamicState);
PPE_RHI_ENUMTOSTRING_DECL(ERayTracingGeometryFlags);
PPE_RHI_ENUMTOSTRING_DECL(ERayTracingInstanceFlags);
PPE_RHI_ENUMTOSTRING_DECL(ERayTracingBuildFlags);
PPE_RHI_ENUMTOSTRING_DECL(EShaderType);
PPE_RHI_ENUMTOSTRING_DECL(EShaderStages);
PPE_RHI_ENUMTOSTRING_DECL(EShaderAccess);
PPE_RHI_ENUMTOSTRING_DECL(EShaderLangFormat);
PPE_RHI_ENUMTOSTRING_DECL(EShaderDebugMode);
PPE_RHI_ENUMTOSTRING_DECL(ETextureFilter);
PPE_RHI_ENUMTOSTRING_DECL(EMipmapFilter);
PPE_RHI_ENUMTOSTRING_DECL(EAddressMode);
PPE_RHI_ENUMTOSTRING_DECL(EBorderColor);
PPE_RHI_ENUMTOSTRING_DECL(EIndexFormat);
PPE_RHI_ENUMTOSTRING_DECL(EVertexFormat);
PPE_RHI_ENUMTOSTRING_DECL(EPixelValueType);
PPE_RHI_ENUMTOSTRING_DECL(EPresentMode);
PPE_RHI_ENUMTOSTRING_DECL(ESurfaceTransform);
PPE_RHI_ENUMTOSTRING_DECL(EResourceState);
//----------------------------------------------------------------------------
#undef PPE_RHI_ENUMTOSTRING_DECL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
