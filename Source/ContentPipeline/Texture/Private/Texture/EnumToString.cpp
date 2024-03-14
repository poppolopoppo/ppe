// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Texture/EnumToString.h"

#include "Texture/TextureCompression.h"
#include "Texture/TextureGeneration.h"
#include "Texture/TextureEnums.h"

#include "IO/TextWriter.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& EnumToString_(TBasicTextWriter<_Char>& oss, EImageFormat value) {
    switch (value) {
    case EImageFormat::PNG:
        return oss << STRING_LITERAL(_Char, "png");
    case EImageFormat::BMP:
        return oss << STRING_LITERAL(_Char, "bmp");
    case EImageFormat::TGA:
        return oss << STRING_LITERAL(_Char, "png");
    case EImageFormat::JPG:
        return oss << STRING_LITERAL(_Char, "jpg");
    case EImageFormat::HDR:
        return oss << STRING_LITERAL(_Char, "hdr");
    case EImageFormat::Unknown:
        return oss << STRING_LITERAL(_Char, "UNKNOWN");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& EnumToString_(TBasicTextWriter<_Char>& oss, ETextureMipGeneration value) {
    switch (value) {
    case ETextureMipGeneration::Default:
        return oss << STRING_LITERAL(_Char, "Default");
    case ETextureMipGeneration::Box:
        return oss << STRING_LITERAL(_Char, "Box");
    case ETextureMipGeneration::CubicSpine:
        return oss << STRING_LITERAL(_Char, "CubicSpine");
    case ETextureMipGeneration::CatmullRom:
        return oss << STRING_LITERAL(_Char, "CatmullRom");
    case ETextureMipGeneration::MitchellNetrevalli:
        return oss << STRING_LITERAL(_Char, "MitchellNetrevalli");
    case ETextureMipGeneration::PointSample:
        return oss << STRING_LITERAL(_Char, "PointSample");
    case ETextureMipGeneration::GaussianBlur3:
        return oss << STRING_LITERAL(_Char, "GaussianBlur3");
    case ETextureMipGeneration::GaussianBlur5:
        return oss << STRING_LITERAL(_Char, "GaussianBlur5");
    case ETextureMipGeneration::GaussianBlur7:
        return oss << STRING_LITERAL(_Char, "GaussianBlur7");
    case ETextureMipGeneration::GaussianBlur9:
        return oss << STRING_LITERAL(_Char, "GaussianBlur9");
    case ETextureMipGeneration::ContrastAdaptiveSharpen1:
        return oss << STRING_LITERAL(_Char, "ContrastAdaptiveSharpen1");
    case ETextureMipGeneration::ContrastAdaptiveSharpen2:
        return oss << STRING_LITERAL(_Char, "ContrastAdaptiveSharpen2");
    case ETextureMipGeneration::ContrastAdaptiveSharpen3:
        return oss << STRING_LITERAL(_Char, "ContrastAdaptiveSharpen3");
    case ETextureMipGeneration::ContrastAdaptiveSharpen4:
        return oss << STRING_LITERAL(_Char, "ContrastAdaptiveSharpen4");
    case ETextureMipGeneration::ContrastAdaptiveSharpen5:
        return oss << STRING_LITERAL(_Char, "ContrastAdaptiveSharpen5");
    case ETextureMipGeneration::ContrastAdaptiveSharpen6:
        return oss << STRING_LITERAL(_Char, "ContrastAdaptiveSharpen6");
    case ETextureMipGeneration::ContrastAdaptiveSharpen7:
        return oss << STRING_LITERAL(_Char, "ContrastAdaptiveSharpen7");
    case ETextureMipGeneration::ContrastAdaptiveSharpen8:
        return oss << STRING_LITERAL(_Char, "ContrastAdaptiveSharpen8");
    case ETextureMipGeneration::ContrastAdaptiveSharpen9:
        return oss << STRING_LITERAL(_Char, "ContrastAdaptiveSharpen9");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& EnumToString_(TBasicTextWriter<_Char>& oss, ETextureSourceCompression value) {
    switch (value) {
    case ETextureSourceCompression::None:
        return oss << STRING_LITERAL(_Char, "none");
    case ETextureSourceCompression::JPG:
        return oss << STRING_LITERAL(_Char, "jpg");
    case ETextureSourceCompression::PNG:
        return oss << STRING_LITERAL(_Char, "png");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& EnumToString_(TBasicTextWriter<_Char>& oss, ETextureSourceFlags value) {
    if (value ^ ETextureSourceFlags::HDR)
        oss << STRING_LITERAL(_Char, "HDR");
    if (value ^ ETextureSourceFlags::LongLatCubemap)
        oss << STRING_LITERAL(_Char, "LongLatCubemap");
    if (value ^ ETextureSourceFlags::MaskedAlpha)
        oss << STRING_LITERAL(_Char, "MaskedAlpha");
    if (value ^ ETextureSourceFlags::PreMultipliedAlpha)
        oss << STRING_LITERAL(_Char, "PreMultipliedAlpha");
    if (value ^ ETextureSourceFlags::Tilable)
        oss << STRING_LITERAL(_Char, "Tilable");
    if (value == ETextureSourceFlags::Unknown)
        oss << STRING_LITERAL(_Char, "UNKNOWN");
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& EnumToString_(TBasicTextWriter<_Char>& oss, ETextureSourceFormat value) {
    switch (value) {
    case ETextureSourceFormat::Unknown:
        return oss << STRING_LITERAL(_Char, "UNKNOWN");
    case ETextureSourceFormat::BGRA8:
        return oss << STRING_LITERAL(_Char, "BGRA8");
    case ETextureSourceFormat::BGRE8:
        return oss << STRING_LITERAL(_Char, "BGRE8");
    case ETextureSourceFormat::G16:
        return oss << STRING_LITERAL(_Char, "G16");
    case ETextureSourceFormat::G8:
        return oss << STRING_LITERAL(_Char, "G8");
    case ETextureSourceFormat::R16f:
        return oss << STRING_LITERAL(_Char, "R16f");
    case ETextureSourceFormat::RG16:
        return oss << STRING_LITERAL(_Char, "R16");
    case ETextureSourceFormat::RG8:
        return oss << STRING_LITERAL(_Char, "R8");
    case ETextureSourceFormat::RA16:
        return oss << STRING_LITERAL(_Char, "RA16");
    case ETextureSourceFormat::RA8:
        return oss << STRING_LITERAL(_Char, "RA8");
    case ETextureSourceFormat::RGBA16:
        return oss << STRING_LITERAL(_Char, "RGBA16");
    case ETextureSourceFormat::RGBA16f:
        return oss << STRING_LITERAL(_Char, "RGBA16f");
    case ETextureSourceFormat::RGBA32f:
        return oss << STRING_LITERAL(_Char, "RGBA32f");
    case ETextureSourceFormat::RGBA8:
        return oss << STRING_LITERAL(_Char, "RGBA8");

    case ETextureSourceFormat::_Last:
        AssertNotReached();
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& EnumToString_(TBasicTextWriter<_Char>& oss, ETextureCompressionQuality value) {
    switch (value) {
    case ETextureCompressionQuality::High:
        return oss << STRING_LITERAL(_Char, "High");
    case ETextureCompressionQuality::Medium:
        return oss << STRING_LITERAL(_Char, "Medium");
    case ETextureCompressionQuality::Low:
        return oss << STRING_LITERAL(_Char, "Low");
    }
    AssertNotReached();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EImageFormat value) {
    return EnumToString_(oss, value);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, EImageFormat value) {
    return EnumToString_(oss, value);
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, ETextureMipGeneration value) {
    return EnumToString_(oss, value);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, ETextureMipGeneration value) {
    return EnumToString_(oss, value);
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, ETextureSourceCompression value) {
    return EnumToString_(oss, value);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, ETextureSourceCompression value) {
    return EnumToString_(oss, value);
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, ETextureSourceFlags value) {
    return EnumToString_(oss, value);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, ETextureSourceFlags value) {
    return EnumToString_(oss, value);
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, ETextureSourceFormat value) {
    return EnumToString_(oss, value);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, ETextureSourceFormat value) {
    return EnumToString_(oss, value);
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, ETextureCompressionQuality value) {
    return EnumToString_(oss, value);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, ETextureCompressionQuality value) {
    return EnumToString_(oss, value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
