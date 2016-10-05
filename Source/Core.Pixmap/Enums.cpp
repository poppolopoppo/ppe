#include "stdafx.h"

#include "Enums.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView BlockFormatToCStr(EBlockFormat value) {
    switch (value)
    {
    case Core::Pixmap::EBlockFormat::DXT1:
        return "DXT1";
    case Core::Pixmap::EBlockFormat::DXT5:
        return "DXT5";
    default:
        AssertNotImplemented();
        break;
    }
    return FStringView();
}
//----------------------------------------------------------------------------
FStringView ColorDepthToCStr(EColorDepth value) {
    switch (value)
    {
    case Core::Pixmap::EColorDepth::_8bits:
        return "8";
    case Core::Pixmap::EColorDepth::_16bits:
        return "16";
    case Core::Pixmap::EColorDepth::_32bits:
        return "32";
    default:
        AssertNotImplemented();
        break;
    }
    return FStringView();
}
//----------------------------------------------------------------------------
FStringView ColorMaskToCStr(EColorMask value) {
    switch (value)
    {
    case Core::Pixmap::EColorMask::R:
        return "R";
    case Core::Pixmap::EColorMask::RG:
        return "RG";
    case Core::Pixmap::EColorMask::RGB:
        return "RGB";
    case Core::Pixmap::EColorMask::RGBA:
        return "RGBA";
    default:
        AssertNotImplemented();
        break;
    }
    return FStringView();
}
//----------------------------------------------------------------------------
FStringView ColorSpaceToCStr(EColorSpace value) {
    switch (value)
    {
    case Core::Pixmap::EColorSpace::Linear:
        return "Linear";
    case Core::Pixmap::EColorSpace::sRGB:
        return "sRGB";
    case Core::Pixmap::EColorSpace::Float:
        return "Float";
    case Core::Pixmap::EColorSpace::YCoCg:
        return "YCoCg";
    default:
        AssertNotImplemented();
        break;
    }
    return FStringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
