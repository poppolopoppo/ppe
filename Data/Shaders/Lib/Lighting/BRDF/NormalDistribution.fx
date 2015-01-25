#ifndef _LIB_LIGHTING_BRDF_NORMAL_DISTRIBUTION_FX_INCLUDED
#define _LIB_LIGHTING_BRDF_NORMAL_DISTRIBUTION_FX_INCLUDED

namespace Lighting {
namespace NormalDistribution {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float GGX(float a, float NdH) {
    // Isotropic ggx.
    float a2 = a*a;
    float NdH2 = NdH * NdH;

    float denominator = NdH2 * (a2 - 1.0f) + 1.0f;
    denominator *= denominator;
    denominator *= fPI;

    return a2 / denominator;
}
//----------------------------------------------------------------------------
float BlinnPhong(float a, float NdH) {
    return (1 / (fPI * a * a)) * pow(NdH, 2 / (a * a) - 2);
}
//----------------------------------------------------------------------------
float Beckmann(float a, float NdH) {
    float a2 = a * a;
    float NdH2 = NdH * NdH;

    return (1.0f/(fPI * a2 * NdH2 * NdH2 + 0.001)) * exp( (NdH2 - 1.0f) / ( a2 * NdH2));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace NormalDistribution
} //!namespace Lighting

#endif //!_LIB_LIGHTING_BRDF_NORMAL_DISTRIBUTION_FX_INCLUDED
