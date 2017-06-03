#pragma once

#include "Core/Core.h"

#include "Core/Color/Color_fwd.h"
#include "Core/Maths/ScalarVector_fwd.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGammaSpace {
    /** No gamma correction is applied to this space, the incoming colors are assumed to already be in linear space. */
    Linear,
    /** A simplified sRGB gamma correction is applied, pow(1/2.2). */
    Pow22,
    /** Use the standard sRGB conversion. */
    sRGB,
};
//----------------------------------------------------------------------------
float3 Hue_to_RGB(float hue);
//----------------------------------------------------------------------------
float3 RGB_to_HCV(const float3& rgb);
//----------------------------------------------------------------------------
float3 HSV_to_RGB(const float3& hsv);
float3 RGB_to_HSV(const float3& rgb);
//----------------------------------------------------------------------------
float3 HSL_to_RGB(const float3& hsl);
float3 RGB_to_HSL(const float3& rgb);
//----------------------------------------------------------------------------
float3 YCoCg_to_RGB(const float3& yCoCg);
float3 RGB_to_YCoCg(const float3& rgb);
//----------------------------------------------------------------------------
float SRGB_to_Linear(float srgb);
float SRGB_to_Linear(u8 srgb); // optimized version with a precomputed table
float Linear_to_SRGB(float lin);
//----------------------------------------------------------------------------
float Linear_to_Pow22(float lin);
float Pow22_to_Linear(float pow22);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct CORE_API FLinearColor {
    float R, G, B, A;

    FORCE_INLINE FLinearColor() {}
    FORCE_INLINE explicit FLinearColor(Meta::FForceInit) : R(0), G(0), B(0), A(0) {}
    FORCE_INLINE FLinearColor(float r, float g, float b, float a = 1.0f) : R(r), G(g), B(b), A(a) {}

    FLinearColor(const FColor& color);
    FLinearColor(const float3& rgb, float a = 1.0f);
    FLinearColor(const float4& rgba);

    operator const float3&() const { return *reinterpret_cast<const float3*>(this); }
    operator const float4&() const { return *reinterpret_cast<const float4*>(this); }

    float& operator [](size_t index) { Assert(index < 4); return (&R)[index]; }
    float operator [](size_t index) const { Assert(index < 4); return (&R)[index]; }

    FLinearColor Desaturate(float desaturation) const;
    FLinearColor Fade(float alpha) const { return FLinearColor(R, G, B, alpha); }

    float Luminance() const { return (R * 0.3f + G * 0.59f + B * 0.11f); }

    float4 ToBGRA() const;
    float3 ToHSL() const;
    float3 ToHSV() const;
    float3 ToYCoCg() const;

    FColor ToRGBE() const;
    FColor Quantize(EGammaSpace gamma) const;

    static FLinearColor FromHue(float hue, float a = 1.0f);
    static FLinearColor FromHSL(const float3& hsl, float a = 1.0f);
    static FLinearColor FromHSV(const float3& hsv, float a = 1.0f);
    static FLinearColor FromYCoCg(const float3& yCoCg, float a = 1.0f);
    static FLinearColor FromTemperature(float kelvins, float a = 1.0f);

    inline friend bool operator ==(const FLinearColor& lhs, const FLinearColor& rhs) {
        return (lhs.R == rhs.R && lhs.G == rhs.G && lhs.B == rhs.B && lhs.A == rhs.A);
    }
    inline friend bool operator !=(const FLinearColor& lhs, const FLinearColor& rhs) {
        return (not operator ==(lhs, rhs));
    }

    static FLinearColor AliceBlue();
    static FLinearColor AntiqueWhite();
    static FLinearColor Aqua();
    static FLinearColor Aquamarine();
    static FLinearColor Azure();
    static FLinearColor Beige();
    static FLinearColor Bisque();
    static FLinearColor Black();
    static FLinearColor BlanchedAlmond();
    static FLinearColor Blue();
    static FLinearColor BlueViolet();
    static FLinearColor Brown();
    static FLinearColor BurlyWood();
    static FLinearColor CadetBlue();
    static FLinearColor Chartreuse();
    static FLinearColor Chocolate();
    static FLinearColor Coral();
    static FLinearColor CornflowerBlue();
    static FLinearColor Cornsilk();
    static FLinearColor Crimson();
    static FLinearColor Cyan();
    static FLinearColor DarkBlue();
    static FLinearColor DarkCyan();
    static FLinearColor DarkGoldenRod();
    static FLinearColor DarkGray();
    static FLinearColor DarkGreen();
    static FLinearColor DarkKhaki();
    static FLinearColor DarkMagenta();
    static FLinearColor DarkOliveGreen();
    static FLinearColor DarkOrange();
    static FLinearColor DarkOrchid();
    static FLinearColor DarkRed();
    static FLinearColor DarkSalmon();
    static FLinearColor DarkSeaGreen();
    static FLinearColor DarkSlateBlue();
    static FLinearColor DarkSlateGray();
    static FLinearColor DarkTurquoise();
    static FLinearColor DarkViolet();
    static FLinearColor DeepPink();
    static FLinearColor DeepSkyBlue();
    static FLinearColor DimGray();
    static FLinearColor DodgerBlue();
    static FLinearColor FireBrick();
    static FLinearColor FloralWhite();
    static FLinearColor ForestGreen();
    static FLinearColor Fuchsia();
    static FLinearColor Gainsboro();
    static FLinearColor GhostWhite();
    static FLinearColor Gold();
    static FLinearColor GoldenRod();
    static FLinearColor Gray();
    static FLinearColor Green();
    static FLinearColor GreenYellow();
    static FLinearColor HoneyDew();
    static FLinearColor HotPink();
    static FLinearColor IndianRed();
    static FLinearColor Indigo();
    static FLinearColor Ivory();
    static FLinearColor Khaki();
    static FLinearColor Lavender();
    static FLinearColor LavenderBlush();
    static FLinearColor LawnGreen();
    static FLinearColor LemonChiffon();
    static FLinearColor LightBlue();
    static FLinearColor LightCoral();
    static FLinearColor LightCyan();
    static FLinearColor LightGoldenRodYellow();
    static FLinearColor LightGray();
    static FLinearColor LightGreen();
    static FLinearColor LightPink();
    static FLinearColor LightSalmon();
    static FLinearColor LightSeaGreen();
    static FLinearColor LightSkyBlue();
    static FLinearColor LightSlateGray();
    static FLinearColor LightSteelBlue();
    static FLinearColor LightYellow();
    static FLinearColor Lime();
    static FLinearColor LimeGreen();
    static FLinearColor Linen();
    static FLinearColor Magenta();
    static FLinearColor Maroon();
    static FLinearColor MediumAquamarine();
    static FLinearColor MediumBlue();
    static FLinearColor MediumOrchid();
    static FLinearColor MediumPurple();
    static FLinearColor MediumSeaGreen();
    static FLinearColor MediumSlateBlue();
    static FLinearColor MediumSpringGreen();
    static FLinearColor MediumTurquoise();
    static FLinearColor MediumVioletRed();
    static FLinearColor MidnightBlue();
    static FLinearColor MintCream();
    static FLinearColor MistyRose();
    static FLinearColor Moccasin();
    static FLinearColor NavajoWhite();
    static FLinearColor Navy();
    static FLinearColor OldLace();
    static FLinearColor Olive();
    static FLinearColor OliveDrab();
    static FLinearColor Orange();
    static FLinearColor OrangeRed();
    static FLinearColor Orchid();
    static FLinearColor PaleGoldenRod();
    static FLinearColor PaleGreen();
    static FLinearColor PaleTurquoise();
    static FLinearColor PaleVioletRed();
    static FLinearColor PapayaWhip();
    static FLinearColor PeachPuff();
    static FLinearColor Peru();
    static FLinearColor Pink();
    static FLinearColor Plum();
    static FLinearColor PowderBlue();
    static FLinearColor Purple();
    static FLinearColor Red();
    static FLinearColor RosyBrown();
    static FLinearColor RoyalBlue();
    static FLinearColor SaddleBrown();
    static FLinearColor Salmon();
    static FLinearColor SandyBrown();
    static FLinearColor SeaGreen();
    static FLinearColor SeaShell();
    static FLinearColor Sienna();
    static FLinearColor Silver();
    static FLinearColor SkyBlue();
    static FLinearColor SlateBlue();
    static FLinearColor SlateGray();
    static FLinearColor Snow();
    static FLinearColor SpringGreen();
    static FLinearColor SteelBlue();
    static FLinearColor Tan();
    static FLinearColor Teal();
    static FLinearColor Thistle();
    static FLinearColor Tomato();
    static FLinearColor Transparent();
    static FLinearColor Turquoise();
    static FLinearColor Violet();
    static FLinearColor Wheat();
    static FLinearColor White();
    static FLinearColor WhiteSmoke();
    static FLinearColor Yellow();
    static FLinearColor YellowGreen();
};
CORE_ASSUME_TYPE_AS_POD(FLinearColor)
//----------------------------------------------------------------------------
FLinearColor AlphaBlend(const FLinearColor& dst, const FLinearColor& src);
FLinearColor Lerp(const FLinearColor& from, const FLinearColor& to, float t);
FLinearColor LerpUsingHSV(const FLinearColor& from, const FLinearColor& to, float t);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct CORE_API FColor {
    union {
        u8  R, G, B, A;
        u32 DWord;
    };

    FORCE_INLINE FColor() {}
    FORCE_INLINE explicit FColor(Meta::FForceInit) : DWord(0) {}
    FORCE_INLINE explicit FColor(u32 dword) : DWord(dword) {}
    FORCE_INLINE FColor(u8 r, u8 g, u8 b, u8 a = 0xFF) : R(r), G(g), B(b), A(a) {}

    FColor(const ubyte3& rgb, u8 a = 1.0f);
    FColor(const ubyte4& rgba);

    operator const ubyte3&() const { return *reinterpret_cast<const ubyte3*>(this); }
    operator const ubyte4&() const { return *reinterpret_cast<const ubyte4*>(this); }

    u8& operator [](size_t index) { Assert(index < 4); return (&R)[index]; }
    u8 operator [](size_t index) const { Assert(index < 4); return (&R)[index]; }

    FColor Fade(u8 alpha) const { return FColor(R, G, B, alpha); }

    u32 ToPackedRGBA() const { return DWord;  }
    u32 ToPackedBGRA() const { return ((B << 24) | (G << 16) | (R << 8) | (A << 0)); }
    u32 ToPackedARGB() const { return ((A << 24) | (R << 16) | (G << 8) | (B << 0)); }
    u32 ToPackedABGR() const { return ((A << 24) | (B << 16) | (G << 8) | (R << 0)); }

    FLinearColor FromRGBE() const;
    FLinearColor ToLinear() const;

    static FColor FromTemperature(float kelvins, float a = 1.0f);

    inline friend bool operator ==(const FColor& lhs, const FColor& rhs) { return (lhs.DWord == rhs.DWord); }
    inline friend bool operator !=(const FColor& lhs, const FColor& rhs) { return (lhs.DWord != rhs.DWord); }

    inline hash_t hash_value(const FColor& color) { return (color.DWord); }

    static FColor AliceBlue();
    static FColor AntiqueWhite();
    static FColor Aqua();
    static FColor Aquamarine();
    static FColor Azure();
    static FColor Beige();
    static FColor Bisque();
    static FColor Black();
    static FColor BlanchedAlmond();
    static FColor Blue();
    static FColor BlueViolet();
    static FColor Brown();
    static FColor BurlyWood();
    static FColor CadetBlue();
    static FColor Chartreuse();
    static FColor Chocolate();
    static FColor Coral();
    static FColor CornflowerBlue();
    static FColor Cornsilk();
    static FColor Crimson();
    static FColor Cyan();
    static FColor DarkBlue();
    static FColor DarkCyan();
    static FColor DarkGoldenRod();
    static FColor DarkGray();
    static FColor DarkGreen();
    static FColor DarkKhaki();
    static FColor DarkMagenta();
    static FColor DarkOliveGreen();
    static FColor DarkOrange();
    static FColor DarkOrchid();
    static FColor DarkRed();
    static FColor DarkSalmon();
    static FColor DarkSeaGreen();
    static FColor DarkSlateBlue();
    static FColor DarkSlateGray();
    static FColor DarkTurquoise();
    static FColor DarkViolet();
    static FColor DeepPink();
    static FColor DeepSkyBlue();
    static FColor DimGray();
    static FColor DodgerBlue();
    static FColor FireBrick();
    static FColor FloralWhite();
    static FColor ForestGreen();
    static FColor Fuchsia();
    static FColor Gainsboro();
    static FColor GhostWhite();
    static FColor Gold();
    static FColor GoldenRod();
    static FColor Gray();
    static FColor Green();
    static FColor GreenYellow();
    static FColor HoneyDew();
    static FColor HotPink();
    static FColor IndianRed();
    static FColor Indigo();
    static FColor Ivory();
    static FColor Khaki();
    static FColor Lavender();
    static FColor LavenderBlush();
    static FColor LawnGreen();
    static FColor LemonChiffon();
    static FColor LightBlue();
    static FColor LightCoral();
    static FColor LightCyan();
    static FColor LightGoldenRodYellow();
    static FColor LightGray();
    static FColor LightGreen();
    static FColor LightPink();
    static FColor LightSalmon();
    static FColor LightSeaGreen();
    static FColor LightSkyBlue();
    static FColor LightSlateGray();
    static FColor LightSteelBlue();
    static FColor LightYellow();
    static FColor Lime();
    static FColor LimeGreen();
    static FColor Linen();
    static FColor Magenta();
    static FColor Maroon();
    static FColor MediumAquamarine();
    static FColor MediumBlue();
    static FColor MediumOrchid();
    static FColor MediumPurple();
    static FColor MediumSeaGreen();
    static FColor MediumSlateBlue();
    static FColor MediumSpringGreen();
    static FColor MediumTurquoise();
    static FColor MediumVioletRed();
    static FColor MidnightBlue();
    static FColor MintCream();
    static FColor MistyRose();
    static FColor Moccasin();
    static FColor NavajoWhite();
    static FColor Navy();
    static FColor OldLace();
    static FColor Olive();
    static FColor OliveDrab();
    static FColor Orange();
    static FColor OrangeRed();
    static FColor Orchid();
    static FColor PaleGoldenRod();
    static FColor PaleGreen();
    static FColor PaleTurquoise();
    static FColor PaleVioletRed();
    static FColor PapayaWhip();
    static FColor PeachPuff();
    static FColor Peru();
    static FColor Pink();
    static FColor Plum();
    static FColor PowderBlue();
    static FColor Purple();
    static FColor Red();
    static FColor RosyBrown();
    static FColor RoyalBlue();
    static FColor SaddleBrown();
    static FColor Salmon();
    static FColor SandyBrown();
    static FColor SeaGreen();
    static FColor SeaShell();
    static FColor Sienna();
    static FColor Silver();
    static FColor SkyBlue();
    static FColor SlateBlue();
    static FColor SlateGray();
    static FColor Snow();
    static FColor SpringGreen();
    static FColor SteelBlue();
    static FColor Tan();
    static FColor Teal();
    static FColor Thistle();
    static FColor Tomato();
    static FColor Transparent();
    static FColor Turquoise();
    static FColor Violet();
    static FColor Wheat();
    static FColor White();
    static FColor WhiteSmoke();
    static FColor Yellow();
    static FColor YellowGreen();
};
CORE_ASSUME_TYPE_AS_POD(FColor)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Color/Color-inl.h"
