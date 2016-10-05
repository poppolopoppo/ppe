#pragma once

#include "Core/Core.h"

#include "Core/Color/Color_fwd.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"
#include "Core/Maths/PackingHelpers.h"

namespace Core {
template <typename T>
class TMemoryView;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FColorShuffleRGBA { static constexpr size_t R = 0, G = 1, B = 2, A = 3; };
//----------------------------------------------------------------------------
struct FColorShuffleBGRA { static constexpr size_t B = 0, G = 1, R = 2, A = 3; };
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
class TBasicColor {
public:
    TBasicColor();
    ~TBasicColor();

    TBasicColor(T broadcast);
    TBasicColor(T x, T y, T z, T w);
    TBasicColor(const TBasicColorData<T>& data);
    TBasicColor(const TScalarVector<float, 3>& xyz, float alpha);

    TBasicColor(const TBasicColor& other);
    TBasicColor& operator =(const TBasicColor& other);

    template <typename U, typename _Shuffle2>
    TBasicColor(const TBasicColor<U, _Shuffle2>& other);
    template <typename U, typename _Shuffle2>
    TBasicColor& operator =(const TBasicColor<U, _Shuffle2>& other);

    operator TBasicColorData<T>& () { return _data; }
    operator const TBasicColorData<T>& () const { return _data; }

    TBasicColorData<T>& Data() { return _data; }
    const TBasicColorData<T>& Data() const { return _data; }

    T *Pointer() { return _data._data; }
    const T *Pointer() const { return _data._data; }

    T& r() { return _data._data[_Shuffle::R]; }
    T& g() { return _data._data[_Shuffle::G]; }
    T& b() { return _data._data[_Shuffle::B]; }
    T& a() { return _data._data[_Shuffle::A]; }

    T r() const { return _data._data[_Shuffle::R]; }
    T g() const { return _data._data[_Shuffle::G]; }
    T b() const { return _data._data[_Shuffle::B]; }
    T a() const { return _data._data[_Shuffle::A]; }

    T& operator [](size_t i) { return _data[i]; }
    T operator [](size_t i) const { return _data[i]; }

    TBasicColor AlphaBlend(const TBasicColor& other) const;
    TBasicColor Fade(T alpha) const;

    void FromRGB(const TScalarVector<T, 3>& rgb) { r() = rgb.x(); g() = rgb.y(); b() = rgb.z(); }
    TScalarVector<T, 3> ToRGB() const {  return TScalarVector<T, 3>(r(), g(), b()); }

    TBasicColor<T, FColorShuffleRGBA> ToRGBA() const;
    TBasicColor<T, FColorShuffleBGRA> ToBGRA() const;

    TBasicColor ToSRGB() const;
    TBasicColor ToLinear() const;

