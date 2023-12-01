#pragma once

#include "Core.h"

#include "Color/Color_fwd.h"
#include "Maths/ScalarVector_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGammaSpace : u8 {
    /** No gamma correction is applied to this space, the incoming colors are assumed to already be in linear space. */
    Linear,
    /** A simplified sRGB gamma correction is applied, pow(1/2.2). */
    Pow22,
    /** Use the standard sRGB conversion. */
    sRGB,
    /** Use the new ACES standard for HDR values. */
    ACES,
};
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API float3 ACESFitted(float3 linear);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API float3 Pastelizer(float hue);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API float3 Hue_to_RGB(float hue);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API float3 RGB_to_HCV(const float3& rgb);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API float3 HSV_to_RGB(const float3& hsv);
NODISCARD PPE_CORE_API float3 RGB_to_HSV(const float3& rgb);
NODISCARD PPE_CORE_API float3 HSV_to_RGB_smooth(const float3& hsv);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API float3 HSL_to_RGB(const float3& hsl);
NODISCARD PPE_CORE_API float3 RGB_to_HSL(const float3& rgb);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API float3 YCoCg_to_RGB(const float3& yCoCg);
NODISCARD PPE_CORE_API float3 RGB_to_YCoCg(const float3& rgb);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API float3 RGB_to_OKLab(const float3& lin);
NODISCARD PPE_CORE_API float3 OKLab_to_RGB(const float3& oklab);
//----------------------------------------------------------------------------
float SRGB_to_Linear(float srgb);
NODISCARD PPE_CORE_API float SRGB_to_Linear(u8 srgb); // optimized version with a precomputed table
float Linear_to_SRGB(float lin);
//----------------------------------------------------------------------------
float Linear_to_Pow22(float lin);
float Pow22_to_Linear(float pow22);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FLinearColor {
    float R, G, B, A;

    FLinearColor() = default;

    CONSTEXPR explicit FLinearColor(Meta::FZeroValue) NOEXCEPT : R(0), G(0), B(0), A(0) {}
    CONSTEXPR explicit FLinearColor(Meta::FForceInit) NOEXCEPT : FLinearColor(Zero) {}
    CONSTEXPR explicit FLinearColor(Meta::FNoInit) NOEXCEPT {}
    CONSTEXPR FLinearColor(float r, float g, float b, float a = 1.0f) NOEXCEPT : R(r), G(g), B(b), A(a) {}

    PPE_CORE_API FLinearColor(const FColor& color);
    PPE_CORE_API FLinearColor(const float3& rgb, float a = 1.0f);
    PPE_CORE_API FLinearColor(const float4& rgba);

    NODISCARD operator const float3&() const { return *reinterpret_cast<const float3*>(this); }
    NODISCARD operator const float4&() const { return *reinterpret_cast<const float4*>(this); }

    NODISCARD float& operator [](size_t index) { Assert(index < 4); return (&R)[index]; }
    NODISCARD float operator [](size_t index) const { Assert(index < 4); return (&R)[index]; }

    NODISCARD PPE_CORE_API  FLinearColor Desaturate(float desaturation) const;
    NODISCARD CONSTEXPR FLinearColor Fade(float alpha) const { return FLinearColor(R, G, B, alpha); }

    NODISCARD CONSTEXPR float Luminance() const { return (0.2126f * R + 0.7152f * G + 0.0722f * B); }
    NODISCARD PPE_CORE_API FLinearColor SetLuminance(float lum) const;

    NODISCARD PPE_CORE_API float4 ToACES() const;
    NODISCARD PPE_CORE_API float4 ToBGRA() const;
    NODISCARD PPE_CORE_API float3 ToHSL() const;
    NODISCARD PPE_CORE_API float3 ToHSV() const;
    NODISCARD PPE_CORE_API float3 ToYCoCg() const;

    NODISCARD PPE_CORE_API FColor ToRGBE() const;
    NODISCARD PPE_CORE_API FColor Quantize(EGammaSpace gamma) const;

    NODISCARD PPE_CORE_API static FLinearColor FromLuma(float luma, float a = 1.0f);
    NODISCARD PPE_CORE_API static FLinearColor FromPastel(float hue, float a = 1.0f);
    NODISCARD PPE_CORE_API static FLinearColor FromHue(float hue, float a = 1.0f);
    NODISCARD PPE_CORE_API static FLinearColor FromHSL(const float3& hsl, float a = 1.0f);
    NODISCARD PPE_CORE_API static FLinearColor FromHSV(const float3& hsv, float a = 1.0f);
    NODISCARD PPE_CORE_API static FLinearColor FromHSV_smooth(const float3& hsv, float a = 1.0f);
    NODISCARD PPE_CORE_API static FLinearColor FromYCoCg(const float3& yCoCg, float a = 1.0f);
    NODISCARD PPE_CORE_API static FLinearColor FromHash(hash_t h, float a = 1.0f);
    NODISCARD PPE_CORE_API static FLinearColor FromHeatmap(float x, float a = 1.0f);
    NODISCARD PPE_CORE_API static FLinearColor FromTemperature(float kelvins, float a = 1.0f);

    NODISCARD CONSTEXPR inline friend bool operator ==(const FLinearColor& lhs, const FLinearColor& rhs) {
        return (lhs.R == rhs.R && lhs.G == rhs.G && lhs.B == rhs.B && lhs.A == rhs.A);
    }
    NODISCARD CONSTEXPR inline friend bool operator !=(const FLinearColor& lhs, const FLinearColor& rhs) {
        return (not operator ==(lhs, rhs));
    }

    static CONSTEXPR FLinearColor PaperWhite() {
        return { 1.f, 1.f, 1.f, 1.f };
    }

    NODISCARD PPE_CORE_API static FLinearColor AliceBlue();
    NODISCARD PPE_CORE_API static FLinearColor AntiqueWhite();
    NODISCARD PPE_CORE_API static FLinearColor Aqua();
    NODISCARD PPE_CORE_API static FLinearColor Aquamarine();
    NODISCARD PPE_CORE_API static FLinearColor Azure();
    NODISCARD PPE_CORE_API static FLinearColor Beige();
    NODISCARD PPE_CORE_API static FLinearColor Bisque();
    NODISCARD PPE_CORE_API static FLinearColor Black();
    NODISCARD PPE_CORE_API static FLinearColor BlanchedAlmond();
    NODISCARD PPE_CORE_API static FLinearColor Blue();
    NODISCARD PPE_CORE_API static FLinearColor BlueViolet();
    NODISCARD PPE_CORE_API static FLinearColor Brown();
    NODISCARD PPE_CORE_API static FLinearColor BurlyWood();
    NODISCARD PPE_CORE_API static FLinearColor CadetBlue();
    NODISCARD PPE_CORE_API static FLinearColor Chartreuse();
    NODISCARD PPE_CORE_API static FLinearColor Chocolate();
    NODISCARD PPE_CORE_API static FLinearColor Coral();
    NODISCARD PPE_CORE_API static FLinearColor CornflowerBlue();
    NODISCARD PPE_CORE_API static FLinearColor Cornsilk();
    NODISCARD PPE_CORE_API static FLinearColor Crimson();
    NODISCARD PPE_CORE_API static FLinearColor Cyan();
    NODISCARD PPE_CORE_API static FLinearColor DarkBlue();
    NODISCARD PPE_CORE_API static FLinearColor DarkCyan();
    NODISCARD PPE_CORE_API static FLinearColor DarkGoldenRod();
    NODISCARD PPE_CORE_API static FLinearColor DarkGray();
    NODISCARD PPE_CORE_API static FLinearColor DarkGreen();
    NODISCARD PPE_CORE_API static FLinearColor DarkKhaki();
    NODISCARD PPE_CORE_API static FLinearColor DarkMagenta();
    NODISCARD PPE_CORE_API static FLinearColor DarkOliveGreen();
    NODISCARD PPE_CORE_API static FLinearColor DarkOrange();
    NODISCARD PPE_CORE_API static FLinearColor DarkOrchid();
    NODISCARD PPE_CORE_API static FLinearColor DarkRed();
    NODISCARD PPE_CORE_API static FLinearColor DarkSalmon();
    NODISCARD PPE_CORE_API static FLinearColor DarkSeaGreen();
    NODISCARD PPE_CORE_API static FLinearColor DarkSlateBlue();
    NODISCARD PPE_CORE_API static FLinearColor DarkSlateGray();
    NODISCARD PPE_CORE_API static FLinearColor DarkTurquoise();
    NODISCARD PPE_CORE_API static FLinearColor DarkViolet();
    NODISCARD PPE_CORE_API static FLinearColor DeepPink();
    NODISCARD PPE_CORE_API static FLinearColor DeepSkyBlue();
    NODISCARD PPE_CORE_API static FLinearColor DimGray();
    NODISCARD PPE_CORE_API static FLinearColor DodgerBlue();
    NODISCARD PPE_CORE_API static FLinearColor FireBrick();
    NODISCARD PPE_CORE_API static FLinearColor FloralWhite();
    NODISCARD PPE_CORE_API static FLinearColor ForestGreen();
    NODISCARD PPE_CORE_API static FLinearColor Fuchsia();
    NODISCARD PPE_CORE_API static FLinearColor Gainsboro();
    NODISCARD PPE_CORE_API static FLinearColor GhostWhite();
    NODISCARD PPE_CORE_API static FLinearColor Gold();
    NODISCARD PPE_CORE_API static FLinearColor GoldenRod();
    NODISCARD PPE_CORE_API static FLinearColor Gray();
    NODISCARD PPE_CORE_API static FLinearColor Green();
    NODISCARD PPE_CORE_API static FLinearColor GreenYellow();
    NODISCARD PPE_CORE_API static FLinearColor HoneyDew();
    NODISCARD PPE_CORE_API static FLinearColor HotPink();
    NODISCARD PPE_CORE_API static FLinearColor IndianRed();
    NODISCARD PPE_CORE_API static FLinearColor Indigo();
    NODISCARD PPE_CORE_API static FLinearColor Ivory();
    NODISCARD PPE_CORE_API static FLinearColor Khaki();
    NODISCARD PPE_CORE_API static FLinearColor Lavender();
    NODISCARD PPE_CORE_API static FLinearColor LavenderBlush();
    NODISCARD PPE_CORE_API static FLinearColor LawnGreen();
    NODISCARD PPE_CORE_API static FLinearColor LemonChiffon();
    NODISCARD PPE_CORE_API static FLinearColor LightBlue();
    NODISCARD PPE_CORE_API static FLinearColor LightCoral();
    NODISCARD PPE_CORE_API static FLinearColor LightCyan();
    NODISCARD PPE_CORE_API static FLinearColor LightGoldenRodYellow();
    NODISCARD PPE_CORE_API static FLinearColor LightGray();
    NODISCARD PPE_CORE_API static FLinearColor LightGreen();
    NODISCARD PPE_CORE_API static FLinearColor LightPink();
    NODISCARD PPE_CORE_API static FLinearColor LightSalmon();
    NODISCARD PPE_CORE_API static FLinearColor LightSeaGreen();
    NODISCARD PPE_CORE_API static FLinearColor LightSkyBlue();
    NODISCARD PPE_CORE_API static FLinearColor LightSlateGray();
    NODISCARD PPE_CORE_API static FLinearColor LightSteelBlue();
    NODISCARD PPE_CORE_API static FLinearColor LightYellow();
    NODISCARD PPE_CORE_API static FLinearColor Lime();
    NODISCARD PPE_CORE_API static FLinearColor LimeGreen();
    NODISCARD PPE_CORE_API static FLinearColor Linen();
    NODISCARD PPE_CORE_API static FLinearColor Magenta();
    NODISCARD PPE_CORE_API static FLinearColor Maroon();
    NODISCARD PPE_CORE_API static FLinearColor MediumAquamarine();
    NODISCARD PPE_CORE_API static FLinearColor MediumBlue();
    NODISCARD PPE_CORE_API static FLinearColor MediumOrchid();
    NODISCARD PPE_CORE_API static FLinearColor MediumPurple();
    NODISCARD PPE_CORE_API static FLinearColor MediumSeaGreen();
    NODISCARD PPE_CORE_API static FLinearColor MediumSlateBlue();
    NODISCARD PPE_CORE_API static FLinearColor MediumSpringGreen();
    NODISCARD PPE_CORE_API static FLinearColor MediumTurquoise();
    NODISCARD PPE_CORE_API static FLinearColor MediumVioletRed();
    NODISCARD PPE_CORE_API static FLinearColor MidnightBlue();
    NODISCARD PPE_CORE_API static FLinearColor MintCream();
    NODISCARD PPE_CORE_API static FLinearColor MistyRose();
    NODISCARD PPE_CORE_API static FLinearColor Moccasin();
    NODISCARD PPE_CORE_API static FLinearColor NavajoWhite();
    NODISCARD PPE_CORE_API static FLinearColor Navy();
    NODISCARD PPE_CORE_API static FLinearColor OldLace();
    NODISCARD PPE_CORE_API static FLinearColor Olive();
    NODISCARD PPE_CORE_API static FLinearColor OliveDrab();
    NODISCARD PPE_CORE_API static FLinearColor Orange();
    NODISCARD PPE_CORE_API static FLinearColor OrangeRed();
    NODISCARD PPE_CORE_API static FLinearColor Orchid();
    NODISCARD PPE_CORE_API static FLinearColor PaleGoldenRod();
    NODISCARD PPE_CORE_API static FLinearColor PaleGreen();
    NODISCARD PPE_CORE_API static FLinearColor PaleTurquoise();
    NODISCARD PPE_CORE_API static FLinearColor PaleVioletRed();
    NODISCARD PPE_CORE_API static FLinearColor PapayaWhip();
    NODISCARD PPE_CORE_API static FLinearColor PeachPuff();
    NODISCARD PPE_CORE_API static FLinearColor Peru();
    NODISCARD PPE_CORE_API static FLinearColor Pink();
    NODISCARD PPE_CORE_API static FLinearColor Plum();
    NODISCARD PPE_CORE_API static FLinearColor PowderBlue();
    NODISCARD PPE_CORE_API static FLinearColor Purple();
    NODISCARD PPE_CORE_API static FLinearColor Red();
    NODISCARD PPE_CORE_API static FLinearColor RosyBrown();
    NODISCARD PPE_CORE_API static FLinearColor RoyalBlue();
    NODISCARD PPE_CORE_API static FLinearColor SaddleBrown();
    NODISCARD PPE_CORE_API static FLinearColor Salmon();
    NODISCARD PPE_CORE_API static FLinearColor SandyBrown();
    NODISCARD PPE_CORE_API static FLinearColor SeaGreen();
    NODISCARD PPE_CORE_API static FLinearColor SeaShell();
    NODISCARD PPE_CORE_API static FLinearColor Sienna();
    NODISCARD PPE_CORE_API static FLinearColor Silver();
    NODISCARD PPE_CORE_API static FLinearColor SkyBlue();
    NODISCARD PPE_CORE_API static FLinearColor SlateBlue();
    NODISCARD PPE_CORE_API static FLinearColor SlateGray();
    NODISCARD PPE_CORE_API static FLinearColor Snow();
    NODISCARD PPE_CORE_API static FLinearColor SpringGreen();
    NODISCARD PPE_CORE_API static FLinearColor SteelBlue();
    NODISCARD PPE_CORE_API static FLinearColor Tan();
    NODISCARD PPE_CORE_API static FLinearColor Teal();
    NODISCARD PPE_CORE_API static FLinearColor Thistle();
    NODISCARD PPE_CORE_API static FLinearColor Tomato();
    NODISCARD PPE_CORE_API static FLinearColor Transparent();
    NODISCARD PPE_CORE_API static FLinearColor Turquoise();
    NODISCARD PPE_CORE_API static FLinearColor Violet();
    NODISCARD PPE_CORE_API static FLinearColor Wheat();
    NODISCARD PPE_CORE_API static FLinearColor White();
    NODISCARD PPE_CORE_API static FLinearColor WhiteSmoke();
    NODISCARD PPE_CORE_API static FLinearColor Yellow();
    NODISCARD PPE_CORE_API static FLinearColor YellowGreen();
};
PPE_ASSUME_TYPE_AS_POD(FLinearColor)
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FLinearColor AlphaBlend(const FLinearColor& dst, const FLinearColor& src);
FLinearColor Lerp(const FLinearColor& from, const FLinearColor& to, float t);
NODISCARD PPE_CORE_API FLinearColor LerpUsingHSV(const FLinearColor& from, const FLinearColor& to, float t);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FColor {
    union {
        struct { u8  A, B, G, R; };
        u32 DWord;
    };

    FColor() = default;

    CONSTEXPR explicit FColor(Meta::FZeroValue) NOEXCEPT : DWord(0) {}
    CONSTEXPR explicit FColor(Meta::FForceInit) NOEXCEPT : FColor(Zero) {}
    CONSTEXPR explicit FColor(Meta::FNoInit) NOEXCEPT {}
    CONSTEXPR explicit FColor(u32 dword) NOEXCEPT : DWord(dword) {}
    CONSTEXPR FColor(u8 r, u8 g, u8 b, u8 a = 0xFF) NOEXCEPT : A(a), B(b), G(g), R(r) {}

    PPE_CORE_API FColor(const ubyte3& rgb, u8 a = 1.0f);
    PPE_CORE_API FColor(const ubyte4& rgba);

    NODISCARD operator const ubyte3&() const { return *reinterpret_cast<const ubyte3*>(this); }
    NODISCARD operator const ubyte4&() const { return *reinterpret_cast<const ubyte4*>(this); }

    NODISCARD u8& operator [](size_t index) { Assert(index < 4); return (&R)[index]; }
    NODISCARD u8 operator [](size_t index) const { Assert(index < 4); return (&R)[index]; }

    NODISCARD CONSTEXPR FColor Fade(u8 alpha) const { return FColor(R, G, B, alpha); }

    NODISCARD u32 ToPackedRGBA() const { return DWord;  }
    NODISCARD u32 ToPackedBGRA() const { return ((B << 24) | (G << 16) | (R << 8) | (A << 0)); }
    NODISCARD u32 ToPackedARGB() const { return ((A << 24) | (R << 16) | (G << 8) | (B << 0)); }
    NODISCARD u32 ToPackedABGR() const { return ((A << 24) | (B << 16) | (G << 8) | (R << 0)); }

    NODISCARD PPE_CORE_API FLinearColor FromRGBE() const;
    NODISCARD PPE_CORE_API FLinearColor ToLinear() const;

    NODISCARD PPE_CORE_API static FColor FromHash(hash_t h, u8 a = 0xFF);
    NODISCARD PPE_CORE_API static FColor FromTemperature(float kelvins, float a = 1.0f, EGammaSpace gamma = EGammaSpace::sRGB);

    NODISCARD CONSTEXPR inline friend bool operator ==(const FColor& lhs, const FColor& rhs) { return (lhs.DWord == rhs.DWord); }
    NODISCARD CONSTEXPR inline friend bool operator !=(const FColor& lhs, const FColor& rhs) { return (lhs.DWord != rhs.DWord); }

    NODISCARD inline hash_t hash_value(const FColor& color) { return (color.DWord); }

    static CONSTEXPR FColor PaperWhite() {
        return { 0xFF, 0xFF, 0xFF, 0xFF };
    }

    NODISCARD PPE_CORE_API static FColor AliceBlue();
    NODISCARD PPE_CORE_API static FColor AntiqueWhite();
    NODISCARD PPE_CORE_API static FColor Aqua();
    NODISCARD PPE_CORE_API static FColor Aquamarine();
    NODISCARD PPE_CORE_API static FColor Azure();
    NODISCARD PPE_CORE_API static FColor Beige();
    NODISCARD PPE_CORE_API static FColor Bisque();
    NODISCARD PPE_CORE_API static FColor Black();
    NODISCARD PPE_CORE_API static FColor BlanchedAlmond();
    NODISCARD PPE_CORE_API static FColor Blue();
    NODISCARD PPE_CORE_API static FColor BlueViolet();
    NODISCARD PPE_CORE_API static FColor Brown();
    NODISCARD PPE_CORE_API static FColor BurlyWood();
    NODISCARD PPE_CORE_API static FColor CadetBlue();
    NODISCARD PPE_CORE_API static FColor Chartreuse();
    NODISCARD PPE_CORE_API static FColor Chocolate();
    NODISCARD PPE_CORE_API static FColor Coral();
    NODISCARD PPE_CORE_API static FColor CornflowerBlue();
    NODISCARD PPE_CORE_API static FColor Cornsilk();
    NODISCARD PPE_CORE_API static FColor Crimson();
    NODISCARD PPE_CORE_API static FColor Cyan();
    NODISCARD PPE_CORE_API static FColor DarkBlue();
    NODISCARD PPE_CORE_API static FColor DarkCyan();
    NODISCARD PPE_CORE_API static FColor DarkGoldenRod();
    NODISCARD PPE_CORE_API static FColor DarkGray();
    NODISCARD PPE_CORE_API static FColor DarkGreen();
    NODISCARD PPE_CORE_API static FColor DarkKhaki();
    NODISCARD PPE_CORE_API static FColor DarkMagenta();
    NODISCARD PPE_CORE_API static FColor DarkOliveGreen();
    NODISCARD PPE_CORE_API static FColor DarkOrange();
    NODISCARD PPE_CORE_API static FColor DarkOrchid();
    NODISCARD PPE_CORE_API static FColor DarkRed();
    NODISCARD PPE_CORE_API static FColor DarkSalmon();
    NODISCARD PPE_CORE_API static FColor DarkSeaGreen();
    NODISCARD PPE_CORE_API static FColor DarkSlateBlue();
    NODISCARD PPE_CORE_API static FColor DarkSlateGray();
    NODISCARD PPE_CORE_API static FColor DarkTurquoise();
    NODISCARD PPE_CORE_API static FColor DarkViolet();
    NODISCARD PPE_CORE_API static FColor DeepPink();
    NODISCARD PPE_CORE_API static FColor DeepSkyBlue();
    NODISCARD PPE_CORE_API static FColor DimGray();
    NODISCARD PPE_CORE_API static FColor DodgerBlue();
    NODISCARD PPE_CORE_API static FColor FireBrick();
    NODISCARD PPE_CORE_API static FColor FloralWhite();
    NODISCARD PPE_CORE_API static FColor ForestGreen();
    NODISCARD PPE_CORE_API static FColor Fuchsia();
    NODISCARD PPE_CORE_API static FColor Gainsboro();
    NODISCARD PPE_CORE_API static FColor GhostWhite();
    NODISCARD PPE_CORE_API static FColor Gold();
    NODISCARD PPE_CORE_API static FColor GoldenRod();
    NODISCARD PPE_CORE_API static FColor Gray();
    NODISCARD PPE_CORE_API static FColor Green();
    NODISCARD PPE_CORE_API static FColor GreenYellow();
    NODISCARD PPE_CORE_API static FColor HoneyDew();
    NODISCARD PPE_CORE_API static FColor HotPink();
    NODISCARD PPE_CORE_API static FColor IndianRed();
    NODISCARD PPE_CORE_API static FColor Indigo();
    NODISCARD PPE_CORE_API static FColor Ivory();
    NODISCARD PPE_CORE_API static FColor Khaki();
    NODISCARD PPE_CORE_API static FColor Lavender();
    NODISCARD PPE_CORE_API static FColor LavenderBlush();
    NODISCARD PPE_CORE_API static FColor LawnGreen();
    NODISCARD PPE_CORE_API static FColor LemonChiffon();
    NODISCARD PPE_CORE_API static FColor LightBlue();
    NODISCARD PPE_CORE_API static FColor LightCoral();
    NODISCARD PPE_CORE_API static FColor LightCyan();
    NODISCARD PPE_CORE_API static FColor LightGoldenRodYellow();
    NODISCARD PPE_CORE_API static FColor LightGray();
    NODISCARD PPE_CORE_API static FColor LightGreen();
    NODISCARD PPE_CORE_API static FColor LightPink();
    NODISCARD PPE_CORE_API static FColor LightSalmon();
    NODISCARD PPE_CORE_API static FColor LightSeaGreen();
    NODISCARD PPE_CORE_API static FColor LightSkyBlue();
    NODISCARD PPE_CORE_API static FColor LightSlateGray();
    NODISCARD PPE_CORE_API static FColor LightSteelBlue();
    NODISCARD PPE_CORE_API static FColor LightYellow();
    NODISCARD PPE_CORE_API static FColor Lime();
    NODISCARD PPE_CORE_API static FColor LimeGreen();
    NODISCARD PPE_CORE_API static FColor Linen();
    NODISCARD PPE_CORE_API static FColor Magenta();
    NODISCARD PPE_CORE_API static FColor Maroon();
    NODISCARD PPE_CORE_API static FColor MediumAquamarine();
    NODISCARD PPE_CORE_API static FColor MediumBlue();
    NODISCARD PPE_CORE_API static FColor MediumOrchid();
    NODISCARD PPE_CORE_API static FColor MediumPurple();
    NODISCARD PPE_CORE_API static FColor MediumSeaGreen();
    NODISCARD PPE_CORE_API static FColor MediumSlateBlue();
    NODISCARD PPE_CORE_API static FColor MediumSpringGreen();
    NODISCARD PPE_CORE_API static FColor MediumTurquoise();
    NODISCARD PPE_CORE_API static FColor MediumVioletRed();
    NODISCARD PPE_CORE_API static FColor MidnightBlue();
    NODISCARD PPE_CORE_API static FColor MintCream();
    NODISCARD PPE_CORE_API static FColor MistyRose();
    NODISCARD PPE_CORE_API static FColor Moccasin();
    NODISCARD PPE_CORE_API static FColor NavajoWhite();
    NODISCARD PPE_CORE_API static FColor Navy();
    NODISCARD PPE_CORE_API static FColor OldLace();
    NODISCARD PPE_CORE_API static FColor Olive();
    NODISCARD PPE_CORE_API static FColor OliveDrab();
    NODISCARD PPE_CORE_API static FColor Orange();
    NODISCARD PPE_CORE_API static FColor OrangeRed();
    NODISCARD PPE_CORE_API static FColor Orchid();
    NODISCARD PPE_CORE_API static FColor PaleGoldenRod();
    NODISCARD PPE_CORE_API static FColor PaleGreen();
    NODISCARD PPE_CORE_API static FColor PaleTurquoise();
    NODISCARD PPE_CORE_API static FColor PaleVioletRed();
    NODISCARD PPE_CORE_API static FColor PapayaWhip();
    NODISCARD PPE_CORE_API static FColor PeachPuff();
    NODISCARD PPE_CORE_API static FColor Peru();
    NODISCARD PPE_CORE_API static FColor Pink();
    NODISCARD PPE_CORE_API static FColor Plum();
    NODISCARD PPE_CORE_API static FColor PowderBlue();
    NODISCARD PPE_CORE_API static FColor Purple();
    NODISCARD PPE_CORE_API static FColor Red();
    NODISCARD PPE_CORE_API static FColor RosyBrown();
    NODISCARD PPE_CORE_API static FColor RoyalBlue();
    NODISCARD PPE_CORE_API static FColor SaddleBrown();
    NODISCARD PPE_CORE_API static FColor Salmon();
    NODISCARD PPE_CORE_API static FColor SandyBrown();
    NODISCARD PPE_CORE_API static FColor SeaGreen();
    NODISCARD PPE_CORE_API static FColor SeaShell();
    NODISCARD PPE_CORE_API static FColor Sienna();
    NODISCARD PPE_CORE_API static FColor Silver();
    NODISCARD PPE_CORE_API static FColor SkyBlue();
    NODISCARD PPE_CORE_API static FColor SlateBlue();
    NODISCARD PPE_CORE_API static FColor SlateGray();
    NODISCARD PPE_CORE_API static FColor Snow();
    NODISCARD PPE_CORE_API static FColor SpringGreen();
    NODISCARD PPE_CORE_API static FColor SteelBlue();
    NODISCARD PPE_CORE_API static FColor Tan();
    NODISCARD PPE_CORE_API static FColor Teal();
    NODISCARD PPE_CORE_API static FColor Thistle();
    NODISCARD PPE_CORE_API static FColor Tomato();
    NODISCARD PPE_CORE_API static FColor Transparent();
    NODISCARD PPE_CORE_API static FColor Turquoise();
    NODISCARD PPE_CORE_API static FColor Violet();
    NODISCARD PPE_CORE_API static FColor Wheat();
    NODISCARD PPE_CORE_API static FColor White();
    NODISCARD PPE_CORE_API static FColor WhiteSmoke();
    NODISCARD PPE_CORE_API static FColor Yellow();
    NODISCARD PPE_CORE_API static FColor YellowGreen();
};
PPE_ASSUME_TYPE_AS_POD(FColor)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Color/Color-inl.h"
