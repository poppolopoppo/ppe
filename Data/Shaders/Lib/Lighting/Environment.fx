#ifndef _LIB_LIGHTING_ENVIRONMENT_FX_INCLUDED
#define _LIB_LIGHTING_ENVIRONMENT_FX_INCLUDED

namespace Lighting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct Environment {
    float AmbientIntensity;
    float ReflectionIntensity;
    TEXTURECUBESTRUCT_DECL(IrradianceMap);
    TEXTURECUBESTRUCT_DECL(ReflectionMap);
};
//----------------------------------------------------------------------------
float3 IrradianceSample(TEXTURECUBEPARAM_DECL(CubeMap), float3 dir) {
    return TEXCUBE(CubeMap, dir).rgb;
}
//----------------------------------------------------------------------------
float3 Irradiance(Environment e, float3 dir) {
    return IrradianceSample(TEXTURECUBESTRUCT_CALL(e, IrradianceMap), dir);
}
//----------------------------------------------------------------------------
float3 ReflectionSample(TEXTURECUBEPARAM_DECL(CubeMap), float3 dir, float lod) {
    return TEXCUBELOD(CubeMap, dir, lod).rgb;
}
//----------------------------------------------------------------------------
float3 Reflection(Environment e, float3 dir, float roughness) {
    return ReflectionSample(TEXTURECUBESTRUCT_CALL(e, ReflectionMap), dir, roughness*roughness*8.0f);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lighting

#endif //!_LIB_LIGHTING_ENVIRONMENT_FX_INCLUDED
