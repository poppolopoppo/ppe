// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Enums.h"

#include "IO/StringView.h"

namespace PPE {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView BlockFormatToCStr(EBlockFormat value) {
    switch (value)
    {
    case PPE::Pixmap::EBlockFormat::DXT1:
        return "DXT1";
    case PPE::Pixmap::EBlockFormat::DXT5:
        return "DXT5";
    }
    AssertNotImplemented();
}
FTextWriter& operator <<(FTextWriter& oss, EBlockFormat value) { return oss << BlockFormatToCStr(value); }
FWTextWriter& operator <<(FWTextWriter& oss, EBlockFormat value) { return oss << BlockFormatToCStr(value); }
//----------------------------------------------------------------------------
FStringView ColorDepthToCStr(EColorDepth value) {
    switch (value)
    {
    case PPE::Pixmap::EColorDepth::_8bits:
        return "8";
    case PPE::Pixmap::EColorDepth::_16bits:
        return "16";
    case PPE::Pixmap::EColorDepth::_32bits:
        return "32";
    }
    AssertNotImplemented();
}
FTextWriter& operator <<(FTextWriter& oss, EColorDepth value) { return oss << ColorDepthToCStr(value); }
FWTextWriter& operator <<(FWTextWriter& oss, EColorDepth value) { return oss << ColorDepthToCStr(value); }
//----------------------------------------------------------------------------
FStringView ColorMaskToCStr(EColorMask value) {
    switch (value)
    {
    case PPE::Pixmap::EColorMask::R:
        return "R";
    case PPE::Pixmap::EColorMask::RG:
        return "RG";
    case PPE::Pixmap::EColorMask::RGB:
        return "RGB";
    case PPE::Pixmap::EColorMask::RGBA:
        return "RGBA";
    }
    AssertNotImplemented();
}
FTextWriter& operator <<(FTextWriter& oss, EColorMask value) { return oss << ColorMaskToCStr(value); }
FWTextWriter& operator <<(FWTextWriter& oss, EColorMask value) { return oss << ColorMaskToCStr(value); }
//----------------------------------------------------------------------------
FStringView ColorSpaceToCStr(EColorSpace value) {
    switch (value)
    {
    case PPE::Pixmap::EColorSpace::Linear:
        return "Linear";
    case PPE::Pixmap::EColorSpace::sRGB:
        return "sRGB";
    case PPE::Pixmap::EColorSpace::YCoCg:
        return "YCoCg";
    }
    AssertNotImplemented();
}
FTextWriter& operator <<(FTextWriter& oss, EColorSpace value) { return oss << ColorSpaceToCStr(value); }
FWTextWriter& operator <<(FWTextWriter& oss, EColorSpace value) { return oss << ColorSpaceToCStr(value); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace PPE
