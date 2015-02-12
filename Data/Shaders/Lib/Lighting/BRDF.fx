#ifndef _LIB_LIGHTING_BRDF_FX_INCLUDED
#define _LIB_LIGHTING_BRDF_FX_INCLUDED

// http://www.alexandre-pestana.com/

#include "Lib/Lighting/BRDF/Specular.fx"

#include "Lib/Lighting/Environment.fx"
#include "Lib/Lighting/Geometry.fx"
#include "Lib/Lighting/Material.fx"

namespace Lighting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float3 LightingTerm(float3 albedoColor,
                    float3 specularColor,
                    float3 normal,
                    float  roughness,
                    float3 lightColor,
                    float3 lightDir,
                    float3 viewDir ) {
    // Compute some useful values.
    float NdL = saturate(dot(normal, lightDir));
    float NdV = saturate(dot(normal, viewDir));
    float3 h  = normalize(lightDir + viewDir);
    float NdH = saturate(dot(normal, h));
    float VdH = saturate(dot(viewDir, h));
    float LdV = saturate(dot(lightDir, viewDir));
    float a   = max(0.001f, roughness * roughness);

    float3 cDiff = Albedo_to_Diffuse(albedoColor);
    float3 cSpec = Specular::Term(specularColor, h, viewDir, lightDir, a, NdL, NdV, NdH, VdH, LdV);

    return lightColor * NdL * (cDiff * (1.0f - cSpec) + cSpec);
}
//----------------------------------------------------------------------------
float3 Shade(   Geometry g, Material m, Environment e,
                float3 lightDir, float3 lightColor, float lightAttenuation, float lightIntensity ) {
    float3 albedoColor = m.Albedo - m.Albedo * m.Metallic;

    float3 specularColor = lerp(m.SpecularColor, m.Albedo, m.Metallic);

    float3 lightingTerm = LightingTerm(albedoColor, specularColor, g.Normal, m.Roughness, lightColor, lightDir, g.Eye);

    float3 reflectVector = reflect(-g.Eye, g.Normal);
    float3 envColor = 0;
    [branch] if (e.ReflectionIntensity > 0)
        envColor = Reflection(e, reflectVector, m.Roughness);
    float3 irradiance = 0;
    [branch] if (e.AmbientIntensity > 0)
        irradiance = Irradiance(e, g.Normal);

    float3 envFresnel = Specular::F_Roughness(specularColor, m.Roughness*m.Roughness, g.Normal, g.Eye);

    return  lightAttenuation * lightIntensity * lightingTerm +
            envFresnel * envColor * e.ReflectionIntensity +
            albedoColor * irradiance * e.AmbientIntensity;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lighting

#endif //!_LIB_LIGHTING_BRDF_FX_INCLUDED
