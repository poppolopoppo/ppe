// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Texture/EnumToString.h"

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
    if (value ^ ETextureSourceFlags::PreMultipliedAlpha)
        oss << STRING_LITERAL(_Char, "PreMultipliedAlpha");
    if (value ^ ETextureSourceFlags::SRGB)
        oss << STRING_LITERAL(_Char, "sRGB");
    if (value ^ ETextureSourceFlags::Tiling)
        oss << STRING_LITERAL(_Char, "Tiling");
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
