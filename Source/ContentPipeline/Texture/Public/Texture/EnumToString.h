#pragma once

#include "Texture_fwd.h"

#include "Texture/TextureEnums.h"

#include "IO/TextReader_fwd.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_TEXTURE_API FTextWriter& operator <<(FTextWriter& oss, EImageFormat value);
PPE_TEXTURE_API FWTextWriter& operator <<(FWTextWriter& oss, EImageFormat value);
//----------------------------------------------------------------------------
PPE_TEXTURE_API FTextWriter& operator <<(FTextWriter& oss, ETextureSourceCompression value);
PPE_TEXTURE_API FWTextWriter& operator <<(FWTextWriter& oss, ETextureSourceCompression value);
//----------------------------------------------------------------------------
PPE_TEXTURE_API FTextWriter& operator <<(FTextWriter& oss, ETextureSourceFlags value);
PPE_TEXTURE_API FWTextWriter& operator <<(FWTextWriter& oss, ETextureSourceFlags value);
//----------------------------------------------------------------------------
PPE_TEXTURE_API FTextWriter& operator <<(FTextWriter& oss, ETextureSourceFormat value);
PPE_TEXTURE_API FWTextWriter& operator <<(FWTextWriter& oss, ETextureSourceFormat value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
