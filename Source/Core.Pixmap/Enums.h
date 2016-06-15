#pragma once

#include "Core.Pixmap/Pixmap.h"

#include "Core/IO/StringSlice.h"
#include "Core/Meta/enum.h"

#include <iosfwd>

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class BlockFormat {
    DXT1    = 8,
    DXT5    = 16,
};
StringSlice BlockFormatToCStr(BlockFormat value);
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, BlockFormat value) {
    return oss << BlockFormatToCStr(value);
}
//----------------------------------------------------------------------------
enum class ColorDepth {
    _8bits  = 8,
    _16bits = 16,
    _32bits = 32,
};
StringSlice ColorDepthToCStr(ColorDepth value);
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, ColorDepth value) {
    return oss << ColorDepthToCStr(value);
}
//----------------------------------------------------------------------------
enum class ColorMask {
    R       = 1,
    RG      = 2,
    RGB     = 3,
    RGBA    = 4,
};
StringSlice ColorMaskToCStr(ColorMask value);
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, ColorMask value) {
    return oss << ColorMaskToCStr(value);
}
//----------------------------------------------------------------------------
enum class ColorSpace {
    Linear  = 0,
    sRGB,
    Float,
    YCoCg,
};
StringSlice ColorSpaceToCStr(ColorSpace value);
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, ColorSpace value) {
    return oss << ColorSpaceToCStr(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
