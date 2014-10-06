#ifndef _LIB_LIGHTING_DIRECTIONALLIGHT_FX_INCLUDED
#define _LIB_LIGHTING_DIRECTIONALLIGHT_FX_INCLUDED

#include "Lib/Lighting/BRDF.fx"
#include "Lib/Lighting/Material.fx"

namespace BRDF {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct DirectionalLight {
    float3  Color;
    float3  Direction;
};
//----------------------------------------------------------------------------
float3 Eval(
    Material m,
    DirectionalLight light,
    float3 normal, float3 view,

    uniform int distribution_mode   = BRDF_DISTRIBUTION_GGX,
    uniform int geometry_mode       = BRDF_GEOMETRY_WALTER,
    uniform int diffuseenergy_mode  = BRDF_DIFFUSEENERGY_FRESNELDIFF

    ) {

    float3 halfVec = normalize(light.Direction + view);

    float NdotL = dot(normal, light.Direction);
    float NdotV = dot(normal, view);

    float NdotL_clamped = max(NdotL, 0.0);
    float NdotV_clamped = max(NdotV, 0.0);

    float brdf_spec =
        Fresnel_SchlickApprox(m.Fresnel0, halfVec, light.Direction) *
        Geometry(geometry_mode, normal, halfVec, view, light.Direction, m.Roughness) *
        Distribution(distribution_mode, normal, halfVec, m.Roughness) /
        (4.0 * NdotL_clamped * NdotV_clamped + 1e-8);

    float3 color_spec = NdotL_clamped * brdf_spec * (light.Color * fPI); // multipy with PI because from the definition of the punctual light source
    float3 color_diff =
        NdotL_clamped *
        DiffuseEnergyRatio(diffuseenergy_mode, m.Fresnel0, normal, light.Direction) *
        (m.Diffuse / fPI) * (light.Color * fPI); // multipy with PI because from the definition of the punctual light source

    return color_diff + color_spec;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace BRDF

#endif //!_LIB_LIGHTING_DIRECTIONALLIGHT_FX_INCLUDED
