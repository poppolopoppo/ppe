#include "stdafx.h"

#include "Enums.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice BlockFormatToCStr(BlockFormat value) {
    switch (value)
    {
    case Core::Pixmap::BlockFormat::DXT1:
        return "DXT1";
    case Core::Pixmap::BlockFormat::DXT5:
        return "DXT5";
    default:
        AssertNotImplemented();
        break;
    }
    return StringSlice();
}
//----------------------------------------------------------------------------
StringSlice ColorDepthToCStr(ColorDepth value) {
    switch (value)
    {
    case Core::Pixmap::ColorDepth::_8bits:
        return "8";
    case Core::Pixmap::ColorDepth::_16bits:
        return "16";
    case Core::Pixmap::ColorDepth::_32bits:
        return "32";
    default:
        AssertNotImplemented();
        break;
    }
    return StringSlice();
}
//----------------------------------------------------------------------------
StringSlice ColorMaskToCStr(ColorMask value) {
    switch (value)
    {
    case Core::Pixmap::ColorMask::R:
        return "R";
    case Core::Pixmap::ColorMask::RG:
        return "RG";
    case Core::Pixmap::ColorMask::RGB:
        return "RGB";
    case Core::Pixmap::ColorMask::RGBA:
        return "RGBA";
    default:
        AssertNotImplemented();
        break;
    }
    return StringSlice();
}
//----------------------------------------------------------------------------
StringSlice ColorSpaceToCStr(ColorSpace value) {
    switch (value)
    {
    case Core::Pixmap::ColorSpace::Linear:
        return "Linear";
    case Core::Pixmap::ColorSpace::sRGB:
        return "sRGB";
    case Core::Pixmap::ColorSpace::Float:
        return "Float";
    case Core::Pixmap::ColorSpace::YCoCg:
        return "YCoCg";
    default:
        AssertNotImplemented();
        break;
    }
    return StringSlice();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