    bool operator ==(const TBasicColor& other) const { return _data == other._data; }
    bool operator !=(const TBasicColor& other) const { return !operator ==(other); }

private:
    TBasicColorData<T> _data;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern template class TBasicColor< TUNorm<u8>, FColorShuffleBGRA >;
extern template class TBasicColor< TUNorm<u8>, FColorShuffleRGBA >;
//----------------------------------------------------------------------------
extern template class TBasicColor< TUNorm<u16>, FColorShuffleBGRA >;
extern template class TBasicColor< TUNorm<u16>, FColorShuffleRGBA >;
//----------------------------------------------------------------------------
extern template class TBasicColor< float, FColorShuffleBGRA >;
extern template class TBasicColor< float, FColorShuffleRGBA >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE float SRGB_to_Linear(float srgb);
float SRGB_to_Linear(u8 srgb); // optimized version with a precomputed table
//----------------------------------------------------------------------------
FORCE_INLINE float Linear_to_SRGB(float lin);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TBasicColorData<T> SRGB_to_Linear(const TBasicColorData<T>& srgb);
//----------------------------------------------------------------------------
template <typename T>
TBasicColorData<T> Linear_to_SRGB(const TBasicColorData<T>& linear);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TBasicColorData<T> HSV_to_RGB(const TBasicColorData<T>& hsv);
//----------------------------------------------------------------------------
template <typename T>
TBasicColorData<T> RGB_to_HSV(const TBasicColorData<T>& rgb);
//----------------------------------------------------------------------------
template <typename T>
TBasicColorData<T> HSL_to_RGB(const TBasicColorData<T>& hsl);
//----------------------------------------------------------------------------
template <typename T>
TBasicColorData<T> RGB_to_HSL(const TBasicColorData<T>& rgb);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Color {
//----------------------------------------------------------------------------
ColorRGBA AliceBlue();
ColorRGBA AntiqueWhite();
ColorRGBA Aqua();
ColorRGBA Aquamarine();
ColorRGBA Azure();
ColorRGBA Beige();
ColorRGBA Bisque();
ColorRGBA Black();
ColorRGBA BlanchedAlmond();
ColorRGBA Blue();
ColorRGBA BlueViolet();
ColorRGBA Brown();
ColorRGBA BurlyWood();
ColorRGBA CadetBlue();
ColorRGBA Chartreuse();
ColorRGBA Chocolate();
ColorRGBA Coral();
ColorRGBA CornflowerBlue();
ColorRGBA Cornsilk();
ColorRGBA Crimson();
ColorRGBA Cyan();
ColorRGBA DarkBlue();
ColorRGBA DarkCyan();
ColorRGBA DarkGoldenRod();
ColorRGBA DarkGray();
ColorRGBA DarkGreen();
ColorRGBA DarkKhaki();
ColorRGBA DarkMagenta();
ColorRGBA DarkOliveGreen();
ColorRGBA DarkOrange();
ColorRGBA DarkOrchid();
ColorRGBA DarkRed();
ColorRGBA DarkSalmon();
ColorRGBA DarkSeaGreen();
ColorRGBA DarkSlateBlue();
ColorRGBA DarkSlateGray();
ColorRGBA DarkTurquoise();
ColorRGBA DarkViolet();
ColorRGBA DeepPink();
ColorRGBA DeepSkyBlue();
ColorRGBA DimGray();
ColorRGBA DodgerBlue();
ColorRGBA FireBrick();
ColorRGBA FloralWhite();
ColorRGBA ForestGreen();
ColorRGBA Fuchsia();
ColorRGBA Gainsboro();
ColorRGBA GhostWhite();
ColorRGBA Gold();
ColorRGBA GoldenRod();
ColorRGBA Gray();
ColorRGBA Green();
ColorRGBA GreenYellow();
ColorRGBA HoneyDew();
ColorRGBA HotPink();
ColorRGBA IndianRed();
ColorRGBA Indigo();
ColorRGBA Ivory();
ColorRGBA Khaki();
ColorRGBA Lavender();
ColorRGBA LavenderBlush();
ColorRGBA LawnGreen();
ColorRGBA LemonChiffon();
ColorRGBA LightBlue();
ColorRGBA LightCoral();
ColorRGBA LightCyan();
ColorRGBA LightGoldenRodYellow();
ColorRGBA LightGray();
ColorRGBA LightGreen();
ColorRGBA LightPink();
ColorRGBA LightSalmon();
ColorRGBA LightSeaGreen();
ColorRGBA LightSkyBlue();
ColorRGBA LightSlateGray();
ColorRGBA LightSteelBlue();
ColorRGBA LightYellow();
ColorRGBA Lime();
ColorRGBA LimeGreen();
ColorRGBA Linen();
ColorRGBA Magenta();
ColorRGBA Maroon();
ColorRGBA MediumAquaMarine();
ColorRGBA MediumBlue();
ColorRGBA MediumOrchid();
ColorRGBA MediumPurple();
ColorRGBA MediumSeaGreen();
ColorRGBA MediumSlateBlue();
ColorRGBA MediumSpringGreen();
ColorRGBA MediumTurquoise();
ColorRGBA MediumVioletRed();
ColorRGBA MidnightBlue();
ColorRGBA MintCream();
ColorRGBA MistyRose();
ColorRGBA Moccasin();
ColorRGBA NavajoWhite();
ColorRGBA Navy();
ColorRGBA OldLace();
ColorRGBA Olive();
ColorRGBA OliveDrab();
ColorRGBA Orange();
ColorRGBA OrangeRed();
ColorRGBA Orchid();
ColorRGBA PaleGoldenRod();
ColorRGBA PaleGreen();
ColorRGBA PaleTurquoise();
ColorRGBA PaleVioletRed();
ColorRGBA PapayaWhip();
ColorRGBA PeachPuff();
ColorRGBA Peru();
ColorRGBA Pink();
ColorRGBA Plum();
ColorRGBA PowderBlue();
ColorRGBA Purple();
ColorRGBA Red();
ColorRGBA RosyBrown();
ColorRGBA RoyalBlue();
ColorRGBA SaddleBrown();
ColorRGBA Salmon();
ColorRGBA SandyBrown();
ColorRGBA SeaGreen();
ColorRGBA SeaShell();
ColorRGBA Sienna();
ColorRGBA Silver();
ColorRGBA SkyBlue();
ColorRGBA SlateBlue();
ColorRGBA SlateGray();
ColorRGBA Snow();
ColorRGBA SpringGreen();
ColorRGBA SteelBlue();
ColorRGBA Tan();
ColorRGBA Teal();
ColorRGBA Thistle();
ColorRGBA Tomato();
ColorRGBA Transparent();
ColorRGBA Turquoise();
ColorRGBA Violet();
ColorRGBA Wheat();
ColorRGBA White();
ColorRGBA WhiteSmoke();
ColorRGBA Yellow();
ColorRGBA YellowGreen();
//----------------------------------------------------------------------------
} //!namespace color
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Color/Color-inl.h"
