#pragma once

#include "Core.Pixmap/Pixmap.h"

#include "Core/IO/StringView.h"
#include "Core/Meta/enum.h"

#include <iosfwd>

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EBlockFormat {
    DXT1    = 8,
    DXT5    = 16,
};
FStringView BlockFormatToCStr(EBlockFormat value);
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, EBlockFormat value) {
    return oss << BlockFormatToCStr(value);
}
//----------------------------------------------------------------------------
enum class EColorDepth {
    _8bits  = 8,
    _16bits = 16,
    _32bits = 32,
};
FStringView ColorDepthToCStr(EColorDepth value);
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, EColorDepth value) {
    return oss << ColorDepthToCStr(value);
}
//----------------------------------------------------------------------------
enum class EColorMask {
    R       = 1,
    RG      = 2,
    RGB     = 3,
    RGBA    = 4,
};
FStringView ColorMaskToCStr(EColorMask value);
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, EColorMask value) {
    return oss << ColorMaskToCStr(value);
}
//----------------------------------------------------------------------------
enum class EColorSpace {
    Linear  = 0,
    sRGB,
    Float,
    YCoCg,
};
FStringView ColorSpaceToCStr(EColorSpace value);
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, EColorSpace value) {
    return oss << ColorSpaceToCStr(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
