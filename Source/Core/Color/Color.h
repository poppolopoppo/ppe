#pragma once

#include "Core/Core.h"

#include "Core/Color/Color_fwd.h"

#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Geometry/ScalarVectorHelpers.h"
#include "Core/Maths/Packing/PackingHelpers.h"

namespace Core {
template <typename T>
class MemoryView;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct ColorShuffleRGBA { static constexpr size_t R = 0, G = 1, B = 2, A = 3; };
//----------------------------------------------------------------------------
struct ColorShuffleBGRA { static constexpr size_t B = 0, G = 1, R = 2, A = 3; };
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
class BasicColor {
public:
    BasicColor();
    ~BasicColor();

    BasicColor(T broadcast);
    BasicColor(T x, T y, T z, T w);
    BasicColor(const BasicColorData<T>& data);
    BasicColor(const ScalarVector<float, 3>& xyz, float alpha);

    BasicColor(const BasicColor& other);
    BasicColor& operator =(const BasicColor& other);

    template <typename U, typename _Shuffle2>
    BasicColor(const BasicColor<U, _Shuffle2>& other);
    template <typename U, typename _Shuffle2>
    BasicColor& operator =(const BasicColor<U, _Shuffle2>& other);

    operator BasicColorData<T>& () { return _data; }
    operator const BasicColorData<T>& () const { return _data; }

    BasicColorData<T>& Data() { return _data; }
    const BasicColorData<T>& Data() const { return _data; }

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

    ScalarVector<T, 3> rgb() const {  return ScalarVector<T, 3>(r(), g(), b()); }

    T& operator [](size_t i) { return _data[i]; }
    T operator [](size_t i) const { return _data[i]; }

    BasicColor<T, ColorShuffleRGBA> ToRGBA() const;
    BasicColor<T, ColorShuffleBGRA> ToBGRA() const;

    BasicColor ToSRGB() const;
    BasicColor ToLinear() const;

