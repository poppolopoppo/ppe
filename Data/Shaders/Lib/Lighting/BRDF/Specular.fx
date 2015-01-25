#ifndef _LIB_LIGHTING_BRDF_SPECULAR_FX_INCLUDED
#define _LIB_LIGHTING_BRDF_SPECULAR_FX_INCLUDED

#include "Lib/Lighting/BRDF/Fresnel.fx"
#include "Lib/Lighting/BRDF/GeometricShadowing.fx"
#include "Lib/Lighting/BRDF/NormalDistribution.fx"

namespace Lighting {
namespace Specular {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define LIGHTING_NDF_BLINNPHONG                 0
#define LIGHTING_NDF_BECKMANN                   1
#define LIGHTING_NDF_GGX                        2
//----------------------------------------------------------------------------
#ifndef LIGHTING_NDF_MODE
#   define LIGHTING_NDF_MODE LIGHTING_NDF_GGX
#endif
//----------------------------------------------------------------------------
float D(float a, float NdH) {
#if     (LIGHTING_NDF_MODE == LIGHTING_NDF_BLINNPHONG)
    return Lighting::NormalDistribution::BlinnPhong(a, NdH);

#elif   (LIGHTING_NDF_MODE == LIGHTING_NDF_BECKMANN)
    return Lighting::NormalDistribution::Beckmann(a, NdH);

#elif   (LIGHTING_NDF_MODE == LIGHTING_NDF_GGX)
    return Lighting::NormalDistribution::GGX(a, NdH);

#else
#   error "BRDF: invalid normal distribution mode"

#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define LIGHTING_FRESNEL_NONE                   0
#define LIGHTING_FRESNEL_SCHLICK                1
#define LIGHTING_FRESNEL_COOKTORRANCE           2
//----------------------------------------------------------------------------
#ifndef LIGHTING_FRESNEL_MODE
#   define LIGHTING_FRESNEL_MODE LIGHTING_FRESNEL_SCHLICK
#endif
//----------------------------------------------------------------------------
float3 F(float3 specularColor, float3 h, float3 v) {
#if     (LIGHTING_FRESNEL_MODE == LIGHTING_FRESNEL_NONE)
    return Lighting::Fresnel::None(specularColor);

#elif   (LIGHTING_FRESNEL_MODE == LIGHTING_FRESNEL_SCHLICK)
    return Lighting::Fresnel::Schlick(specularColor, h, v);

#elif   (LIGHTING_FRESNEL_MODE == LIGHTING_FRESNEL_COOKTORRANCE)
    return Lighting::Fresnel::CookTorrance(specularColor, h, v);

#else
#   error "BRDF: invalid fresnel mode"

#endif
}
//----------------------------------------------------------------------------
float3 F_Roughness(float3 specularColor, float a, float3 h, float3 v) {
#if     (LIGHTING_FRESNEL_MODE == LIGHTING_FRESNEL_NONE)
    return Lighting::Fresnel::None(specularColor);

#elif   (LIGHTING_FRESNEL_MODE == LIGHTING_FRESNEL_SCHLICK)
    // Schlick using roughness to attenuate fresnel.
    return (specularColor + (max(1.0f-a, specularColor) - specularColor) * pow((1 - saturate(dot(v, h))), 5));

#elif   (LIGHTING_FRESNEL_MODE == LIGHTING_FRESNEL_COOKTORRANCE)
    return Lighting::Fresnel::CookTorrance(specularColor, h, v);

#else
#   error "BRDF: invalid fresnel mode"

#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define LIGHTING_GEOMETRIC_IMPLICIT             0
#define LIGHTING_GEOMETRIC_NEUMANN              1
#define LIGHTING_GEOMETRIC_COOKTORRANCE         2
#define LIGHTING_GEOMETRIC_KELEMEN              3
#define LIGHTING_GEOMETRIC_SMITH_BECKMANN       4
#define LIGHTING_GEOMETRIC_SMITH_GGX            5
#define LIGHTING_GEOMETRIC_SMITH_SCHLICK_GGX    6
//----------------------------------------------------------------------------
#ifndef LIGHTING_GEOMETRIC_MODE
#   define LIGHTING_GEOMETRIC_MODE LIGHTING_GEOMETRIC_SMITH_SCHLICK_GGX
#endif
//----------------------------------------------------------------------------
float G(float a, float NdV, float NdL, float NdH, float VdH, float LdV) {
#if     (LIGHTING_GEOMETRIC_MODE == LIGHTING_GEOMETRIC_IMPLICIT)
    return Lighting::GeometricShadowing::Implicit(a, NdV, NdL);

#elif   (LIGHTING_GEOMETRIC_MODE == LIGHTING_GEOMETRIC_NEUMANN)
    return Lighting::GeometricShadowing::Neumann(a, NdV, NdL);

#elif   (LIGHTING_GEOMETRIC_MODE == LIGHTING_GEOMETRIC_COOKTORRANCE)
    return Lighting::GeometricShadowing::CookTorrance(a, NdV, NdL, NdH, VdH);

#elif   (LIGHTING_GEOMETRIC_MODE == LIGHTING_GEOMETRIC_KELEMEN)
    return Lighting::GeometricShadowing::Kelemen(a, NdV, NdL, LdV);

#elif   (LIGHTING_GEOMETRIC_MODE == LIGHTING_GEOMETRIC_SMITH_BECKMANN)
    return Lighting::GeometricShadowing::Smith_Beckmann(a, NdV, NdL);

#elif   (LIGHTING_GEOMETRIC_MODE == LIGHTING_GEOMETRIC_SMITH_GGX)
    return Lighting::GeometricShadowing::Smith_GGX(a, NdV, NdL);

#elif   (LIGHTING_GEOMETRIC_MODE == LIGHTING_GEOMETRIC_SMITH_SCHLICK_GGX)
    return Lighting::GeometricShadowing::Smith_Schlick_GGX(a, NdV, NdL);

#else
#   error "BRDF: invalid geometric shadowing mode"

#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float3 Term(float3 specularColor, float3 h, float3 v, float3 l, float a, float NdL, float NdV, float NdH, float VdH, float LdV) {
    return ((D(a, NdH) * G(a, NdV, NdL, NdH, VdH, LdV)) * F(specularColor, v, h) ) / (4.0f * NdL * NdV + 0.0001f);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Specular
} //!namespace Lighting

#endif //!_LIB_LIGHTING_BRDF_SPECULAR_FX_INCLUDED
