#include "stdafx.h"

#include "Color/Color.h"

#include <algorithm>

#include "Maths/MathHelpers.h"
#include "Maths/PackingHelpers.h"
#include "Maths/PackedVectors.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Memory/MemoryView.h"

namespace PPE {

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_ASSERT_TYPE_IS_POD(FColor);
PPE_ASSERT_TYPE_IS_POD(FLinearColor);
//----------------------------------------------------------------------------
const FLinearColor FLinearColor::PaperWhite(1.f, 1.f, 1.f, 1.f);
//----------------------------------------------------------------------------
FLinearColor::FLinearColor(const FColor& color)
    : FLinearColor(
        UByte0255_to_Float01(color.R),
        UByte0255_to_Float01(color.G),
        UByte0255_to_Float01(color.B),
        UByte0255_to_Float01(color.A) ) {}
//----------------------------------------------------------------------------
FLinearColor::FLinearColor(const float3& rgb, float a/* = 1.0f */)
    : FLinearColor(rgb.x, rgb.y, rgb.z, a) {}
//----------------------------------------------------------------------------
FLinearColor::FLinearColor(const float4& rgba)
    : FLinearColor(rgba.x, rgba.y, rgba.z, rgba.w) {}
//----------------------------------------------------------------------------
FLinearColor FLinearColor::Desaturate(float desaturation) const {
    const float luma = Luminance();
    return FLinearColor(
        Lerp(R, luma, desaturation),
        Lerp(G, luma, desaturation),
        Lerp(B, luma, desaturation),
        A );
}
//----------------------------------------------------------------------------
FLinearColor FLinearColor::SetLuminance(float lum) const {
    lum /= (F_SmallEpsilon + Luminance()); // normalize with old luminance and rescale with new luminance
    return FLinearColor{ R * lum, G * lum, B * lum, A };
}
//----------------------------------------------------------------------------
float4 FLinearColor::ToBGRA() const {
    return float4(B, G, R, A);
}
//----------------------------------------------------------------------------
float3 FLinearColor::ToHSL() const {
    return RGB_to_HSL(*this);
}
//----------------------------------------------------------------------------
float3 FLinearColor::ToHSV() const {
    return RGB_to_HSV(*this);
}
//----------------------------------------------------------------------------
float3 FLinearColor::ToYCoCg() const {
    return RGB_to_YCoCg(*this);
}
//----------------------------------------------------------------------------
FColor FLinearColor::ToRGBE() const {
    const float primary = Max3(R, G, B);
    if (primary < 1e-32f) {
        return FColor(0, 0, 0, 0);
    }
    else {
        int exponent;
        float scale = std::frexp(primary, &exponent) / primary;
        return FColor(
            Float01_to_UByte0255(R * scale),
            Float01_to_UByte0255(G * scale),
            Float01_to_UByte0255(B * scale),
            checked_cast<u8>(Clamp(exponent, -128, 127) + 128) );
    }
}
//----------------------------------------------------------------------------
FColor FLinearColor::Quantize(EGammaSpace gamma) const {
    float r = R, g = G, b = B;

    switch (gamma)
    {
    case PPE::EGammaSpace::Linear:
        r = Saturate(r);
        g = Saturate(g);
        b = Saturate(b);
        break;
    case PPE::EGammaSpace::Pow22:
        r = Linear_to_Pow22(r);
        g = Linear_to_Pow22(g);
        b = Linear_to_Pow22(b);
        break;
    case PPE::EGammaSpace::sRGB:
        r = Linear_to_SRGB(r);
        g = Linear_to_SRGB(g);
        b = Linear_to_SRGB(b);
        break;
    case PPE::EGammaSpace::ACES:
        {
            const float3 c = ACESFitted(float3{ R, G, B });
            r = c.x; g = c.y; b = c.z;
        }
        break;
    default:
        AssertNotImplemented();
        break;
    }

    return FColor(
        Float01_to_UByte0255(r),
        Float01_to_UByte0255(g),
        Float01_to_UByte0255(b),
        Float01_to_UByte0255(A) );
}
//----------------------------------------------------------------------------
FLinearColor FLinearColor::FromPastel(float hue, float a/* = 1.0f */) {
    return FLinearColor(Pastelizer(hue), a);
}
//----------------------------------------------------------------------------
FLinearColor FLinearColor::FromHue(float hue, float a/* = 1.0f */) {
    return FLinearColor(Hue_to_RGB(hue), a);
}
//----------------------------------------------------------------------------
FLinearColor FLinearColor::FromHSL(const float3& hsl, float a/* = 1.0f */) {
    return FLinearColor(HSL_to_RGB(hsl), a);
}
//----------------------------------------------------------------------------
FLinearColor FLinearColor::FromHSV(const float3& hsv, float a/* = 1.0f */) {
    return FLinearColor(HSV_to_RGB(hsv), a);
}
//----------------------------------------------------------------------------
FLinearColor FLinearColor::FromHSV_smooth(const float3& hsv, float a/* = 1.0f */) {
    return FLinearColor(HSV_to_RGB_smooth(hsv), a);
}
//----------------------------------------------------------------------------
FLinearColor FLinearColor::FromYCoCg(const float3& yCoCg, float a/* = 1.0f */) {
    return FLinearColor(YCoCg_to_RGB(yCoCg), a);
}
//----------------------------------------------------------------------------
FLinearColor FLinearColor::FromTemperature(float kelvins, float a/* = 1.0f */) {
    kelvins = Clamp(kelvins, 1000.0f, 15000.0f);

    // Approximate Planckian locus in CIE 1960 UCS
    float u = (0.860117757f + 1.54118254e-4f * kelvins + 1.28641212e-7f * kelvins*kelvins) / (1.0f + 8.42420235e-4f * kelvins + 7.08145163e-7f * kelvins*kelvins);
    float v = (0.317398726f + 4.22806245e-5f * kelvins + 4.20481691e-8f * kelvins*kelvins) / (1.0f - 2.89741816e-5f * kelvins + 1.61456053e-7f * kelvins*kelvins);

    float x = 3.0f * u / (2.0f * u - 8.0f * v + 4.0f);
    float y = 2.0f * v / (2.0f * u - 8.0f * v + 4.0f);
    float z = 1.0f - x - y;

    float Y = 1.0f;
    float X = Y / y * x;
    float Z = Y / y * z;

    // XYZ to RGB with BT.709 primaries
    float R = 3.2404542f * X + -1.5371385f * Y + -0.4985314f * Z;
    float G =-0.9692660f * X +  1.8760108f * Y +  0.0415560f * Z;
    float B = 0.0556434f * X + -0.2040259f * Y +  1.0572252f * Z;

    return FLinearColor(R, G, B, a);
}
//----------------------------------------------------------------------------
FLinearColor AlphaBlend(const FLinearColor& dst, const FLinearColor& src) {
    if (src.A > 0 && dst.A == 0)
        return src;
    else if (src.A == 0)
        return dst;
    else {
        const float ac = (1.0f - src.A) * dst.A;
        return FLinearColor(
            src.R * src.A + dst.R * ac,
            src.G * src.A + dst.G * ac,
            src.B * src.A + dst.B * ac,
            src.A + ac );
    }
}
//----------------------------------------------------------------------------
FLinearColor LerpUsingHSV(const FLinearColor& from, const FLinearColor& to, float t) {
    return FLinearColor(
        HSV_to_RGB(Lerp(from.ToHSV(), to.ToHSV(), t)),
        Lerp(from.A, to.A, t) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FColor FColor::PaperWhite(0xFF, 0xFF, 0xFF, 0xFF);
//----------------------------------------------------------------------------
FColor::FColor(const ubyte3& rgb, u8 a/* = 1.0f */)
    : R(rgb.x), G(rgb.y), B(rgb.z), A(a) {}
//----------------------------------------------------------------------------
FColor::FColor(const ubyte4& rgba)
    : R(rgba.x), G(rgba.y), B(rgba.z), A(rgba.w) {}
//----------------------------------------------------------------------------
FLinearColor FColor::FromRGBE() const {
    if (A == 0) {
        return FLinearColor(0,0,0,0);
    }
    else {
        const float Scale = std::ldexp(1 / 255.0f, A - 128);
        return FLinearColor(R * Scale, G * Scale, B * Scale, 1.0f);
    }
}
//----------------------------------------------------------------------------
FLinearColor FColor::ToLinear() const {
    return FLinearColor(
        SRGB_to_Linear(R),
        SRGB_to_Linear(G),
        SRGB_to_Linear(B),
        UByte0255_to_Float01(A) );
}
//----------------------------------------------------------------------------
FColor FColor::FromTemperature(float kelvins, float a/* = 1.0f */, EGammaSpace gamma/* = EGammaSpace::sRGB */) {
    return FLinearColor::FromTemperature(kelvins, a).Quantize(gamma);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// precomputed with the float version of SRGB_to_Linear() :
static CONSTEXPR const float GSRGB_to_Linear[256] = {
0.000000e+00f, 3.035270e-04f, 6.070540e-04f, 9.105810e-04f, 1.214108e-03f, 1.517635e-03f, 1.821162e-03f, 2.124689e-03f,
2.428216e-03f, 2.731743e-03f, 3.035270e-03f, 3.346536e-03f, 3.676507e-03f, 4.024717e-03f, 4.391442e-03f, 4.776953e-03f,
5.181517e-03f, 5.605391e-03f, 6.048833e-03f, 6.512091e-03f, 6.995410e-03f, 7.499032e-03f, 8.023193e-03f, 8.568125e-03f,
9.134058e-03f, 9.721217e-03f, 1.032982e-02f, 1.096009e-02f, 1.161224e-02f, 1.228649e-02f, 1.298303e-02f, 1.370208e-02f,
1.444384e-02f, 1.520851e-02f, 1.599629e-02f, 1.680738e-02f, 1.764195e-02f, 1.850022e-02f, 1.938236e-02f, 2.028856e-02f,
2.121901e-02f, 2.217388e-02f, 2.315337e-02f, 2.415763e-02f, 2.518686e-02f, 2.624122e-02f, 2.732089e-02f, 2.842604e-02f,
2.955683e-02f, 3.071344e-02f, 3.189603e-02f, 3.310477e-02f, 3.433981e-02f, 3.560131e-02f, 3.688945e-02f, 3.820437e-02f,
3.954623e-02f, 4.091520e-02f, 4.231141e-02f, 4.373503e-02f, 4.518620e-02f, 4.666509e-02f, 4.817182e-02f, 4.970657e-02f,
5.126946e-02f, 5.286065e-02f, 5.448028e-02f, 5.612849e-02f, 5.780543e-02f, 5.951124e-02f, 6.124605e-02f, 6.301002e-02f,
6.480327e-02f, 6.662594e-02f, 6.847817e-02f, 7.036009e-02f, 7.227185e-02f, 7.421357e-02f, 7.618538e-02f, 7.818742e-02f,
8.021982e-02f, 8.228271e-02f, 8.437621e-02f, 8.650046e-02f, 8.865559e-02f, 9.084171e-02f, 9.305896e-02f, 9.530747e-02f,
9.758735e-02f, 9.989873e-02f, 1.022417e-01f, 1.046165e-01f, 1.070231e-01f, 1.094617e-01f, 1.119324e-01f, 1.144354e-01f,
1.169707e-01f, 1.195384e-01f, 1.221388e-01f, 1.247718e-01f, 1.274377e-01f, 1.301365e-01f, 1.328683e-01f, 1.356333e-01f,
1.384316e-01f, 1.412633e-01f, 1.441285e-01f, 1.470273e-01f, 1.499598e-01f, 1.529262e-01f, 1.559265e-01f, 1.589608e-01f,
1.620294e-01f, 1.651322e-01f, 1.682694e-01f, 1.714411e-01f, 1.746474e-01f, 1.778884e-01f, 1.811642e-01f, 1.844750e-01f,
1.878208e-01f, 1.912017e-01f, 1.946178e-01f, 1.980693e-01f, 2.015563e-01f, 2.050787e-01f, 2.086369e-01f, 2.122308e-01f,
2.158605e-01f, 2.195262e-01f, 2.232280e-01f, 2.269659e-01f, 2.307400e-01f, 2.345506e-01f, 2.383976e-01f, 2.422811e-01f,
2.462013e-01f, 2.501583e-01f, 2.541521e-01f, 2.581829e-01f, 2.622507e-01f, 2.663556e-01f, 2.704978e-01f, 2.746773e-01f,
2.788943e-01f, 2.831487e-01f, 2.874408e-01f, 2.917706e-01f, 2.961383e-01f, 3.005438e-01f, 3.049873e-01f, 3.094689e-01f,
3.139887e-01f, 3.185468e-01f, 3.231432e-01f, 3.277781e-01f, 3.324515e-01f, 3.371636e-01f, 3.419144e-01f, 3.467041e-01f,
3.515326e-01f, 3.564001e-01f, 3.613068e-01f, 3.662526e-01f, 3.712377e-01f, 3.762621e-01f, 3.813260e-01f, 3.864294e-01f,
3.915725e-01f, 3.967552e-01f, 4.019778e-01f, 4.072402e-01f, 4.125426e-01f, 4.178851e-01f, 4.232677e-01f, 4.286905e-01f,
4.341536e-01f, 4.396572e-01f, 4.452012e-01f, 4.507858e-01f, 4.564110e-01f, 4.620770e-01f, 4.677838e-01f, 4.735315e-01f,
4.793202e-01f, 4.851499e-01f, 4.910208e-01f, 4.969330e-01f, 5.028865e-01f, 5.088813e-01f, 5.149177e-01f, 5.209956e-01f,
5.271151e-01f, 5.332764e-01f, 5.394795e-01f, 5.457245e-01f, 5.520114e-01f, 5.583404e-01f, 5.647115e-01f, 5.711248e-01f,
5.775804e-01f, 5.840784e-01f, 5.906188e-01f, 5.972018e-01f, 6.038273e-01f, 6.104956e-01f, 6.172066e-01f, 6.239604e-01f,
6.307571e-01f, 6.375969e-01f, 6.444797e-01f, 6.514056e-01f, 6.583748e-01f, 6.653873e-01f, 6.724432e-01f, 6.795425e-01f,
6.866853e-01f, 6.938718e-01f, 7.011019e-01f, 7.083758e-01f, 7.156935e-01f, 7.230551e-01f, 7.304607e-01f, 7.379104e-01f,
7.454042e-01f, 7.529422e-01f, 7.605245e-01f, 7.681511e-01f, 7.758222e-01f, 7.835378e-01f, 7.912979e-01f, 7.991027e-01f,
8.069523e-01f, 8.148466e-01f, 8.227858e-01f, 8.307699e-01f, 8.387990e-01f, 8.468732e-01f, 8.549926e-01f, 8.631572e-01f,
8.713671e-01f, 8.796224e-01f, 8.879231e-01f, 8.962694e-01f, 9.046612e-01f, 9.130986e-01f, 9.215819e-01f, 9.301109e-01f,
9.386857e-01f, 9.473065e-01f, 9.559733e-01f, 9.646862e-01f, 9.734453e-01f, 9.822505e-01f, 9.911021e-01f, 1.000000e+00f,
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
float SRGB_to_Linear(u8 srgb) {
    return GSRGB_to_Linear[srgb];
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// ACES fitted aka HDR scaling
// from https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
//----------------------------------------------------------------------------
static const float3x3 ACESInputMat {
    0.59719f, 0.35458f, 0.04823f,
    0.07600f, 0.90834f, 0.01566f,
    0.02840f, 0.13383f, 0.83777f
};
//----------------------------------------------------------------------------
// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat {
     1.60475f, -0.53108f, -0.07367f,
    -0.10208f,  1.10813f, -0.00605f,
    -0.00327f, -0.07276f,  1.07602f
};
//----------------------------------------------------------------------------
static CONSTEXPR float3 RRTAndODTFit(float3 v) {
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}
//----------------------------------------------------------------------------
float3 ACESFitted(float3 linear) {
    linear = ACESInputMat.Multiply(linear);

    // Apply RRT and ODT
    linear = RRTAndODTFit(linear);

    linear = ACESOutputMat.Multiply(linear);

    // Clamp to [0, 1]
    linear = Clamp(linear, 0.0f, 1.0f);

    return linear;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// https://www.shadertoy.com/view/4d3SR4
float3 Pastelizer(float hue) {
    hue = Frac(hue + 0.92620819117478f) * 6.2831853071796f;
    float2 cocg =  0.25f * SinCos(hue);
    float2 br = float2(-cocg.y, cocg.y) - cocg.x;
    float3 c = 0.729f + float3(br.y, cocg.x, br.x);
    return c * c;
}
//----------------------------------------------------------------------------
float3 Hue_to_RGB(float hue) {
    float R = Abs(hue * 6 - 3) - 1;
    float G = 2 - Abs(hue * 6 - 2);
    float B = 2 - Abs(hue * 6 - 4);
    return Saturate(float3(R,G,B));
}
//----------------------------------------------------------------------------
// http://www.chilliant.com/rgb2hsv.html
float3 RGB_to_HCV(const float3& rgb) {
    const float Epsilon = 1e-10f;
    // Based on work by Sam Hocevar and Emil Persson
    float4 P = (rgb.y < rgb.z) ? float4(rgb.z, rgb.y, -1.0f, 2.0f/3.0f) : float4(rgb.y, rgb.z, 0.0f, -1.0f/3.0f);
    float4 Q = (rgb.x < P.x) ? float4(P.xyw, rgb.x) : float4(rgb.x, P.yzx);
    float C = Q.x - Min(Q.w, Q.y);
    float H = Abs((Q.w - Q.y) / (6 * C + Epsilon) + Q.z);
    return float3(H, C, Q.x);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float3 HSV_to_RGB(const float3& hsv) {
    float3 RGB = Hue_to_RGB(hsv.x);
    float3 x = RGB * 2.f;
    return ((x - 1) * hsv.y + 1) * hsv.z;
}
//----------------------------------------------------------------------------
float3 RGB_to_HSV(const float3& rgb) {
    const float Epsilon = 1e-10f;
    float3 HCV = RGB_to_HCV(rgb);
    float S = HCV.y / (HCV.z + Epsilon);
    return float3(HCV.x, S, HCV.z);
}
//----------------------------------------------------------------------------
// IQ's goodness
// https://www.shadertoy.com/view/MsS3Wc
float3 HSV_to_RGB_smooth(const float3& rgb) {
    return rgb.z * (1.f - rgb.y * Smoothstep(2.f, 1.f, Abs(FMod(rgb.x*6.f + float3(0, 4, 2), 6.f) - 3.f)));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float3 HSL_to_RGB(const float3& hsl) {
    float3 RGB = Hue_to_RGB(hsl.x);
    float C = (1 - Abs(2 * hsl.z - 1)) * hsl.y;
    return (RGB - 0.5f) * C + hsl.z;
}
//----------------------------------------------------------------------------
float3 RGB_to_HSL(const float3& rgb) {
    const float Epsilon = 1e-10f;
    float3 HCV = RGB_to_HCV(rgb);
    float L = HCV.z - HCV.y * 0.5f;
    float S = HCV.y / (1 - Abs(L * 2 - 1) + Epsilon);
    return float3(HCV.x, S, L);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// YCoCg
// ranges: Y=0..1, Co=-0.5..0.5, Cg=-0.5..0.5
// ranges returned by those functions is shifted to [0..1] for every channel
// https://www.shadertoy.com/view/4dcSRN
//----------------------------------------------------------------------------
float3 YCoCg_to_RGB(const float3& yCoCg) {
    float y = yCoCg.x;
    float Co = yCoCg.y + 0.5f;
    float Cg = yCoCg.z + 0.5f;
    float r = Co - Cg;
    float b = -Co - Cg;
    return float3(y + r, y + Cg, y + b);
}
//----------------------------------------------------------------------------
float3 RGB_to_YCoCg(const float3& rgb) {
    float tmp = 0.5f*(rgb.x + rgb.z);
    float y = rgb.y + tmp;
    float Cg = rgb.y - tmp;
    float Co = rgb.x - rgb.z;
    return float3(y * 0.5f, Co*0.5f+0.5f, Cg*0.5f+0.5f);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLinearColor FLinearColor::AliceBlue() { return FLinearColor(0.871f, 0.939f, 1.000f, 1.0f); }
FLinearColor FLinearColor::AntiqueWhite() { return FLinearColor(0.956f, 0.831f, 0.680f, 1.0f); }
FLinearColor FLinearColor::Aqua() { return FLinearColor(0.000f, 1.000f, 1.000f, 1.0f); }
FLinearColor FLinearColor::Aquamarine() { return FLinearColor(0.212f, 1.000f, 0.658f, 1.0f); }
FLinearColor FLinearColor::Azure() { return FLinearColor(0.871f, 1.000f, 1.000f, 1.0f); }
FLinearColor FLinearColor::Beige() { return FLinearColor(0.913f, 0.913f, 0.716f, 1.0f); }
FLinearColor FLinearColor::Bisque() { return FLinearColor(1.000f, 0.776f, 0.552f, 1.0f); }
FLinearColor FLinearColor::Black() { return FLinearColor(0.000f, 0.000f, 0.000f, 1.0f); }
FLinearColor FLinearColor::BlanchedAlmond() { return FLinearColor(1.000f, 0.831f, 0.610f, 1.0f); }
FLinearColor FLinearColor::Blue() { return FLinearColor(0.000f, 0.000f, 1.000f, 1.0f); }
FLinearColor FLinearColor::BlueViolet() { return FLinearColor(0.254f, 0.024f, 0.761f, 1.0f); }
FLinearColor FLinearColor::Brown() { return FLinearColor(0.376f, 0.023f, 0.023f, 1.0f); }
FLinearColor FLinearColor::BurlyWood() { return FLinearColor(0.730f, 0.479f, 0.242f, 1.0f); }
FLinearColor FLinearColor::CadetBlue() { return FLinearColor(0.114f, 0.342f, 0.352f, 1.0f); }
FLinearColor FLinearColor::Chartreuse() { return FLinearColor(0.212f, 1.000f, 0.000f, 1.0f); }
FLinearColor FLinearColor::Chocolate() { return FLinearColor(0.644f, 0.141f, 0.013f, 1.0f); }
FLinearColor FLinearColor::Coral() { return FLinearColor(1.000f, 0.212f, 0.080f, 1.0f); }
FLinearColor FLinearColor::CornflowerBlue() { return FLinearColor(0.127f, 0.301f, 0.847f, 1.0f); }
FLinearColor FLinearColor::Cornsilk() { return FLinearColor(1.000f, 0.939f, 0.716f, 1.0f); }
FLinearColor FLinearColor::Crimson() { return FLinearColor(0.716f, 0.007f, 0.045f, 1.0f); }
FLinearColor FLinearColor::Cyan() { return FLinearColor(0.000f, 1.000f, 1.000f, 1.0f); }
FLinearColor FLinearColor::DarkBlue() { return FLinearColor(0.000f, 0.000f, 0.258f, 1.0f); }
FLinearColor FLinearColor::DarkCyan() { return FLinearColor(0.000f, 0.258f, 0.258f, 1.0f); }
FLinearColor FLinearColor::DarkGoldenRod() { return FLinearColor(0.479f, 0.238f, 0.003f, 1.0f); }
FLinearColor FLinearColor::DarkGray() { return FLinearColor(0.397f, 0.397f, 0.397f, 1.0f); }
FLinearColor FLinearColor::DarkGreen() { return FLinearColor(0.000f, 0.127f, 0.000f, 1.0f); }
FLinearColor FLinearColor::DarkKhaki() { return FLinearColor(0.509f, 0.474f, 0.147f, 1.0f); }
FLinearColor FLinearColor::DarkMagenta() { return FLinearColor(0.258f, 0.000f, 0.258f, 1.0f); }
FLinearColor FLinearColor::DarkOliveGreen() { return FLinearColor(0.091f, 0.147f, 0.028f, 1.0f); }
FLinearColor FLinearColor::DarkOrange() { return FLinearColor(1.000f, 0.262f, 0.000f, 1.0f); }
FLinearColor FLinearColor::DarkOrchid() { return FLinearColor(0.319f, 0.032f, 0.604f, 1.0f); }
FLinearColor FLinearColor::DarkRed() { return FLinearColor(0.258f, 0.000f, 0.000f, 1.0f); }
FLinearColor FLinearColor::DarkSalmon() { return FLinearColor(0.815f, 0.305f, 0.195f, 1.0f); }
FLinearColor FLinearColor::DarkSeaGreen() { return FLinearColor(0.275f, 0.503f, 0.275f, 1.0f); }
FLinearColor FLinearColor::DarkSlateBlue() { return FLinearColor(0.065f, 0.047f, 0.258f, 1.0f); }
FLinearColor FLinearColor::DarkSlateGray() { return FLinearColor(0.028f, 0.078f, 0.078f, 1.0f); }
FLinearColor FLinearColor::DarkTurquoise() { return FLinearColor(0.000f, 0.617f, 0.638f, 1.0f); }
FLinearColor FLinearColor::DarkViolet() { return FLinearColor(0.296f, 0.000f, 0.651f, 1.0f); }
FLinearColor FLinearColor::DeepPink() { return FLinearColor(1.000f, 0.007f, 0.292f, 1.0f); }
FLinearColor FLinearColor::DeepSkyBlue() { return FLinearColor(0.000f, 0.521f, 1.000f, 1.0f); }
FLinearColor FLinearColor::DimGray() { return FLinearColor(0.141f, 0.141f, 0.141f, 1.0f); }
FLinearColor FLinearColor::DodgerBlue() { return FLinearColor(0.013f, 0.279f, 1.000f, 1.0f); }
FLinearColor FLinearColor::FireBrick() { return FLinearColor(0.445f, 0.016f, 0.016f, 1.0f); }
FLinearColor FLinearColor::FloralWhite() { return FLinearColor(1.000f, 0.956f, 0.871f, 1.0f); }
FLinearColor FLinearColor::ForestGreen() { return FLinearColor(0.016f, 0.258f, 0.016f, 1.0f); }
FLinearColor FLinearColor::Fuchsia() { return FLinearColor(1.000f, 0.000f, 1.000f, 1.0f); }
FLinearColor FLinearColor::Gainsboro() { return FLinearColor(0.716f, 0.716f, 0.716f, 1.0f); }
FLinearColor FLinearColor::GhostWhite() { return FLinearColor(0.939f, 0.939f, 1.000f, 1.0f); }
FLinearColor FLinearColor::Gold() { return FLinearColor(1.000f, 0.680f, 0.000f, 1.0f); }
FLinearColor FLinearColor::GoldenRod() { return FLinearColor(0.701f, 0.376f, 0.014f, 1.0f); }
FLinearColor FLinearColor::Gray() { return FLinearColor(0.216f, 0.216f, 0.216f, 1.0f); }
FLinearColor FLinearColor::Green() { return FLinearColor(0.000f, 0.216f, 0.000f, 1.0f); }
FLinearColor FLinearColor::GreenYellow() { return FLinearColor(0.418f, 1.000f, 0.028f, 1.0f); }
FLinearColor FLinearColor::HoneyDew() { return FLinearColor(0.871f, 1.000f, 0.871f, 1.0f); }
FLinearColor FLinearColor::HotPink() { return FLinearColor(1.000f, 0.141f, 0.456f, 1.0f); }
FLinearColor FLinearColor::IndianRed() { return FLinearColor(0.610f, 0.107f, 0.107f, 1.0f); }
FLinearColor FLinearColor::Indigo() { return FLinearColor(0.070f, 0.000f, 0.223f, 1.0f); }
FLinearColor FLinearColor::Ivory() { return FLinearColor(1.000f, 1.000f, 0.871f, 1.0f); }
FLinearColor FLinearColor::Khaki() { return FLinearColor(0.871f, 0.791f, 0.262f, 1.0f); }
FLinearColor FLinearColor::Lavender() { return FLinearColor(0.791f, 0.791f, 0.956f, 1.0f); }
FLinearColor FLinearColor::LavenderBlush() { return FLinearColor(1.000f, 0.871f, 0.913f, 1.0f); }
FLinearColor FLinearColor::LawnGreen() { return FLinearColor(0.202f, 0.973f, 0.000f, 1.0f); }
FLinearColor FLinearColor::LemonChiffon() { return FLinearColor(1.000f, 0.956f, 0.610f, 1.0f); }
FLinearColor FLinearColor::LightBlue() { return FLinearColor(0.418f, 0.687f, 0.791f, 1.0f); }
FLinearColor FLinearColor::LightCoral() { return FLinearColor(0.871f, 0.216f, 0.216f, 1.0f); }
FLinearColor FLinearColor::LightCyan() { return FLinearColor(0.745f, 1.000f, 1.000f, 1.0f); }
FLinearColor FLinearColor::LightGoldenRodYellow() { return FLinearColor(0.956f, 0.956f, 0.644f, 1.0f); }
FLinearColor FLinearColor::LightGray() { return FLinearColor(0.651f, 0.651f, 0.651f, 1.0f); }
FLinearColor FLinearColor::LightGreen() { return FLinearColor(0.279f, 0.855f, 0.279f, 1.0f); }
FLinearColor FLinearColor::LightPink() { return FLinearColor(1.000f, 0.468f, 0.533f, 1.0f); }
FLinearColor FLinearColor::LightSalmon() { return FLinearColor(1.000f, 0.352f, 0.195f, 1.0f); }
FLinearColor FLinearColor::LightSeaGreen() { return FLinearColor(0.014f, 0.445f, 0.402f, 1.0f); }
FLinearColor FLinearColor::LightSkyBlue() { return FLinearColor(0.242f, 0.617f, 0.956f, 1.0f); }
FLinearColor FLinearColor::LightSlateGray() { return FLinearColor(0.184f, 0.246f, 0.319f, 1.0f); }
FLinearColor FLinearColor::LightSteelBlue() { return FLinearColor(0.434f, 0.552f, 0.730f, 1.0f); }
FLinearColor FLinearColor::LightYellow() { return FLinearColor(1.000f, 1.000f, 0.745f, 1.0f); }
FLinearColor FLinearColor::Lime() { return FLinearColor(0.000f, 1.000f, 0.000f, 1.0f); }
FLinearColor FLinearColor::LimeGreen() { return FLinearColor(0.032f, 0.610f, 0.032f, 1.0f); }
FLinearColor FLinearColor::Linen() { return FLinearColor(0.956f, 0.871f, 0.791f, 1.0f); }
FLinearColor FLinearColor::Magenta() { return FLinearColor(1.000f, 0.000f, 1.000f, 1.0f); }
FLinearColor FLinearColor::Maroon() { return FLinearColor(0.216f, 0.000f, 0.000f, 1.0f); }
FLinearColor FLinearColor::MediumAquamarine() { return FLinearColor(0.133f, 0.610f, 0.402f, 1.0f); }
FLinearColor FLinearColor::MediumBlue() { return FLinearColor(0.000f, 0.000f, 0.610f, 1.0f); }
FLinearColor FLinearColor::MediumOrchid() { return FLinearColor(0.491f, 0.091f, 0.651f, 1.0f); }
FLinearColor FLinearColor::MediumPurple() { return FLinearColor(0.292f, 0.162f, 0.687f, 1.0f); }
FLinearColor FLinearColor::MediumSeaGreen() { return FLinearColor(0.045f, 0.451f, 0.165f, 1.0f); }
FLinearColor FLinearColor::MediumSlateBlue() { return FLinearColor(0.198f, 0.138f, 0.855f, 1.0f); }
FLinearColor FLinearColor::MediumSpringGreen() { return FLinearColor(0.000f, 0.956f, 0.323f, 1.0f); }
FLinearColor FLinearColor::MediumTurquoise() { return FLinearColor(0.065f, 0.638f, 0.604f, 1.0f); }
FLinearColor FLinearColor::MediumVioletRed() { return FLinearColor(0.571f, 0.007f, 0.235f, 1.0f); }
FLinearColor FLinearColor::MidnightBlue() { return FLinearColor(0.010f, 0.010f, 0.162f, 1.0f); }
FLinearColor FLinearColor::MintCream() { return FLinearColor(0.913f, 1.000f, 0.956f, 1.0f); }
FLinearColor FLinearColor::MistyRose() { return FLinearColor(1.000f, 0.776f, 0.753f, 1.0f); }
FLinearColor FLinearColor::Moccasin() { return FLinearColor(1.000f, 0.776f, 0.462f, 1.0f); }
FLinearColor FLinearColor::NavajoWhite() { return FLinearColor(1.000f, 0.730f, 0.418f, 1.0f); }
FLinearColor FLinearColor::Navy() { return FLinearColor(0.000f, 0.000f, 0.216f, 1.0f); }
FLinearColor FLinearColor::OldLace() { return FLinearColor(0.982f, 0.913f, 0.791f, 1.0f); }
FLinearColor FLinearColor::Olive() { return FLinearColor(0.216f, 0.216f, 0.000f, 1.0f); }
FLinearColor FLinearColor::OliveDrab() { return FLinearColor(0.147f, 0.270f, 0.017f, 1.0f); }
FLinearColor FLinearColor::Orange() { return FLinearColor(1.000f, 0.376f, 0.000f, 1.0f); }
FLinearColor FLinearColor::OrangeRed() { return FLinearColor(1.000f, 0.060f, 0.000f, 1.0f); }
FLinearColor FLinearColor::Orchid() { return FLinearColor(0.701f, 0.162f, 0.672f, 1.0f); }
FLinearColor FLinearColor::PaleGoldenRod() { return FLinearColor(0.855f, 0.807f, 0.402f, 1.0f); }
FLinearColor FLinearColor::PaleGreen() { return FLinearColor(0.314f, 0.965f, 0.314f, 1.0f); }
FLinearColor FLinearColor::PaleTurquoise() { return FLinearColor(0.429f, 0.855f, 0.855f, 1.0f); }
FLinearColor FLinearColor::PaleVioletRed() { return FLinearColor(0.687f, 0.162f, 0.292f, 1.0f); }
FLinearColor FLinearColor::PapayaWhip() { return FLinearColor(1.000f, 0.863f, 0.665f, 1.0f); }
FLinearColor FLinearColor::PeachPuff() { return FLinearColor(1.000f, 0.701f, 0.485f, 1.0f); }
FLinearColor FLinearColor::Peru() { return FLinearColor(0.610f, 0.235f, 0.050f, 1.0f); }
FLinearColor FLinearColor::Pink() { return FLinearColor(1.000f, 0.527f, 0.597f, 1.0f); }
FLinearColor FLinearColor::Plum() { return FLinearColor(0.723f, 0.352f, 0.723f, 1.0f); }
FLinearColor FLinearColor::PowderBlue() { return FLinearColor(0.434f, 0.745f, 0.791f, 1.0f); }
FLinearColor FLinearColor::Purple() { return FLinearColor(0.216f, 0.000f, 0.216f, 1.0f); }
FLinearColor FLinearColor::Red() { return FLinearColor(1.000f, 0.000f, 0.000f, 1.0f); }
FLinearColor FLinearColor::RosyBrown() { return FLinearColor(0.503f, 0.275f, 0.275f, 1.0f); }
FLinearColor FLinearColor::RoyalBlue() { return FLinearColor(0.053f, 0.141f, 0.753f, 1.0f); }
FLinearColor FLinearColor::SaddleBrown() { return FLinearColor(0.258f, 0.060f, 0.007f, 1.0f); }
FLinearColor FLinearColor::Salmon() { return FLinearColor(0.956f, 0.216f, 0.168f, 1.0f); }
FLinearColor FLinearColor::SandyBrown() { return FLinearColor(0.905f, 0.371f, 0.117f, 1.0f); }
FLinearColor FLinearColor::SeaGreen() { return FLinearColor(0.027f, 0.258f, 0.095f, 1.0f); }
FLinearColor FLinearColor::SeaShell() { return FLinearColor(1.000f, 0.913f, 0.855f, 1.0f); }
FLinearColor FLinearColor::Sienna() { return FLinearColor(0.352f, 0.084f, 0.026f, 1.0f); }
FLinearColor FLinearColor::Silver() { return FLinearColor(0.527f, 0.527f, 0.527f, 1.0f); }
FLinearColor FLinearColor::SkyBlue() { return FLinearColor(0.242f, 0.617f, 0.831f, 1.0f); }
FLinearColor FLinearColor::SlateBlue() { return FLinearColor(0.144f, 0.102f, 0.610f, 1.0f); }
FLinearColor FLinearColor::SlateGray() { return FLinearColor(0.162f, 0.216f, 0.279f, 1.0f); }
FLinearColor FLinearColor::Snow() { return FLinearColor(1.000f, 0.956f, 0.956f, 1.0f); }
FLinearColor FLinearColor::SpringGreen() { return FLinearColor(0.000f, 1.000f, 0.212f, 1.0f); }
FLinearColor FLinearColor::SteelBlue() { return FLinearColor(0.061f, 0.223f, 0.456f, 1.0f); }
FLinearColor FLinearColor::Tan() { return FLinearColor(0.644f, 0.456f, 0.262f, 1.0f); }
FLinearColor FLinearColor::Teal() { return FLinearColor(0.000f, 0.216f, 0.216f, 1.0f); }
FLinearColor FLinearColor::Thistle() { return FLinearColor(0.687f, 0.521f, 0.687f, 1.0f); }
FLinearColor FLinearColor::Tomato() { return FLinearColor(1.000f, 0.125f, 0.063f, 1.0f); }
FLinearColor FLinearColor::Transparent() { return FLinearColor(0.000f, 0.000f, 0.000f, 0.0f); }
FLinearColor FLinearColor::Turquoise() { return FLinearColor(0.051f, 0.745f, 0.631f, 1.0f); }
FLinearColor FLinearColor::Violet() { return FLinearColor(0.855f, 0.223f, 0.855f, 1.0f); }
FLinearColor FLinearColor::Wheat() { return FLinearColor(0.913f, 0.730f, 0.451f, 1.0f); }
FLinearColor FLinearColor::White() { return FLinearColor(1.000f, 1.000f, 1.000f, 1.0f); }
FLinearColor FLinearColor::WhiteSmoke() { return FLinearColor(0.913f, 0.913f, 0.913f, 1.0f); }
FLinearColor FLinearColor::Yellow() { return FLinearColor(1.000f, 1.000f, 0.000f, 1.0f); }
FLinearColor FLinearColor::YellowGreen() { return FLinearColor(0.323f, 0.610f, 0.032f, 1.0f); }
//----------------------------------------------------------------------------
FColor FColor::AliceBlue() { return FColor(0xF0F8FFFFu); }
FColor FColor::AntiqueWhite() { return FColor(0xFAEBD7FFu); }
FColor FColor::Aqua() { return FColor(0x00FFFFFFu); }
FColor FColor::Aquamarine() { return FColor(0x7FFFD4FFu); }
FColor FColor::Azure() { return FColor(0xF0FFFFFFu); }
FColor FColor::Beige() { return FColor(0xF5F5DCFFu); }
FColor FColor::Bisque() { return FColor(0xFFE4C4FFu); }
FColor FColor::Black() { return FColor(0x000000FFu); }
FColor FColor::BlanchedAlmond() { return FColor(0xFFEBCDFFu); }
FColor FColor::Blue() { return FColor(0x0000FFFFu); }
FColor FColor::BlueViolet() { return FColor(0x8A2BE2FFu); }
FColor FColor::Brown() { return FColor(0xA52A2AFFu); }
FColor FColor::BurlyWood() { return FColor(0xDEB887FFu); }
FColor FColor::CadetBlue() { return FColor(0x5F9EA0FFu); }
FColor FColor::Chartreuse() { return FColor(0x7FFF00FFu); }
FColor FColor::Chocolate() { return FColor(0xD2691EFFu); }
FColor FColor::Coral() { return FColor(0xFF7F50FFu); }
FColor FColor::CornflowerBlue() { return FColor(0x6495EDFFu); }
FColor FColor::Cornsilk() { return FColor(0xFFF8DCFFu); }
FColor FColor::Crimson() { return FColor(0xDC143CFFu); }
FColor FColor::Cyan() { return FColor(0x00FFFFFFu); }
FColor FColor::DarkBlue() { return FColor(0x00008BFFu); }
FColor FColor::DarkCyan() { return FColor(0x008B8BFFu); }
FColor FColor::DarkGoldenRod() { return FColor(0xB8860BFFu); }
FColor FColor::DarkGray() { return FColor(0xA9A9A9FFu); }
FColor FColor::DarkGreen() { return FColor(0x006400FFu); }
FColor FColor::DarkKhaki() { return FColor(0xBDB76BFFu); }
FColor FColor::DarkMagenta() { return FColor(0x8B008BFFu); }
FColor FColor::DarkOliveGreen() { return FColor(0x556B2FFFu); }
FColor FColor::DarkOrange() { return FColor(0xFF8C00FFu); }
FColor FColor::DarkOrchid() { return FColor(0x9932CCFFu); }
FColor FColor::DarkRed() { return FColor(0x8B0000FFu); }
FColor FColor::DarkSalmon() { return FColor(0xE9967AFFu); }
FColor FColor::DarkSeaGreen() { return FColor(0x8FBC8FFFu); }
FColor FColor::DarkSlateBlue() { return FColor(0x483D8BFFu); }
FColor FColor::DarkSlateGray() { return FColor(0x2F4F4FFFu); }
FColor FColor::DarkTurquoise() { return FColor(0x00CED1FFu); }
FColor FColor::DarkViolet() { return FColor(0x9400D3FFu); }
FColor FColor::DeepPink() { return FColor(0xFF1493FFu); }
FColor FColor::DeepSkyBlue() { return FColor(0x00BFFFFFu); }
FColor FColor::DimGray() { return FColor(0x696969FFu); }
FColor FColor::DodgerBlue() { return FColor(0x1E90FFFFu); }
FColor FColor::FireBrick() { return FColor(0xB22222FFu); }
FColor FColor::FloralWhite() { return FColor(0xFFFAF0FFu); }
FColor FColor::ForestGreen() { return FColor(0x228B22FFu); }
FColor FColor::Fuchsia() { return FColor(0xFF00FFFFu); }
FColor FColor::Gainsboro() { return FColor(0xDCDCDCFFu); }
FColor FColor::GhostWhite() { return FColor(0xF8F8FFFFu); }
FColor FColor::Gold() { return FColor(0xFFD700FFu); }
FColor FColor::GoldenRod() { return FColor(0xDAA520FFu); }
FColor FColor::Gray() { return FColor(0x808080FFu); }
FColor FColor::Green() { return FColor(0x008000FFu); }
FColor FColor::GreenYellow() { return FColor(0xADFF2FFFu); }
FColor FColor::HoneyDew() { return FColor(0xF0FFF0FFu); }
FColor FColor::HotPink() { return FColor(0xFF69B4FFu); }
FColor FColor::IndianRed() { return FColor(0xCD5C5CFFu); }
FColor FColor::Indigo() { return FColor(0x4B0082FFu); }
FColor FColor::Ivory() { return FColor(0xFFFFF0FFu); }
FColor FColor::Khaki() { return FColor(0xF0E68CFFu); }
FColor FColor::Lavender() { return FColor(0xE6E6FAFFu); }
FColor FColor::LavenderBlush() { return FColor(0xFFF0F5FFu); }
FColor FColor::LawnGreen() { return FColor(0x7CFC00FFu); }
FColor FColor::LemonChiffon() { return FColor(0xFFFACDFFu); }
FColor FColor::LightBlue() { return FColor(0xADD8E6FFu); }
FColor FColor::LightCoral() { return FColor(0xF08080FFu); }
FColor FColor::LightCyan() { return FColor(0xE0FFFFFFu); }
FColor FColor::LightGoldenRodYellow() { return FColor(0xFAFAD2FFu); }
FColor FColor::LightGray() { return FColor(0xD3D3D3FFu); }
FColor FColor::LightGreen() { return FColor(0x90EE90FFu); }
FColor FColor::LightPink() { return FColor(0xFFB6C1FFu); }
FColor FColor::LightSalmon() { return FColor(0xFFA07AFFu); }
FColor FColor::LightSeaGreen() { return FColor(0x20B2AAFFu); }
FColor FColor::LightSkyBlue() { return FColor(0x87CEFAFFu); }
FColor FColor::LightSlateGray() { return FColor(0x778899FFu); }
FColor FColor::LightSteelBlue() { return FColor(0xB0C4DEFFu); }
FColor FColor::LightYellow() { return FColor(0xFFFFE0FFu); }
FColor FColor::Lime() { return FColor(0x00FF00FFu); }
FColor FColor::LimeGreen() { return FColor(0x32CD32FFu); }
FColor FColor::Linen() { return FColor(0xFAF0E6FFu); }
FColor FColor::Magenta() { return FColor(0xFF00FFFFu); }
FColor FColor::Maroon() { return FColor(0x800000FFu); }
FColor FColor::MediumAquamarine() { return FColor(0x66CDAAFFu); }
FColor FColor::MediumBlue() { return FColor(0x0000CDFFu); }
FColor FColor::MediumOrchid() { return FColor(0xBA55D3FFu); }
FColor FColor::MediumPurple() { return FColor(0x9370D8FFu); }
FColor FColor::MediumSeaGreen() { return FColor(0x3CB371FFu); }
FColor FColor::MediumSlateBlue() { return FColor(0x7B68EEFFu); }
FColor FColor::MediumSpringGreen() { return FColor(0x00FA9AFFu); }
FColor FColor::MediumTurquoise() { return FColor(0x48D1CCFFu); }
FColor FColor::MediumVioletRed() { return FColor(0xC71585FFu); }
FColor FColor::MidnightBlue() { return FColor(0x191970FFu); }
FColor FColor::MintCream() { return FColor(0xF5FFFAFFu); }
FColor FColor::MistyRose() { return FColor(0xFFE4E1FFu); }
FColor FColor::Moccasin() { return FColor(0xFFE4B5FFu); }
FColor FColor::NavajoWhite() { return FColor(0xFFDEADFFu); }
FColor FColor::Navy() { return FColor(0x000080FFu); }
FColor FColor::OldLace() { return FColor(0xFDF5E6FFu); }
FColor FColor::Olive() { return FColor(0x808000FFu); }
FColor FColor::OliveDrab() { return FColor(0x6B8E23FFu); }
FColor FColor::Orange() { return FColor(0xFFA500FFu); }
FColor FColor::OrangeRed() { return FColor(0xFF4500FFu); }
FColor FColor::Orchid() { return FColor(0xDA70D6FFu); }
FColor FColor::PaleGoldenRod() { return FColor(0xEEE8AAFFu); }
FColor FColor::PaleGreen() { return FColor(0x98FB98FFu); }
FColor FColor::PaleTurquoise() { return FColor(0xAFEEEEFFu); }
FColor FColor::PaleVioletRed() { return FColor(0xD87093FFu); }
FColor FColor::PapayaWhip() { return FColor(0xFFEFD5FFu); }
FColor FColor::PeachPuff() { return FColor(0xFFDAB9FFu); }
FColor FColor::Peru() { return FColor(0xCD853FFFu); }
FColor FColor::Pink() { return FColor(0xFFC0CBFFu); }
FColor FColor::Plum() { return FColor(0xDDA0DDFFu); }
FColor FColor::PowderBlue() { return FColor(0xB0E0E6FFu); }
FColor FColor::Purple() { return FColor(0x800080FFu); }
FColor FColor::Red() { return FColor(0xFF0000FFu); }
FColor FColor::RosyBrown() { return FColor(0xBC8F8FFFu); }
FColor FColor::RoyalBlue() { return FColor(0x4169E1FFu); }
FColor FColor::SaddleBrown() { return FColor(0x8B4513FFu); }
FColor FColor::Salmon() { return FColor(0xFA8072FFu); }
FColor FColor::SandyBrown() { return FColor(0xF4A460FFu); }
FColor FColor::SeaGreen() { return FColor(0x2E8B57FFu); }
FColor FColor::SeaShell() { return FColor(0xFFF5EEFFu); }
FColor FColor::Sienna() { return FColor(0xA0522DFFu); }
FColor FColor::Silver() { return FColor(0xC0C0C0FFu); }
FColor FColor::SkyBlue() { return FColor(0x87CEEBFFu); }
FColor FColor::SlateBlue() { return FColor(0x6A5ACDFFu); }
FColor FColor::SlateGray() { return FColor(0x708090FFu); }
FColor FColor::Snow() { return FColor(0xFFFAFAFFu); }
FColor FColor::SpringGreen() { return FColor(0x00FF7FFFu); }
FColor FColor::SteelBlue() { return FColor(0x4682B4FFu); }
FColor FColor::Tan() { return FColor(0xD2B48CFFu); }
FColor FColor::Teal() { return FColor(0x008080FFu); }
FColor FColor::Thistle() { return FColor(0xD8BFD8FFu); }
FColor FColor::Tomato() { return FColor(0xFF6347FFu); }
FColor FColor::Transparent() { return FColor(0x00000000u); }
FColor FColor::Turquoise() { return FColor(0x40E0D0FFu); }
FColor FColor::Violet() { return FColor(0xEE82EEFFu); }
FColor FColor::Wheat() { return FColor(0xF5DEB3FFu); }
FColor FColor::White() { return FColor(0xFFFFFFFFu); }
FColor FColor::WhiteSmoke() { return FColor(0xF5F5F5FFu); }
FColor FColor::Yellow() { return FColor(0xFFFF00FFu); }
FColor FColor::YellowGreen() { return FColor(0x9ACD32FFu); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(FColor) == sizeof(u32));
STATIC_ASSERT(sizeof(FLinearColor) == sizeof(u128));
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
