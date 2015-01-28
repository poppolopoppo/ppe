#ifndef _LIB_LIGHTING_MATERIAL_FX_INCLUDED
#define _LIB_LIGHTING_MATERIAL_FX_INCLUDED

namespace Lighting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct Material {
    float3  Albedo;
    float   Metallic;
    float   Roughness;
    float3  SpecularColor;
};
//----------------------------------------------------------------------------
float3 Albedo_to_Diffuse(float3 albedo) {
    return albedo * fInvPI;
}
//----------------------------------------------------------------------------
float RefractiveIndex_to_Fresnel0(float refractiveIndex) {
    return Sqr((refractiveIndex - 1.0)/(refractiveIndex + 1.0));
}
//----------------------------------------------------------------------------
float Glossiness_to_Roughness(float glossiness) {
    return 1.0 - glossiness;
}
//----------------------------------------------------------------------------
float Roughness_to_Glossiness(float glossiness) {
    return 1.0 - glossiness;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lighting

#endif //!_LIB_LIGHTING_MATERIAL_FX_INCLUDED