    bool operator ==(const BasicColor& other) const { return _data == other._data; }
    bool operator !=(const BasicColor& other) const { return !operator ==(other); }

private:
    BasicColorData<T> _data;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern template class BasicColor< UNorm<u8>, ColorShuffleBGRA >;
extern template class BasicColor< UNorm<u8>, ColorShuffleRGBA >;
//----------------------------------------------------------------------------
extern template class BasicColor< UNorm<u16>, ColorShuffleBGRA >;
extern template class BasicColor< UNorm<u16>, ColorShuffleRGBA >;
//----------------------------------------------------------------------------
extern template class BasicColor< float, ColorShuffleBGRA >;
extern template class BasicColor< float, ColorShuffleRGBA >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE float SRGB_to_Linear(float srgb);
//----------------------------------------------------------------------------
FORCE_INLINE float Linear_to_SRGB(float lin);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
BasicColorData<T> SRGB_to_Linear(const BasicColorData<T>& srgb);
//----------------------------------------------------------------------------
template <typename T>
BasicColorData<T> Linear_to_SRGB(const BasicColorData<T>& linear);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float3 Hue_to_RGB(float hue);
//----------------------------------------------------------------------------
float3 RGB_to_HCV(const float3& rgb);
//----------------------------------------------------------------------------
float3 HSV_to_RGB(const float3& hsv);
//----------------------------------------------------------------------------
float3 RGB_to_HSV(const float3& rgb);
//----------------------------------------------------------------------------
float3 HSL_to_RGB(const float3& hsl);
//----------------------------------------------------------------------------
float3 RGB_to_HSL(const float3& rgb);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
BasicColorData<T> HSV_to_RGB(const BasicColorData<T>& hsv);
//----------------------------------------------------------------------------
template <typename T>
BasicColorData<T> RGB_to_HSV(const BasicColorData<T>& rgb);
//----------------------------------------------------------------------------
template <typename T>
BasicColorData<T> HSL_to_RGB(const BasicColorData<T>& hsl);
//----------------------------------------------------------------------------
template <typename T>
BasicColorData<T> RGB_to_HSL(const BasicColorData<T>& rgb);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct Color {
//----------------------------------------------------------------------------
static MemoryView<const ColorRGBA *const> AllColors();
//----------------------------------------------------------------------------
static const ColorRGBA AliceBlue;
static const ColorRGBA AntiqueWhite;
static const ColorRGBA Aqua;
static const ColorRGBA Aquamarine;
static const ColorRGBA Azure;
static const ColorRGBA Beige;
static const ColorRGBA Bisque;
static const ColorRGBA Black;
static const ColorRGBA BlanchedAlmond;
static const ColorRGBA Blue;
static const ColorRGBA BlueViolet;
static const ColorRGBA Brown;
static const ColorRGBA BurlyWood;
static const ColorRGBA CadetBlue;
static const ColorRGBA Chartreuse;
static const ColorRGBA Chocolate;
static const ColorRGBA Coral;
static const ColorRGBA CornflowerBlue;
static const ColorRGBA Cornsilk;
static const ColorRGBA Crimson;
static const ColorRGBA Cyan;
static const ColorRGBA DarkBlue;
static const ColorRGBA DarkCyan;
static const ColorRGBA DarkGoldenRod;
static const ColorRGBA DarkGray;
static const ColorRGBA DarkGreen;
static const ColorRGBA DarkKhaki;
static const ColorRGBA DarkMagenta;
static const ColorRGBA DarkOliveGreen;
static const ColorRGBA DarkOrange;
static const ColorRGBA DarkOrchid;
static const ColorRGBA DarkRed;
static const ColorRGBA DarkSalmon;
static const ColorRGBA DarkSeaGreen;
static const ColorRGBA DarkSlateBlue;
static const ColorRGBA DarkSlateGray;
static const ColorRGBA DarkTurquoise;
static const ColorRGBA DarkViolet;
static const ColorRGBA DeepPink;
static const ColorRGBA DeepSkyBlue;
static const ColorRGBA DimGray;
static const ColorRGBA DodgerBlue;
static const ColorRGBA FireBrick;
static const ColorRGBA FloralWhite;
static const ColorRGBA ForestGreen;
static const ColorRGBA Fuchsia;
static const ColorRGBA Gainsboro;
static const ColorRGBA GhostWhite;
static const ColorRGBA Gold;
static const ColorRGBA GoldenRod;
static const ColorRGBA Gray;
static const ColorRGBA Green;
static const ColorRGBA GreenYellow;
static const ColorRGBA HoneyDew;
static const ColorRGBA HotPink;
static const ColorRGBA IndianRed;
static const ColorRGBA Indigo;
static const ColorRGBA Ivory;
static const ColorRGBA Khaki;
static const ColorRGBA Lavender;
static const ColorRGBA LavenderBlush;
static const ColorRGBA LawnGreen;
static const ColorRGBA LemonChiffon;
static const ColorRGBA LightBlue;
static const ColorRGBA LightCoral;
static const ColorRGBA LightCyan;
static const ColorRGBA LightGoldenRodYellow;
static const ColorRGBA LightGray;
static const ColorRGBA LightGreen;
static const ColorRGBA LightPink;
static const ColorRGBA LightSalmon;
static const ColorRGBA LightSeaGreen;
static const ColorRGBA LightSkyBlue;
static const ColorRGBA LightSlateGray;
static const ColorRGBA LightSteelBlue;
static const ColorRGBA LightYellow;
static const ColorRGBA Lime;
static const ColorRGBA LimeGreen;
static const ColorRGBA Linen;
static const ColorRGBA Magenta;
static const ColorRGBA Maroon;
static const ColorRGBA MediumAquaMarine;
static const ColorRGBA MediumBlue;
static const ColorRGBA MediumOrchid;
static const ColorRGBA MediumPurple;
static const ColorRGBA MediumSeaGreen;
static const ColorRGBA MediumSlateBlue;
static const ColorRGBA MediumSpringGreen;
static const ColorRGBA MediumTurquoise;
static const ColorRGBA MediumVioletRed;
static const ColorRGBA MidnightBlue;
static const ColorRGBA MintCream;
static const ColorRGBA MistyRose;
static const ColorRGBA Moccasin;
static const ColorRGBA NavajoWhite;
static const ColorRGBA Navy;
static const ColorRGBA OldLace;
static const ColorRGBA Olive;
static const ColorRGBA OliveDrab;
static const ColorRGBA Orange;
static const ColorRGBA OrangeRed;
static const ColorRGBA Orchid;
static const ColorRGBA PaleGoldenRod;
static const ColorRGBA PaleGreen;
static const ColorRGBA PaleTurquoise;
static const ColorRGBA PaleVioletRed;
static const ColorRGBA PapayaWhip;
static const ColorRGBA PeachPuff;
static const ColorRGBA Peru;
static const ColorRGBA Pink;
static const ColorRGBA Plum;
static const ColorRGBA PowderBlue;
static const ColorRGBA Purple;
static const ColorRGBA Red;
static const ColorRGBA RosyBrown;
static const ColorRGBA RoyalBlue;
static const ColorRGBA SaddleBrown;
static const ColorRGBA Salmon;
static const ColorRGBA SandyBrown;
static const ColorRGBA SeaGreen;
static const ColorRGBA SeaShell;
static const ColorRGBA Sienna;
static const ColorRGBA Silver;
static const ColorRGBA SkyBlue;
static const ColorRGBA SlateBlue;
static const ColorRGBA SlateGray;
static const ColorRGBA Snow;
static const ColorRGBA SpringGreen;
static const ColorRGBA SteelBlue;
static const ColorRGBA Tan;
static const ColorRGBA Teal;
static const ColorRGBA Thistle;
static const ColorRGBA Tomato;
static const ColorRGBA Transparent;
static const ColorRGBA Turquoise;
static const ColorRGBA Violet;
static const ColorRGBA Wheat;
static const ColorRGBA White;
static const ColorRGBA WhiteSmoke;
static const ColorRGBA Yellow;
static const ColorRGBA YellowGreen;
//----------------------------------------------------------------------------
}; //!struct color
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Color/Color-inl.h"
