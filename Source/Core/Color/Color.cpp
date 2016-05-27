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
// http://www.chilliant.com/rgb2hsv.html
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
//////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////
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
// YCoCg
// ranges: Y=0..1, Co=-0.5..0.5, Cg=-0.5..0.5
// ranges returned by thoses functions is shifted to [0..1] for every channel
// https://www.shadertoy.com/view/4dcSRN
//----------------------------------------------------------------------------
float3 YCoCg_to_RGB(const float3& yCoCg) {
    float y = yCoCg.x();
    float Co = yCoCg.y() + 0.5f;
    float Cg = yCoCg.z() + 0.5f;
    float r = Co - Cg;
    float b = -Co - Cg;
    return float3(y + r, y + Cg, y + b);
}
//----------------------------------------------------------------------------
float3 RGB_to_YCoCg(const float3& rgb) {
    float tmp = 0.5f*(rgb.x() + rgb.z());
    float y = rgb.y() + tmp;
    float Cg = rgb.y() - tmp;
    float Co = rgb.x() - rgb.z();
    return float3(y * 0.5f, Co*0.5f+0.5f, Cg*0.5f+0.5f);
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
ColorRGBA AliceBlue() { return ColorRGBA(u8(0xF0), u8(0xF8), u8(0xFF), u8(0xFF)); }
ColorRGBA AntiqueWhite() { return ColorRGBA(u8(0xFA), u8(0xEB), u8(0xD7), u8(0xFF)); }
ColorRGBA Aqua() { return ColorRGBA(u8(0x00), u8(0xFF), u8(0xFF), u8(0xFF)); }
ColorRGBA Aquamarine() { return ColorRGBA(u8(0x7F), u8(0xFF), u8(0xD4), u8(0xFF)); }
ColorRGBA Azure() { return ColorRGBA(u8(0xF0), u8(0xFF), u8(0xFF), u8(0xFF)); }
ColorRGBA Beige() { return ColorRGBA(u8(0xF5), u8(0xF5), u8(0xDC), u8(0xFF)); }
ColorRGBA Bisque() { return ColorRGBA(u8(0xFF), u8(0xE4), u8(0xC4), u8(0xFF)); }
ColorRGBA Black() { return ColorRGBA(u8(0x00), u8(0x00), u8(0x00), u8(0xFF)); }
ColorRGBA BlanchedAlmond() { return ColorRGBA(u8(0xFF), u8(0xEB), u8(0xCD), u8(0xFF)); }
ColorRGBA Blue() { return ColorRGBA(u8(0x00), u8(0x00), u8(0xFF), u8(0xFF)); }
ColorRGBA BlueViolet() { return ColorRGBA(u8(0x8A), u8(0x2B), u8(0xE2), u8(0xFF)); }
ColorRGBA Brown() { return ColorRGBA(u8(0xA5), u8(0x2A), u8(0x2A), u8(0xFF)); }
ColorRGBA BurlyWood() { return ColorRGBA(u8(0xDE), u8(0xB8), u8(0x87), u8(0xFF)); }
ColorRGBA CadetBlue() { return ColorRGBA(u8(0x5F), u8(0x9E), u8(0xA0), u8(0xFF)); }
ColorRGBA Chartreuse() { return ColorRGBA(u8(0x7F), u8(0xFF), u8(0x00), u8(0xFF)); }
ColorRGBA Chocolate() { return ColorRGBA(u8(0xD2), u8(0x69), u8(0x1E), u8(0xFF)); }
ColorRGBA Coral() { return ColorRGBA(u8(0xFF), u8(0x7F), u8(0x50), u8(0xFF)); }
ColorRGBA CornflowerBlue() { return ColorRGBA(u8(0x64), u8(0x95), u8(0xED), u8(0xFF)); }
ColorRGBA Cornsilk() { return ColorRGBA(u8(0xFF), u8(0xF8), u8(0xDC), u8(0xFF)); }
ColorRGBA Crimson() { return ColorRGBA(u8(0xDC), u8(0x14), u8(0x3C), u8(0xFF)); }
ColorRGBA Cyan() { return ColorRGBA(u8(0x00), u8(0xFF), u8(0xFF), u8(0xFF)); }
ColorRGBA DarkBlue() { return ColorRGBA(u8(0x00), u8(0x00), u8(0x8B), u8(0xFF)); }
ColorRGBA DarkCyan() { return ColorRGBA(u8(0x00), u8(0x8B), u8(0x8B), u8(0xFF)); }
ColorRGBA DarkGoldenRod() { return ColorRGBA(u8(0xB8), u8(0x86), u8(0x0B), u8(0xFF)); }
ColorRGBA DarkGray() { return ColorRGBA(u8(0xA9), u8(0xA9), u8(0xA9), u8(0xFF)); }
ColorRGBA DarkGreen() { return ColorRGBA(u8(0x00), u8(0x64), u8(0x00), u8(0xFF)); }
ColorRGBA DarkKhaki() { return ColorRGBA(u8(0xBD), u8(0xB7), u8(0x6B), u8(0xFF)); }
ColorRGBA DarkMagenta() { return ColorRGBA(u8(0x8B), u8(0x00), u8(0x8B), u8(0xFF)); }
ColorRGBA DarkOliveGreen() { return ColorRGBA(u8(0x55), u8(0x6B), u8(0x2F), u8(0xFF)); }
ColorRGBA DarkOrange() { return ColorRGBA(u8(0xFF), u8(0x8C), u8(0x00), u8(0xFF)); }
ColorRGBA DarkOrchid() { return ColorRGBA(u8(0x99), u8(0x32), u8(0xCC), u8(0xFF)); }
ColorRGBA DarkRed() { return ColorRGBA(u8(0x8B), u8(0x00), u8(0x00), u8(0xFF)); }
ColorRGBA DarkSalmon() { return ColorRGBA(u8(0xE9), u8(0x96), u8(0x7A), u8(0xFF)); }
ColorRGBA DarkSeaGreen() { return ColorRGBA(u8(0x8F), u8(0xBC), u8(0x8F), u8(0xFF)); }
ColorRGBA DarkSlateBlue() { return ColorRGBA(u8(0x48), u8(0x3D), u8(0x8B), u8(0xFF)); }
ColorRGBA DarkSlateGray() { return ColorRGBA(u8(0x2F), u8(0x4F), u8(0x4F), u8(0xFF)); }
ColorRGBA DarkTurquoise() { return ColorRGBA(u8(0x00), u8(0xCE), u8(0xD1), u8(0xFF)); }
ColorRGBA DarkViolet() { return ColorRGBA(u8(0x94), u8(0x00), u8(0xD3), u8(0xFF)); }
ColorRGBA DeepPink() { return ColorRGBA(u8(0xFF), u8(0x14), u8(0x93), u8(0xFF)); }
ColorRGBA DeepSkyBlue() { return ColorRGBA(u8(0x00), u8(0xBF), u8(0xFF), u8(0xFF)); }
ColorRGBA DimGray() { return ColorRGBA(u8(0x69), u8(0x69), u8(0x69), u8(0xFF)); }
ColorRGBA DodgerBlue() { return ColorRGBA(u8(0x1E), u8(0x90), u8(0xFF), u8(0xFF)); }
ColorRGBA FireBrick() { return ColorRGBA(u8(0xB2), u8(0x22), u8(0x22), u8(0xFF)); }
ColorRGBA FloralWhite() { return ColorRGBA(u8(0xFF), u8(0xFA), u8(0xF0), u8(0xFF)); }
ColorRGBA ForestGreen() { return ColorRGBA(u8(0x22), u8(0x8B), u8(0x22), u8(0xFF)); }
ColorRGBA Fuchsia() { return ColorRGBA(u8(0xFF), u8(0x00), u8(0xFF), u8(0xFF)); }
ColorRGBA Gainsboro() { return ColorRGBA(u8(0xDC), u8(0xDC), u8(0xDC), u8(0xFF)); }
ColorRGBA GhostWhite() { return ColorRGBA(u8(0xF8), u8(0xF8), u8(0xFF), u8(0xFF)); }
ColorRGBA Gold() { return ColorRGBA(u8(0xFF), u8(0xD7), u8(0x00), u8(0xFF)); }
ColorRGBA GoldenRod() { return ColorRGBA(u8(0xDA), u8(0xA5), u8(0x20), u8(0xFF)); }
ColorRGBA Gray() { return ColorRGBA(u8(0x80), u8(0x80), u8(0x80), u8(0xFF)); }
ColorRGBA Green() { return ColorRGBA(u8(0x00), u8(0x80), u8(0x00), u8(0xFF)); }
ColorRGBA GreenYellow() { return ColorRGBA(u8(0xAD), u8(0xFF), u8(0x2F), u8(0xFF)); }
ColorRGBA HoneyDew() { return ColorRGBA(u8(0xF0), u8(0xFF), u8(0xF0), u8(0xFF)); }
ColorRGBA HotPink() { return ColorRGBA(u8(0xFF), u8(0x69), u8(0xB4), u8(0xFF)); }
ColorRGBA IndianRed() { return ColorRGBA(u8(0xCD), u8(0x5C), u8(0x5C), u8(0xFF)); }
ColorRGBA Indigo() { return ColorRGBA(u8(0x4B), u8(0x00), u8(0x82), u8(0xFF)); }
ColorRGBA Ivory() { return ColorRGBA(u8(0xFF), u8(0xFF), u8(0xF0), u8(0xFF)); }
ColorRGBA Khaki() { return ColorRGBA(u8(0xF0), u8(0xE6), u8(0x8C), u8(0xFF)); }
ColorRGBA Lavender() { return ColorRGBA(u8(0xE6), u8(0xE6), u8(0xFA), u8(0xFF)); }
ColorRGBA LavenderBlush() { return ColorRGBA(u8(0xFF), u8(0xF0), u8(0xF5), u8(0xFF)); }
ColorRGBA LawnGreen() { return ColorRGBA(u8(0x7C), u8(0xFC), u8(0x00), u8(0xFF)); }
ColorRGBA LemonChiffon() { return ColorRGBA(u8(0xFF), u8(0xFA), u8(0xCD), u8(0xFF)); }
ColorRGBA LightBlue() { return ColorRGBA(u8(0xAD), u8(0xD8), u8(0xE6), u8(0xFF)); }
ColorRGBA LightCoral() { return ColorRGBA(u8(0xF0), u8(0x80), u8(0x80), u8(0xFF)); }
ColorRGBA LightCyan() { return ColorRGBA(u8(0xE0), u8(0xFF), u8(0xFF), u8(0xFF)); }
ColorRGBA LightGoldenRodYellow() { return ColorRGBA(u8(0xFA), u8(0xFA), u8(0xD2), u8(0xFF)); }
ColorRGBA LightGray() { return ColorRGBA(u8(0xD3), u8(0xD3), u8(0xD3), u8(0xFF)); }
ColorRGBA LightGreen() { return ColorRGBA(u8(0x90), u8(0xEE), u8(0x90), u8(0xFF)); }
ColorRGBA LightPink() { return ColorRGBA(u8(0xFF), u8(0xB6), u8(0xC1), u8(0xFF)); }
ColorRGBA LightSalmon() { return ColorRGBA(u8(0xFF), u8(0xA0), u8(0x7A), u8(0xFF)); }
ColorRGBA LightSeaGreen() { return ColorRGBA(u8(0x20), u8(0xB2), u8(0xAA), u8(0xFF)); }
ColorRGBA LightSkyBlue() { return ColorRGBA(u8(0x87), u8(0xCE), u8(0xFA), u8(0xFF)); }
ColorRGBA LightSlateGray() { return ColorRGBA(u8(0x77), u8(0x88), u8(0x99), u8(0xFF)); }
ColorRGBA LightSteelBlue() { return ColorRGBA(u8(0xB0), u8(0xC4), u8(0xDE), u8(0xFF)); }
ColorRGBA LightYellow() { return ColorRGBA(u8(0xFF), u8(0xFF), u8(0xE0), u8(0xFF)); }
ColorRGBA Lime() { return ColorRGBA(u8(0x00), u8(0xFF), u8(0x00), u8(0xFF)); }
ColorRGBA LimeGreen() { return ColorRGBA(u8(0x32), u8(0xCD), u8(0x32), u8(0xFF)); }
ColorRGBA Linen() { return ColorRGBA(u8(0xFA), u8(0xF0), u8(0xE6), u8(0xFF)); }
ColorRGBA Magenta() { return ColorRGBA(u8(0xFF), u8(0x00), u8(0xFF), u8(0xFF)); }
ColorRGBA Maroon() { return ColorRGBA(u8(0x80), u8(0x00), u8(0x00), u8(0xFF)); }
ColorRGBA MediumAquaMarine() { return ColorRGBA(u8(0x66), u8(0xCD), u8(0xAA), u8(0xFF)); }
ColorRGBA MediumBlue() { return ColorRGBA(u8(0x00), u8(0x00), u8(0xCD), u8(0xFF)); }
ColorRGBA MediumOrchid() { return ColorRGBA(u8(0xBA), u8(0x55), u8(0xD3), u8(0xFF)); }
ColorRGBA MediumPurple() { return ColorRGBA(u8(0x93), u8(0x70), u8(0xDB), u8(0xFF)); }
ColorRGBA MediumSeaGreen() { return ColorRGBA(u8(0x3C), u8(0xB3), u8(0x71), u8(0xFF)); }
ColorRGBA MediumSlateBlue() { return ColorRGBA(u8(0x7B), u8(0x68), u8(0xEE), u8(0xFF)); }
ColorRGBA MediumSpringGreen() { return ColorRGBA(u8(0x00), u8(0xFA), u8(0x9A), u8(0xFF)); }
ColorRGBA MediumTurquoise() { return ColorRGBA(u8(0x48), u8(0xD1), u8(0xCC), u8(0xFF)); }
ColorRGBA MediumVioletRed() { return ColorRGBA(u8(0xC7), u8(0x15), u8(0x85), u8(0xFF)); }
ColorRGBA MidnightBlue() { return ColorRGBA(u8(0x19), u8(0x19), u8(0x70), u8(0xFF)); }
ColorRGBA MintCream() { return ColorRGBA(u8(0xF5), u8(0xFF), u8(0xFA), u8(0xFF)); }
ColorRGBA MistyRose() { return ColorRGBA(u8(0xFF), u8(0xE4), u8(0xE1), u8(0xFF)); }
ColorRGBA Moccasin() { return ColorRGBA(u8(0xFF), u8(0xE4), u8(0xB5), u8(0xFF)); }
ColorRGBA NavajoWhite() { return ColorRGBA(u8(0xFF), u8(0xDE), u8(0xAD), u8(0xFF)); }
ColorRGBA Navy() { return ColorRGBA(u8(0x00), u8(0x00), u8(0x80), u8(0xFF)); }
ColorRGBA OldLace() { return ColorRGBA(u8(0xFD), u8(0xF5), u8(0xE6), u8(0xFF)); }
ColorRGBA Olive() { return ColorRGBA(u8(0x80), u8(0x80), u8(0x00), u8(0xFF)); }
ColorRGBA OliveDrab() { return ColorRGBA(u8(0x6B), u8(0x8E), u8(0x23), u8(0xFF)); }
ColorRGBA Orange() { return ColorRGBA(u8(0xFF), u8(0xA5), u8(0x00), u8(0xFF)); }
ColorRGBA OrangeRed() { return ColorRGBA(u8(0xFF), u8(0x45), u8(0x00), u8(0xFF)); }
ColorRGBA Orchid() { return ColorRGBA(u8(0xDA), u8(0x70), u8(0xD6), u8(0xFF)); }
ColorRGBA PaleGoldenRod() { return ColorRGBA(u8(0xEE), u8(0xE8), u8(0xAA), u8(0xFF)); }
ColorRGBA PaleGreen() { return ColorRGBA(u8(0x98), u8(0xFB), u8(0x98), u8(0xFF)); }
ColorRGBA PaleTurquoise() { return ColorRGBA(u8(0xAF), u8(0xEE), u8(0xEE), u8(0xFF)); }
ColorRGBA PaleVioletRed() { return ColorRGBA(u8(0xDB), u8(0x70), u8(0x93), u8(0xFF)); }
ColorRGBA PapayaWhip() { return ColorRGBA(u8(0xFF), u8(0xEF), u8(0xD5), u8(0xFF)); }
ColorRGBA PeachPuff() { return ColorRGBA(u8(0xFF), u8(0xDA), u8(0xB9), u8(0xFF)); }
ColorRGBA Peru() { return ColorRGBA(u8(0xCD), u8(0x85), u8(0x3F), u8(0xFF)); }
ColorRGBA Pink() { return ColorRGBA(u8(0xFF), u8(0xC0), u8(0xCB), u8(0xFF)); }
ColorRGBA Plum() { return ColorRGBA(u8(0xDD), u8(0xA0), u8(0xDD), u8(0xFF)); }
ColorRGBA PowderBlue() { return ColorRGBA(u8(0xB0), u8(0xE0), u8(0xE6), u8(0xFF)); }
ColorRGBA Purple() { return ColorRGBA(u8(0x80), u8(0x00), u8(0x80), u8(0xFF)); }
ColorRGBA Red() { return ColorRGBA(u8(0xFF), u8(0x00), u8(0x00), u8(0xFF)); }
ColorRGBA RosyBrown() { return ColorRGBA(u8(0xBC), u8(0x8F), u8(0x8F), u8(0xFF)); }
ColorRGBA RoyalBlue() { return ColorRGBA(u8(0x41), u8(0x69), u8(0xE1), u8(0xFF)); }
ColorRGBA SaddleBrown() { return ColorRGBA(u8(0x8B), u8(0x45), u8(0x13), u8(0xFF)); }
ColorRGBA Salmon() { return ColorRGBA(u8(0xFA), u8(0x80), u8(0x72), u8(0xFF)); }
ColorRGBA SandyBrown() { return ColorRGBA(u8(0xF4), u8(0xA4), u8(0x60), u8(0xFF)); }
ColorRGBA SeaGreen() { return ColorRGBA(u8(0x2E), u8(0x8B), u8(0x57), u8(0xFF)); }
ColorRGBA SeaShell() { return ColorRGBA(u8(0xFF), u8(0xF5), u8(0xEE), u8(0xFF)); }
ColorRGBA Sienna() { return ColorRGBA(u8(0xA0), u8(0x52), u8(0x2D), u8(0xFF)); }
ColorRGBA Silver() { return ColorRGBA(u8(0xC0), u8(0xC0), u8(0xC0), u8(0xFF)); }
ColorRGBA SkyBlue() { return ColorRGBA(u8(0x87), u8(0xCE), u8(0xEB), u8(0xFF)); }
ColorRGBA SlateBlue() { return ColorRGBA(u8(0x6A), u8(0x5A), u8(0xCD), u8(0xFF)); }
ColorRGBA SlateGray() { return ColorRGBA(u8(0x70), u8(0x80), u8(0x90), u8(0xFF)); }
ColorRGBA Snow() { return ColorRGBA(u8(0xFF), u8(0xFA), u8(0xFA), u8(0xFF)); }
ColorRGBA SpringGreen() { return ColorRGBA(u8(0x00), u8(0xFF), u8(0x7F), u8(0xFF)); }
ColorRGBA SteelBlue() { return ColorRGBA(u8(0x46), u8(0x82), u8(0xB4), u8(0xFF)); }
ColorRGBA Tan() { return ColorRGBA(u8(0xD2), u8(0xB4), u8(0x8C), u8(0xFF)); }
ColorRGBA Teal() { return ColorRGBA(u8(0x00), u8(0x80), u8(0x80), u8(0xFF)); }
ColorRGBA Thistle() { return ColorRGBA(u8(0xD8), u8(0xBF), u8(0xD8), u8(0xFF)); }
ColorRGBA Tomato() { return ColorRGBA(u8(0xFF), u8(0x63), u8(0x47), u8(0xFF)); }
ColorRGBA Transparent() { return ColorRGBA(u8(0x00), u8(0x00), u8(0x00), u8(0x00)); }
ColorRGBA Turquoise() { return ColorRGBA(u8(0x40), u8(0xE0), u8(0xD0), u8(0xFF)); }
ColorRGBA Violet() { return ColorRGBA(u8(0xEE), u8(0x82), u8(0xEE), u8(0xFF)); }
ColorRGBA Wheat() { return ColorRGBA(u8(0xF5), u8(0xDE), u8(0xB3), u8(0xFF)); }
ColorRGBA White() { return ColorRGBA(u8(0xFF), u8(0xFF), u8(0xFF), u8(0xFF)); }
ColorRGBA WhiteSmoke() { return ColorRGBA(u8(0xF5), u8(0xF5), u8(0xF5), u8(0xFF)); }
ColorRGBA Yellow() { return ColorRGBA(u8(0xFF), u8(0xFF), u8(0x00), u8(0xFF)); }
ColorRGBA YellowGreen() { return ColorRGBA(u8(0x9A), u8(0xCD), u8(0x32), u8(0xFF)); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
