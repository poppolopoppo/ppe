#pragma once

#include "Core/Color/Color.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
BasicColor<T, _Shuffle>::BasicColor() : _data(T(0)) {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
BasicColor<T, _Shuffle>::~BasicColor() {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
BasicColor<T, _Shuffle>::BasicColor(T broadcast)
:   _data(broadcast) {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
BasicColor<T, _Shuffle>::BasicColor(T x, T y, T z, T w)
:   _data(x, y, z, w) {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
BasicColor<T, _Shuffle>::BasicColor(const BasicColorData<T>& data)
:   _data(data) {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
BasicColor<T, _Shuffle>::BasicColor(const ScalarVector<float, 3>& xyz, float alpha)
:   _data(xyz.x(), xyz.y(), xyz.z(), alpha) {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
BasicColor<T, _Shuffle>::BasicColor(const BasicColor& other)
:   _data(other._data) {}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
auto BasicColor<T, _Shuffle>::operator =(const BasicColor& other) -> BasicColor& {
    _data = other._data;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
template <typename U, typename _Shuffle2>
BasicColor<T, _Shuffle>::BasicColor(const BasicColor<U, _Shuffle2>& other) {
    r() = other.r();
    g() = other.g();
    b() = other.b();
    a() = other.a();
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
template <typename U, typename _Shuffle2>
auto BasicColor<T, _Shuffle>::operator =(const BasicColor<U, _Shuffle2>& other) -> BasicColor& {
    r() = other.r();
    g() = other.g();
    b() = other.b();
    a() = other.a();
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
BasicColor<T, ColorShuffleRGBA> BasicColor<T, _Shuffle>::ToRGBA() const {
    return BasicColor<T, ColorShuffleRGBA>(*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
BasicColor<T, ColorShuffleBGRA> BasicColor<T, _Shuffle>::ToBGRA() const {
    return BasicColor<T, ColorShuffleBGRA>(*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
auto BasicColor<T, _Shuffle>::ToSRGB() const -> BasicColor {
    return BasicColor(Linear_to_SRGB(_data));
}
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
auto BasicColor<T, _Shuffle>::ToLinear() const -> BasicColor {
    return BasicColor(SRGB_to_Linear(_data));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// http://chilliant.blogspot.fr/2012/08/srgb-approximations-for-hlsl.html
//----------------------------------------------------------------------------
float SRGB_to_Linear(float srgb) {
    return srgb * (srgb * (srgb * 0.305306011f + 0.682171111f) + 0.012522878f);
}
//----------------------------------------------------------------------------
float Linear_to_SRGB(float lin) {
    const float s1 = std::sqrt(lin);
    const float s2 = std::sqrt(s1);
    const float s3 = std::sqrt(s2);
    return 0.585122381f * s1 + 0.783140355f * s2 - 0.368262736f * s3;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
BasicColorData<T> SRGB_to_Linear(const BasicColorData<T>& srgb) {
    return BasicColorData<T>(
        SRGB_to_Linear(srgb.x()),
        SRGB_to_Linear(srgb.y()),
        SRGB_to_Linear(srgb.z()),
        srgb.w()
        );
}
//----------------------------------------------------------------------------
template <typename T>
BasicColorData<T> Linear_to_SRGB(const BasicColorData<T>& linear) {
    return BasicColorData<T>(
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
BasicColorData<T> HSV_to_RGB(const BasicColorData<T>& hsv) {
    return float4(HSV_to_RGB(hsv.xyz()), hsv.w());
}
//----------------------------------------------------------------------------
template <typename T>
BasicColorData<T> RGB_to_HSV(const BasicColorData<T>& rgb) {
    return float4(RGB_to_HSV(rgb.xyz()), rgb.w());
}
//----------------------------------------------------------------------------
template <typename T>
BasicColorData<T> HSL_to_RGB(const BasicColorData<T>& hsl) {
    return float4(HSL_to_RGB(hsl.xyz()), hsl.w());
}
//----------------------------------------------------------------------------
template <typename T>
BasicColorData<T> RGB_to_HSL(const BasicColorData<T>& rgb) {
    return float4(RGB_to_HSL(rgb.xyz()), rgb.w());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
