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
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, EBlockFormat value) {
    return oss << BlockFormatToCStr(value);
}
//----------------------------------------------------------------------------
enum EColorDepth {
    _8bits  = 8,
    _16bits = 16,
    _32bits = 32,
};
CORE_PIXMAP_API FStringView ColorDepthToCStr(EColorDepth value);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, EColorDepth value) {
    return oss << ColorDepthToCStr(value);
}
//----------------------------------------------------------------------------
enum EColorMask {
    R       = 1,
    RG      = 2,
    RGB     = 3,
    RGBA    = 4,
};
CORE_PIXMAP_API FStringView ColorMaskToCStr(EColorMask value);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, EColorMask value) {
    return oss << ColorMaskToCStr(value);
}
//----------------------------------------------------------------------------
enum EColorSpace {
    Linear  = 0,
    sRGB,
    YCoCg,
};
CORE_PIXMAP_API FStringView ColorSpaceToCStr(EColorSpace value);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, EColorSpace value) {
    return oss << ColorSpaceToCStr(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
