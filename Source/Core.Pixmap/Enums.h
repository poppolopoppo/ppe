#pragma once

#include "Core.Pixmap/Pixmap.h"

#include "Core/IO/StringSlice.h"
#include "Core/Meta/enum.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ColorDepth {
    _8bits  = 8,
    _16bits = 16,
    _32bits = 32,
};
StringSlice ColorDepthToCStr(ColorDepth value);
//----------------------------------------------------------------------------
enum class ColorMask {
    R       = 1,
    RG      = 2,
    RGB     = 3,
    RGBA    = 4,
};
StringSlice ColorMaskToCStr(ColorMask value);
//----------------------------------------------------------------------------
enum class ColorSpace {
    Linear  = 0,
    sRGB,
    Float,
    YCoCg,
};
StringSlice ColorSpaceToCStr(ColorSpace value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
