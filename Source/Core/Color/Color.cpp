#include "stdafx.h"

#include "Color.h"

#include <algorithm>

#include "Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float3 Hue_to_RGB(float hue) {
    float R = std::abs(hue * 6 - 3) - 1;
    float G = 2 - std::abs(hue * 6 - 2);
    float B = 2 - std::abs(hue * 6 - 4);
    return Saturate(float3(R,G,B));
}
//----------------------------------------------------------------------------
float3 RGB_to_HCV(const float3& rgb) {
    const float Epsilon = 1e-10f;
    // Based on work by Sam Hocevar and Emil Persson
    float4 P = (rgb.y() < rgb.z()) ? float4(rgb.z(), rgb.y(), -1.0f, 2.0f/3.0f) : float4(rgb.y(), rgb.z(), 0.0f, -1.0f/3.0f);
    float4 Q = (rgb.x() < P.x()) ? float4(P.xyw(), rgb.x()) : float4(rgb.x(), P.yzx());
    float C = Q.x() - std::min(Q.w(), Q.y());
    float H = std::abs((Q.w() - Q.y()) / (6 * C + Epsilon) + Q.z());
    return float3(H, C, Q.x());
}
//----------------------------------------------------------------------------
float3 HSV_to_RGB(const float3& hsv) {
    float3 RGB = Hue_to_RGB(hsv.x());
    return ((RGB - 1) * hsv.y() + 1) * hsv.z();
}
//----------------------------------------------------------------------------
float3 RGB_to_HSV(const float3& rgb) {
    const float Epsilon = 1e-10f;
    float3 HCV = RGB_to_HCV(rgb);
    float S = HCV.y() / (HCV.z() + Epsilon);
    return float3(HCV.x(), S, HCV.z());
}
//----------------------------------------------------------------------------
float3 HSL_to_RGB(const float3& hsl) {
    float3 RGB = Hue_to_RGB(hsl.x());
    float C = (1 - std::abs(2 * hsl.z() - 1)) * hsl.y();
    return (RGB - 0.5f) * C + hsl.z();
}
//----------------------------------------------------------------------------
float3 RGB_to_HSL(const float3& rgb) {
    const float Epsilon = 1e-10f;
    float3 HCV = RGB_to_HCV(rgb);
    float L = HCV.z() - HCV.y() * 0.5f;
    float S = HCV.y() / (1 - std::abs(L * 2 - 1) + Epsilon);
    return float3(HCV.x(), S, L);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template class BasicColor< UNorm<u8>, ColorShuffleBGRA >;
template class BasicColor< UNorm<u8>, ColorShuffleRGBA >;
//----------------------------------------------------------------------------
template class BasicColor< UNorm<u16>, ColorShuffleBGRA >;
template class BasicColor< UNorm<u16>, ColorShuffleRGBA >;
//----------------------------------------------------------------------------
template class BasicColor< float, ColorShuffleBGRA >;
template class BasicColor< float, ColorShuffleRGBA >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const ColorRGBA Color::AliceBlue{u8(0xF0), u8(0xF8), u8(0xFF), u8(0xFF)};
const ColorRGBA Color::AntiqueWhite{u8(0xFA), u8(0xEB), u8(0xD7), u8(0xFF)};
const ColorRGBA Color::Aqua{u8(0x00), u8(0xFF), u8(0xFF), u8(0xFF)};
const ColorRGBA Color::Aquamarine{u8(0x7F), u8(0xFF), u8(0xD4), u8(0xFF)};
const ColorRGBA Color::Azure{u8(0xF0), u8(0xFF), u8(0xFF), u8(0xFF)};
const ColorRGBA Color::Beige{u8(0xF5), u8(0xF5), u8(0xDC), u8(0xFF)};
const ColorRGBA Color::Bisque{u8(0xFF), u8(0xE4), u8(0xC4), u8(0xFF)};
const ColorRGBA Color::Black{u8(0x00), u8(0x00), u8(0x00), u8(0xFF)};
const ColorRGBA Color::BlanchedAlmond{u8(0xFF), u8(0xEB), u8(0xCD), u8(0xFF)};
const ColorRGBA Color::Blue{u8(0x00), u8(0x00), u8(0xFF), u8(0xFF)};
const ColorRGBA Color::BlueViolet{u8(0x8A), u8(0x2B), u8(0xE2), u8(0xFF)};
const ColorRGBA Color::Brown{u8(0xA5), u8(0x2A), u8(0x2A), u8(0xFF)};
const ColorRGBA Color::BurlyWood{u8(0xDE), u8(0xB8), u8(0x87), u8(0xFF)};
const ColorRGBA Color::CadetBlue{u8(0x5F), u8(0x9E), u8(0xA0), u8(0xFF)};
const ColorRGBA Color::Chartreuse{u8(0x7F), u8(0xFF), u8(0x00), u8(0xFF)};
const ColorRGBA Color::Chocolate{u8(0xD2), u8(0x69), u8(0x1E), u8(0xFF)};
const ColorRGBA Color::Coral{u8(0xFF), u8(0x7F), u8(0x50), u8(0xFF)};
const ColorRGBA Color::CornflowerBlue{u8(0x64), u8(0x95), u8(0xED), u8(0xFF)};
const ColorRGBA Color::Cornsilk{u8(0xFF), u8(0xF8), u8(0xDC), u8(0xFF)};
const ColorRGBA Color::Crimson{u8(0xDC), u8(0x14), u8(0x3C), u8(0xFF)};
const ColorRGBA Color::Cyan{u8(0x00), u8(0xFF), u8(0xFF), u8(0xFF)};
const ColorRGBA Color::DarkBlue{u8(0x00), u8(0x00), u8(0x8B), u8(0xFF)};
const ColorRGBA Color::DarkCyan{u8(0x00), u8(0x8B), u8(0x8B), u8(0xFF)};
const ColorRGBA Color::DarkGoldenRod{u8(0xB8), u8(0x86), u8(0x0B), u8(0xFF)};
const ColorRGBA Color::DarkGray{u8(0xA9), u8(0xA9), u8(0xA9), u8(0xFF)};
const ColorRGBA Color::DarkGreen{u8(0x00), u8(0x64), u8(0x00), u8(0xFF)};
const ColorRGBA Color::DarkKhaki{u8(0xBD), u8(0xB7), u8(0x6B), u8(0xFF)};
const ColorRGBA Color::DarkMagenta{u8(0x8B), u8(0x00), u8(0x8B), u8(0xFF)};
const ColorRGBA Color::DarkOliveGreen{u8(0x55), u8(0x6B), u8(0x2F), u8(0xFF)};
const ColorRGBA Color::DarkOrange{u8(0xFF), u8(0x8C), u8(0x00), u8(0xFF)};
const ColorRGBA Color::DarkOrchid{u8(0x99), u8(0x32), u8(0xCC), u8(0xFF)};
const ColorRGBA Color::DarkRed{u8(0x8B), u8(0x00), u8(0x00), u8(0xFF)};
const ColorRGBA Color::DarkSalmon{u8(0xE9), u8(0x96), u8(0x7A), u8(0xFF)};
const ColorRGBA Color::DarkSeaGreen{u8(0x8F), u8(0xBC), u8(0x8F), u8(0xFF)};
const ColorRGBA Color::DarkSlateBlue{u8(0x48), u8(0x3D), u8(0x8B), u8(0xFF)};
const ColorRGBA Color::DarkSlateGray{u8(0x2F), u8(0x4F), u8(0x4F), u8(0xFF)};
const ColorRGBA Color::DarkTurquoise{u8(0x00), u8(0xCE), u8(0xD1), u8(0xFF)};
const ColorRGBA Color::DarkViolet{u8(0x94), u8(0x00), u8(0xD3), u8(0xFF)};
const ColorRGBA Color::DeepPink{u8(0xFF), u8(0x14), u8(0x93), u8(0xFF)};
const ColorRGBA Color::DeepSkyBlue{u8(0x00), u8(0xBF), u8(0xFF), u8(0xFF)};
const ColorRGBA Color::DimGray{u8(0x69), u8(0x69), u8(0x69), u8(0xFF)};
const ColorRGBA Color::DodgerBlue{u8(0x1E), u8(0x90), u8(0xFF), u8(0xFF)};
const ColorRGBA Color::FireBrick{u8(0xB2), u8(0x22), u8(0x22), u8(0xFF)};
const ColorRGBA Color::FloralWhite{u8(0xFF), u8(0xFA), u8(0xF0), u8(0xFF)};
const ColorRGBA Color::ForestGreen{u8(0x22), u8(0x8B), u8(0x22), u8(0xFF)};
const ColorRGBA Color::Fuchsia{u8(0xFF), u8(0x00), u8(0xFF), u8(0xFF)};
const ColorRGBA Color::Gainsboro{u8(0xDC), u8(0xDC), u8(0xDC), u8(0xFF)};
const ColorRGBA Color::GhostWhite{u8(0xF8), u8(0xF8), u8(0xFF), u8(0xFF)};
const ColorRGBA Color::Gold{u8(0xFF), u8(0xD7), u8(0x00), u8(0xFF)};
const ColorRGBA Color::GoldenRod{u8(0xDA), u8(0xA5), u8(0x20), u8(0xFF)};
const ColorRGBA Color::Gray{u8(0x80), u8(0x80), u8(0x80), u8(0xFF)};
const ColorRGBA Color::Green{u8(0x00), u8(0x80), u8(0x00), u8(0xFF)};
const ColorRGBA Color::GreenYellow{u8(0xAD), u8(0xFF), u8(0x2F), u8(0xFF)};
const ColorRGBA Color::HoneyDew{u8(0xF0), u8(0xFF), u8(0xF0), u8(0xFF)};
const ColorRGBA Color::HotPink{u8(0xFF), u8(0x69), u8(0xB4), u8(0xFF)};
const ColorRGBA Color::IndianRed{u8(0xCD), u8(0x5C), u8(0x5C), u8(0xFF)};
const ColorRGBA Color::Indigo{u8(0x4B), u8(0x00), u8(0x82), u8(0xFF)};
const ColorRGBA Color::Ivory{u8(0xFF), u8(0xFF), u8(0xF0), u8(0xFF)};
const ColorRGBA Color::Khaki{u8(0xF0), u8(0xE6), u8(0x8C), u8(0xFF)};
const ColorRGBA Color::Lavender{u8(0xE6), u8(0xE6), u8(0xFA), u8(0xFF)};
const ColorRGBA Color::LavenderBlush{u8(0xFF), u8(0xF0), u8(0xF5), u8(0xFF)};
const ColorRGBA Color::LawnGreen{u8(0x7C), u8(0xFC), u8(0x00), u8(0xFF)};
const ColorRGBA Color::LemonChiffon{u8(0xFF), u8(0xFA), u8(0xCD), u8(0xFF)};
const ColorRGBA Color::LightBlue{u8(0xAD), u8(0xD8), u8(0xE6), u8(0xFF)};
const ColorRGBA Color::LightCoral{u8(0xF0), u8(0x80), u8(0x80), u8(0xFF)};
const ColorRGBA Color::LightCyan{u8(0xE0), u8(0xFF), u8(0xFF), u8(0xFF)};
const ColorRGBA Color::LightGoldenRodYellow{u8(0xFA), u8(0xFA), u8(0xD2), u8(0xFF)};
const ColorRGBA Color::LightGray{u8(0xD3), u8(0xD3), u8(0xD3), u8(0xFF)};
const ColorRGBA Color::LightGreen{u8(0x90), u8(0xEE), u8(0x90), u8(0xFF)};
const ColorRGBA Color::LightPink{u8(0xFF), u8(0xB6), u8(0xC1), u8(0xFF)};
const ColorRGBA Color::LightSalmon{u8(0xFF), u8(0xA0), u8(0x7A), u8(0xFF)};
const ColorRGBA Color::LightSeaGreen{u8(0x20), u8(0xB2), u8(0xAA), u8(0xFF)};
const ColorRGBA Color::LightSkyBlue{u8(0x87), u8(0xCE), u8(0xFA), u8(0xFF)};
const ColorRGBA Color::LightSlateGray{u8(0x77), u8(0x88), u8(0x99), u8(0xFF)};
const ColorRGBA Color::LightSteelBlue{u8(0xB0), u8(0xC4), u8(0xDE), u8(0xFF)};
const ColorRGBA Color::LightYellow{u8(0xFF), u8(0xFF), u8(0xE0), u8(0xFF)};
const ColorRGBA Color::Lime{u8(0x00), u8(0xFF), u8(0x00), u8(0xFF)};
const ColorRGBA Color::LimeGreen{u8(0x32), u8(0xCD), u8(0x32), u8(0xFF)};
const ColorRGBA Color::Linen{u8(0xFA), u8(0xF0), u8(0xE6), u8(0xFF)};
const ColorRGBA Color::Magenta{u8(0xFF), u8(0x00), u8(0xFF), u8(0xFF)};
const ColorRGBA Color::Maroon{u8(0x80), u8(0x00), u8(0x00), u8(0xFF)};
const ColorRGBA Color::MediumAquaMarine{u8(0x66), u8(0xCD), u8(0xAA), u8(0xFF)};
const ColorRGBA Color::MediumBlue{u8(0x00), u8(0x00), u8(0xCD), u8(0xFF)};
const ColorRGBA Color::MediumOrchid{u8(0xBA), u8(0x55), u8(0xD3), u8(0xFF)};
const ColorRGBA Color::MediumPurple{u8(0x93), u8(0x70), u8(0xDB), u8(0xFF)};
const ColorRGBA Color::MediumSeaGreen{u8(0x3C), u8(0xB3), u8(0x71), u8(0xFF)};
const ColorRGBA Color::MediumSlateBlue{u8(0x7B), u8(0x68), u8(0xEE), u8(0xFF)};
const ColorRGBA Color::MediumSpringGreen{u8(0x00), u8(0xFA), u8(0x9A), u8(0xFF)};
const ColorRGBA Color::MediumTurquoise{u8(0x48), u8(0xD1), u8(0xCC), u8(0xFF)};
const ColorRGBA Color::MediumVioletRed{u8(0xC7), u8(0x15), u8(0x85), u8(0xFF)};
const ColorRGBA Color::MidnightBlue{u8(0x19), u8(0x19), u8(0x70), u8(0xFF)};
const ColorRGBA Color::MintCream{u8(0xF5), u8(0xFF), u8(0xFA), u8(0xFF)};
const ColorRGBA Color::MistyRose{u8(0xFF), u8(0xE4), u8(0xE1), u8(0xFF)};
const ColorRGBA Color::Moccasin{u8(0xFF), u8(0xE4), u8(0xB5), u8(0xFF)};
const ColorRGBA Color::NavajoWhite{u8(0xFF), u8(0xDE), u8(0xAD), u8(0xFF)};
const ColorRGBA Color::Navy{u8(0x00), u8(0x00), u8(0x80), u8(0xFF)};
const ColorRGBA Color::OldLace{u8(0xFD), u8(0xF5), u8(0xE6), u8(0xFF)};
const ColorRGBA Color::Olive{u8(0x80), u8(0x80), u8(0x00), u8(0xFF)};
const ColorRGBA Color::OliveDrab{u8(0x6B), u8(0x8E), u8(0x23), u8(0xFF)};
const ColorRGBA Color::Orange{u8(0xFF), u8(0xA5), u8(0x00), u8(0xFF)};
const ColorRGBA Color::OrangeRed{u8(0xFF), u8(0x45), u8(0x00), u8(0xFF)};
const ColorRGBA Color::Orchid{u8(0xDA), u8(0x70), u8(0xD6), u8(0xFF)};
const ColorRGBA Color::PaleGoldenRod{u8(0xEE), u8(0xE8), u8(0xAA), u8(0xFF)};
const ColorRGBA Color::PaleGreen{u8(0x98), u8(0xFB), u8(0x98), u8(0xFF)};
const ColorRGBA Color::PaleTurquoise{u8(0xAF), u8(0xEE), u8(0xEE), u8(0xFF)};
const ColorRGBA Color::PaleVioletRed{u8(0xDB), u8(0x70), u8(0x93), u8(0xFF)};
const ColorRGBA Color::PapayaWhip{u8(0xFF), u8(0xEF), u8(0xD5), u8(0xFF)};
const ColorRGBA Color::PeachPuff{u8(0xFF), u8(0xDA), u8(0xB9), u8(0xFF)};
const ColorRGBA Color::Peru{u8(0xCD), u8(0x85), u8(0x3F), u8(0xFF)};
const ColorRGBA Color::Pink{u8(0xFF), u8(0xC0), u8(0xCB), u8(0xFF)};
const ColorRGBA Color::Plum{u8(0xDD), u8(0xA0), u8(0xDD), u8(0xFF)};
const ColorRGBA Color::PowderBlue{u8(0xB0), u8(0xE0), u8(0xE6), u8(0xFF)};
const ColorRGBA Color::Purple{u8(0x80), u8(0x00), u8(0x80), u8(0xFF)};
const ColorRGBA Color::Red{u8(0xFF), u8(0x00), u8(0x00), u8(0xFF)};
const ColorRGBA Color::RosyBrown{u8(0xBC), u8(0x8F), u8(0x8F), u8(0xFF)};
const ColorRGBA Color::RoyalBlue{u8(0x41), u8(0x69), u8(0xE1), u8(0xFF)};
const ColorRGBA Color::SaddleBrown{u8(0x8B), u8(0x45), u8(0x13), u8(0xFF)};
const ColorRGBA Color::Salmon{u8(0xFA), u8(0x80), u8(0x72), u8(0xFF)};
const ColorRGBA Color::SandyBrown{u8(0xF4), u8(0xA4), u8(0x60), u8(0xFF)};
const ColorRGBA Color::SeaGreen{u8(0x2E), u8(0x8B), u8(0x57), u8(0xFF)};
const ColorRGBA Color::SeaShell{u8(0xFF), u8(0xF5), u8(0xEE), u8(0xFF)};
const ColorRGBA Color::Sienna{u8(0xA0), u8(0x52), u8(0x2D), u8(0xFF)};
const ColorRGBA Color::Silver{u8(0xC0), u8(0xC0), u8(0xC0), u8(0xFF)};
const ColorRGBA Color::SkyBlue{u8(0x87), u8(0xCE), u8(0xEB), u8(0xFF)};
const ColorRGBA Color::SlateBlue{u8(0x6A), u8(0x5A), u8(0xCD), u8(0xFF)};
const ColorRGBA Color::SlateGray{u8(0x70), u8(0x80), u8(0x90), u8(0xFF)};
const ColorRGBA Color::Snow{u8(0xFF), u8(0xFA), u8(0xFA), u8(0xFF)};
const ColorRGBA Color::SpringGreen{u8(0x00), u8(0xFF), u8(0x7F), u8(0xFF)};
const ColorRGBA Color::SteelBlue{u8(0x46), u8(0x82), u8(0xB4), u8(0xFF)};
const ColorRGBA Color::Tan{u8(0xD2), u8(0xB4), u8(0x8C), u8(0xFF)};
const ColorRGBA Color::Teal{u8(0x00), u8(0x80), u8(0x80), u8(0xFF)};
const ColorRGBA Color::Thistle{u8(0xD8), u8(0xBF), u8(0xD8), u8(0xFF)};
const ColorRGBA Color::Tomato{u8(0xFF), u8(0x63), u8(0x47), u8(0xFF)};
const ColorRGBA Color::Turquoise{u8(0x40), u8(0xE0), u8(0xD0), u8(0xFF)};
const ColorRGBA Color::Violet{u8(0xEE), u8(0x82), u8(0xEE), u8(0xFF)};
const ColorRGBA Color::Wheat{u8(0xF5), u8(0xDE), u8(0xB3), u8(0xFF)};
const ColorRGBA Color::White{u8(0xFF), u8(0xFF), u8(0xFF), u8(0xFF)};
const ColorRGBA Color::WhiteSmoke{u8(0xF5), u8(0xF5), u8(0xF5), u8(0xFF)};
const ColorRGBA Color::Yellow{u8(0xFF), u8(0xFF), u8(0x00), u8(0xFF)};
const ColorRGBA Color::YellowGreen{u8(0x9A), u8(0xCD), u8(0x32), u8(0xFF)};
//----------------------------------------------------------------------------
namespace {
static const ColorRGBA *const gAllColors[] = {
    &Color::AliceBlue,
    &Color::AntiqueWhite,
    &Color::Aqua,
    &Color::Aquamarine,
    &Color::Azure,
    &Color::Beige,
    &Color::Bisque,
    &Color::Black,
    &Color::BlanchedAlmond,
    &Color::Blue,
    &Color::BlueViolet,
    &Color::Brown,
    &Color::BurlyWood,
    &Color::CadetBlue,
    &Color::Chartreuse,
    &Color::Chocolate,
    &Color::Coral,
    &Color::CornflowerBlue,
    &Color::Cornsilk,
    &Color::Crimson,
    &Color::Cyan,
    &Color::DarkBlue,
    &Color::DarkCyan,
    &Color::DarkGoldenRod,
    &Color::DarkGray,
    &Color::DarkGreen,
    &Color::DarkKhaki,
    &Color::DarkMagenta,
    &Color::DarkOliveGreen,
    &Color::DarkOrange,
    &Color::DarkOrchid,
    &Color::DarkRed,
    &Color::DarkSalmon,
    &Color::DarkSeaGreen,
    &Color::DarkSlateBlue,
    &Color::DarkSlateGray,
    &Color::DarkTurquoise,
    &Color::DarkViolet,
    &Color::DeepPink,
    &Color::DeepSkyBlue,
    &Color::DimGray,
    &Color::DodgerBlue,
    &Color::FireBrick,
    &Color::FloralWhite,
    &Color::ForestGreen,
    &Color::Fuchsia,
    &Color::Gainsboro,
    &Color::GhostWhite,
    &Color::Gold,
    &Color::GoldenRod,
    &Color::Gray,
    &Color::Green,
    &Color::GreenYellow,
    &Color::HoneyDew,
    &Color::HotPink,
    &Color::IndianRed,
    &Color::Indigo,
    &Color::Ivory,
    &Color::Khaki,
    &Color::Lavender,
    &Color::LavenderBlush,
    &Color::LawnGreen,
    &Color::LemonChiffon,
    &Color::LightBlue,
    &Color::LightCoral,
    &Color::LightCyan,
    &Color::LightGoldenRodYellow,
    &Color::LightGray,
    &Color::LightGreen,
    &Color::LightPink,
    &Color::LightSalmon,
    &Color::LightSeaGreen,
    &Color::LightSkyBlue,
    &Color::LightSlateGray,
    &Color::LightSteelBlue,
    &Color::LightYellow,
    &Color::Lime,
    &Color::LimeGreen,
    &Color::Linen,
    &Color::Magenta,
    &Color::Maroon,
    &Color::MediumAquaMarine,
    &Color::MediumBlue,
    &Color::MediumOrchid,
    &Color::MediumPurple,
    &Color::MediumSeaGreen,
    &Color::MediumSlateBlue,
    &Color::MediumSpringGreen,
    &Color::MediumTurquoise,
    &Color::MediumVioletRed,
    &Color::MidnightBlue,
    &Color::MintCream,
    &Color::MistyRose,
    &Color::Moccasin,
    &Color::NavajoWhite,
    &Color::Navy,
    &Color::OldLace,
    &Color::Olive,
    &Color::OliveDrab,
    &Color::Orange,
    &Color::OrangeRed,
    &Color::Orchid,
    &Color::PaleGoldenRod,
    &Color::PaleGreen,
    &Color::PaleTurquoise,
    &Color::PaleVioletRed,
    &Color::PapayaWhip,
    &Color::PeachPuff,
    &Color::Peru,
    &Color::Pink,
    &Color::Plum,
    &Color::PowderBlue,
    &Color::Purple,
    &Color::Red,
    &Color::RosyBrown,
    &Color::RoyalBlue,
    &Color::SaddleBrown,
    &Color::Salmon,
    &Color::SandyBrown,
    &Color::SeaGreen,
    &Color::SeaShell,
    &Color::Sienna,
    &Color::Silver,
    &Color::SkyBlue,
    &Color::SlateBlue,
    &Color::SlateGray,
    &Color::Snow,
    &Color::SpringGreen,
    &Color::SteelBlue,
    &Color::Tan,
    &Color::Teal,
    &Color::Thistle,
    &Color::Tomato,
    &Color::Turquoise,
    &Color::Violet,
    &Color::Wheat,
    &Color::White,
    &Color::WhiteSmoke,
    &Color::Yellow,
    &Color::YellowGreen,
    };
}
//----------------------------------------------------------------------------
MemoryView<const ColorRGBA *const> AllColors() {
    return MakeView(gAllColors);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
