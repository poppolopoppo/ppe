#ifndef _LIB_LIGHTING_MATERIAL_FX_INCLUDED
#define _LIB_LIGHTING_MATERIAL_FX_INCLUDED

namespace BRDF {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct Material {
    float3  Diffuse;
    float   Roughness;
    float   Fresnel0;
};
//----------------------------------------------------------------------------
float RefractiveIndex_to_Fresnel0(float refractiveIndex) {
    return pow((1.0 - refractiveIndex)/(1.0 + refractiveIndex), 2);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace BRDF

#endif //!_LIB_LIGHTING_MATERIAL_FX_INCLUDED
