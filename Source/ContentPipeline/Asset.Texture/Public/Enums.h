#pragma once

#include "Pixmap.h"

#include "IO/String_fwd.h"
#include "IO/TextWriter_fwd.h"
#include "Meta/Enum.h"

namespace PPE {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum EBlockFormat {
    DXT1    = 8,
    DXT5    = 16,
};
PPE_PIXMAP_API FStringView BlockFormatToCStr(EBlockFormat value);
PPE_PIXMAP_API FTextWriter& operator <<(FTextWriter& oss, EBlockFormat value);
PPE_PIXMAP_API FWTextWriter& operator <<(FWTextWriter& oss, EBlockFormat value);
//----------------------------------------------------------------------------
enum EColorDepth {
    _8bits  = 8,
    _16bits = 16,
    _32bits = 32,
};
PPE_PIXMAP_API FStringView ColorDepthToCStr(EColorDepth value);
PPE_PIXMAP_API FTextWriter& operator <<(FTextWriter& oss, EColorDepth value);
PPE_PIXMAP_API FWTextWriter& operator <<(FWTextWriter& oss, EColorDepth value);
//----------------------------------------------------------------------------
enum EColorMask {
    R       = 1,
    RG      = 2,
    RGB     = 3,
    RGBA    = 4,
};
PPE_PIXMAP_API FStringView ColorMaskToCStr(EColorMask value);
PPE_PIXMAP_API FTextWriter& operator <<(FTextWriter& oss, EColorMask value);
PPE_PIXMAP_API FWTextWriter& operator <<(FWTextWriter& oss, EColorMask value);
//----------------------------------------------------------------------------
enum EColorSpace {
    Linear  = 0,
    sRGB,
    YCoCg,
};
PPE_PIXMAP_API FStringView ColorSpaceToCStr(EColorSpace value);
PPE_PIXMAP_API FTextWriter& operator <<(FTextWriter& oss, EColorSpace value);
PPE_PIXMAP_API FWTextWriter& operator <<(FWTextWriter& oss, EColorSpace value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace PPE
