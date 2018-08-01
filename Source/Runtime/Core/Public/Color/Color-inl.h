#pragma once

#include "Color/Color.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// http://chilliant.blogspot.fr/2012/08/srgb-approximations-for-hlsl.html
// Unreal : https://github.com/EpicGames/UnrealEngine/blob/dff3c48be101bb9f84633a733ef79c91c38d9542/Engine/Shaders/GammaCorrectionCommon.usf
//----------------------------------------------------------------------------
inline float SRGB_to_Linear(float srgb) {
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
inline float Linear_to_SRGB(float lin) {
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
inline float Linear_to_Pow22(float lin) {
    constexpr float e = 1 / 2.2f;
    return std::pow(lin, e);
}
//----------------------------------------------------------------------------
inline float Pow22_to_Linear(float pow22) {
    return std::pow(pow22, 2.2f);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FLinearColor Lerp(const FLinearColor& from, const FLinearColor& to, float t) {
    return FLinearColor(
        from.R * (1 - t) + to.R * t,
        from.G * (1 - t) + to.G * t,
        from.B * (1 - t) + to.B * t,
        from.A * (1 - t) + to.A * t );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
