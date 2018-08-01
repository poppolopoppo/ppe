#pragma once

#include "Core.Pixmap/Pixmap.h"

#include "Core/IO/String_fwd.h"
#include "Core/IO/TextWriter_fwd.h"
#include "Core/Meta/Enum.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum EBlockFormat {
    DXT1    = 8,
    DXT5    = 16,
};
CORE_PIXMAP_API FStringView BlockFormatToCStr(EBlockFormat value);
CORE_PIXMAP_API FTextWriter& operator <<(FTextWriter& oss, EBlockFormat value);
CORE_PIXMAP_API FWTextWriter& operator <<(FWTextWriter& oss, EBlockFormat value);
//----------------------------------------------------------------------------
enum EColorDepth {
    _8bits  = 8,
    _16bits = 16,
    _32bits = 32,
};
CORE_PIXMAP_API FStringView ColorDepthToCStr(EColorDepth value);
CORE_PIXMAP_API FTextWriter& operator <<(FTextWriter& oss, EColorDepth value);
CORE_PIXMAP_API FWTextWriter& operator <<(FWTextWriter& oss, EColorDepth value);
//----------------------------------------------------------------------------
enum EColorMask {
    R       = 1,
    RG      = 2,
    RGB     = 3,
    RGBA    = 4,
};
CORE_PIXMAP_API FStringView ColorMaskToCStr(EColorMask value);
CORE_PIXMAP_API FTextWriter& operator <<(FTextWriter& oss, EColorMask value);
CORE_PIXMAP_API FWTextWriter& operator <<(FWTextWriter& oss, EColorMask value);
//----------------------------------------------------------------------------
enum EColorSpace {
    Linear  = 0,
    sRGB,
    YCoCg,
};
CORE_PIXMAP_API FStringView ColorSpaceToCStr(EColorSpace value);
CORE_PIXMAP_API FTextWriter& operator <<(FTextWriter& oss, EColorSpace value);
CORE_PIXMAP_API FWTextWriter& operator <<(FWTextWriter& oss, EColorSpace value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
