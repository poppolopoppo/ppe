#pragma once

#include "Core_fwd.h"

#ifdef EXPORT_PPE_CONTENTPIPELINE_TEXTURE
#   define PPE_TEXTURE_API DLL_EXPORT
#else
#   define PPE_TEXTURE_API DLL_IMPORT
#endif

#include "RHI_fwd.h"

#include "Diagnostic/Logger_fwd.h"
#include "Maths/ScalarVector.h"
#include "Maths/Range.h"
#include "Memory/InSituPtr.h"
#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"
#include "Meta/StronglyTyped.h"

namespace PPE {
FWD_INTEFARCE_UNIQUEPTR(TextureService);
namespace ContentPipeline {
EXTERN_LOG_CATEGORY(PPE_TEXTURE_API, Texture);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EImageFormat : u8;
FWD_PUREINTEFACE_PTR(ImageFormat);
//----------------------------------------------------------------------------
enum class ETextureSourceCompression : u8;
enum class ETextureSourceFlags : u8;
enum class ETextureSourceFormat : u8;
//----------------------------------------------------------------------------
class FTextureSourceProperties;
FWD_REFPTR(TextureSource);
//----------------------------------------------------------------------------
enum class ETextureCompressionQuality : u8;
class FTextureCompressionSettings;
FWD_PUREINTEFACE_PTR(TextureCompression);
//----------------------------------------------------------------------------
enum class ETextureMipGeneration : u8;
class FTextureGeneration;
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(u8, FTextureGroupId);
//----------------------------------------------------------------------------
FWD_REFPTR(Texture);
FWD_REFPTR(Texture2D);
FWD_REFPTR(Texture2DArray);
FWD_REFPTR(Texture3D);
FWD_REFPTR(TextureCube);
FWD_REFPTR(TextureCubeArray);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
