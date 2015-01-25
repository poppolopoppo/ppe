#ifndef _LIB_LIGHTING_BRDF_FRESNEL_FX_INCLUDED
#define _LIB_LIGHTING_BRDF_FRESNEL_FX_INCLUDED

namespace Lighting {
namespace Fresnel {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float3 None(float3 specularColor) {
    return specularColor;
}
//----------------------------------------------------------------------------
float3 Schlick(float3 specularColor, float3 h, float3 v) {
    return (specularColor + (1.0f - specularColor) * pow((1.0f - saturate(dot(v, h))), 5));
}
//----------------------------------------------------------------------------
float3 CookTorrance(float3 specularColor, float3 h, float3 v) {
    float3 n = (1.0f + sqrt(specularColor)) / (1.0f - sqrt(specularColor));
    float c = saturate(dot(v, h));
    float3 g = sqrt(n * n + c * c - 1.0f);

    float3 part1 = (g - c)/(g + c);
    float3 part2 = ((g + c) * c - 1.0f)/((g - c) * c + 1.0f);

    return max(0.0f.xxx, 0.5f * part1 * part1 * ( 1 + part2 * part2));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Fresnel
} //!namespace Lighting

#endif //!_LIB_LIGHTING_BRDF_FRESNEL_FX_INCLUDED
