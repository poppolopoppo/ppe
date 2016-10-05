#pragma once

#include "Core/Color/Color.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
TBasicColor<T, _Shuffle>::TBasicColor() {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
TBasicColor<T, _Shuffle>::~TBasicColor() {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
TBasicColor<T, _Shuffle>::TBasicColor(T broadcast)
:   _data(broadcast) {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
TBasicColor<T, _Shuffle>::TBasicColor(T x, T y, T z, T w)
:   _data(x, y, z, w) {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
TBasicColor<T, _Shuffle>::TBasicColor(const TBasicColorData<T>& data)
:   _data(data) {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
TBasicColor<T, _Shuffle>::TBasicColor(const TScalarVector<float, 3>& xyz, float alpha)
:   _data(xyz.x(), xyz.y(), xyz.z(), alpha) {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
TBasicColor<T, _Shuffle>::TBasicColor(const TBasicColor& other)
:   _data(other._data) {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
auto TBasicColor<T, _Shuffle>::operator =(const TBasicColor& other) -> TBasicColor& {
    _data = other._data;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
template <typename U, typename _Shuffle2>
TBasicColor<T, _Shuffle>::TBasicColor(const TBasicColor<U, _Shuffle2>& other) {
    r() = other.r();
    g() = other.g();
    b() = other.b();
    a() = other.a();
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
template <typename U, typename _Shuffle2>
auto TBasicColor<T, _Shuffle>::operator =(const TBasicColor<U, _Shuffle2>& other) -> TBasicColor& {
    r() = other.r();
    g() = other.g();
    b() = other.b();
    a() = other.a();
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
auto TBasicColor<T, _Shuffle>::AlphaBlend(const TBasicColor& other) const -> TBasicColor {
    const T zero = TNumericLimits<T>::Zero();

    if (other.a() > zero && a() == zero) {
        return other;
    }
    else if (other.a() == zero) {
        return *this;
    }
    else {
        const float oa = float(other.a());
        const float ac = (1.0f - float(other.a())) * float(a());

        TBasicColor result;
        result.r() = float(other.r()) * oa + float(r()) * ac;
        result.g() = float(other.g()) * oa + float(g()) * ac;
        result.b() = float(other.b()) * oa + float(b()) * ac;
        result.a() = (oa + ac);

        return result;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
auto TBasicColor<T, _Shuffle>::Fade(T alpha) const -> TBasicColor {
     TBasicColor c = *this;
     c.a() = alpha;
     return c;
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
TBasicColor<T, FColorShuffleRGBA> TBasicColor<T, _Shuffle>::ToRGBA() const {
    return TBasicColor<T, FColorShuffleRGBA>(*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
TBasicColor<T, FColorShuffleBGRA> TBasicColor<T, _Shuffle>::ToBGRA() const {
    return TBasicColor<T, FColorShuffleBGRA>(*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
auto TBasicColor<T, _Shuffle>::ToSRGB() const -> TBasicColor {
    return TBasicColor(Linear_to_SRGB(_data));
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
auto TBasicColor<T, _Shuffle>::ToLinear() const -> TBasicColor {
    return TBasicColor(SRGB_to_Linear(_data));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// http://chilliant.blogspot.fr/2012/08/srgb-approximations-for-hlsl.html
// Unreal : https://github.com/EpicGames/UnrealEngine/blob/dff3c48be101bb9f84633a733ef79c91c38d9542/Engine/Shaders/GammaCorrectionCommon.usf
//----------------------------------------------------------------------------
float SRGB_to_Linear(float srgb) {
#if 0
    return srgb * (srgb * (srgb * 0.305306011f + 0.682171111f) + 0.012522878f);
#elif 0
    return std::pow(srgb, 2.2f);
#else
	return (srgb > 0.04045f)
        ? std::pow(srgb * (1.0f / 1.055f) + 0.0521327f, 2.4f)
        : srgb * (1.0f / 12.92f);
#endif
}
//----------------------------------------------------------------------------
float Linear_to_SRGB(float lin) {
#if 0
    const float s1 = std::sqrt(lin);
    const float s2 = std::sqrt(s1);
    const float s3 = std::sqrt(s2);
    return 0.585122381f * s1 + 0.783140355f * s2 - 0.368262736f * s3;
#elif 0
    constexpr float e = 1/2.2f;
    return std::pow(lin, e);
#else
    return (lin >= 0.00313067f)
        ? std::pow(lin, (1.0f / 2.4f)) * 1.055f - 0.055f
        : lin * 12.92f;
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline TBasicColorData< TUNorm<u8> > SRGB_to_Linear(const TBasicColorData< TUNorm<u8> >& srgb) {
    return TBasicColorData< TUNorm<u8> >(
        SRGB_to_Linear(srgb.x()._data),
        SRGB_to_Linear(srgb.y()._data),
        SRGB_to_Linear(srgb.z()._data),
        srgb.w()
        );
}
//----------------------------------------------------------------------------
inline TBasicColorData< TUNorm<u16> > SRGB_to_Linear(const TBasicColorData< TUNorm<u16> >& srgb) {
    return TBasicColorData< TUNorm<u16> >(
        SRGB_to_Linear(srgb.x().Normalized()),
        SRGB_to_Linear(srgb.y().Normalized()),
        SRGB_to_Linear(srgb.z().Normalized()),
        srgb.w()
        );
}
//----------------------------------------------------------------------------
template <typename T>
TBasicColorData<T> SRGB_to_Linear(const TBasicColorData<T>& srgb) {
    return TBasicColorData<T>(
        SRGB_to_Linear(srgb.x()),
        SRGB_to_Linear(srgb.y()),
        SRGB_to_Linear(srgb.z()),
        srgb.w()
        );
}
//----------------------------------------------------------------------------
template <typename T>
TBasicColorData<T> Linear_to_SRGB(const TBasicColorData<T>& linear) {
    return TBasicColorData<T>(
        Linear_to_SRGB(linear.x()),
        Linear_to_SRGB(linear.y()),
        Linear_to_SRGB(linear.z()),
        linear.w()
        );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TBasicColorData<T> HSV_to_RGB(const TBasicColorData<T>& hsv) {
    return float4(HSV_to_RGB(hsv.xyz()), hsv.w());
}
//----------------------------------------------------------------------------
template <typename T>
TBasicColorData<T> RGB_to_HSV(const TBasicColorData<T>& rgb) {
    return float4(RGB_to_HSV(rgb.xyz()), rgb.w());
}
//----------------------------------------------------------------------------
template <typename T>
TBasicColorData<T> HSL_to_RGB(const TBasicColorData<T>& hsl) {
    return float4(HSL_to_RGB(hsl.xyz()), hsl.w());
}
//----------------------------------------------------------------------------
template <typename T>
TBasicColorData<T> RGB_to_HSL(const TBasicColorData<T>& rgb) {
    return float4(RGB_to_HSL(rgb.xyz()), rgb.w());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
