#ifndef _LIB_LIGHTING_BRDF_FX_INCLUDED
#define _LIB_LIGHTING_BRDF_FX_INCLUDED

namespace BRDF {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float Fresnel_SchlickApprox(float f0, float3 n, float3 l) {
    return f0 + (1.0 - f0) * pow(1.0 - dot(n, l), 5.0);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define BRDF_DISTRIBUTION_BECKMANN      0
#define BRDF_DISTRIBUTION_BLINNPHONG    1
#define BRDF_DISTRIBUTION_GGX           2
//----------------------------------------------------------------------------
float Distribution_Beckmann(float3 n, float3 h, float roughness) {
    float m_Sq = roughness * roughness;
    float NdotH_Sq = max(dot(n, h), 0.0);
    NdotH_Sq = NdotH_Sq * NdotH_Sq;
    return exp( (NdotH_Sq - 1.0)/(m_Sq*NdotH_Sq) )/ (fPI * m_Sq * NdotH_Sq * NdotH_Sq);
}
//----------------------------------------------------------------------------
float Distribution_BlinnPhong(float3 n, float3 h, float roughness) {
    float m = 2.0/(roughness*roughness) - 2.0;
    return (m + 2.0) * pow( max(dot(n, h), 0.0), m) / (2.0 * fPI);
}
//----------------------------------------------------------------------------
float Distribution_GGX(float3 n, float3 h, float roughness) {
    float m_Sq = roughness * roughness;
    float NdotH_Sq = max(dot(n, h), 0.0);
    NdotH_Sq = NdotH_Sq * NdotH_Sq;
    return m_Sq / (fPI * pow(NdotH_Sq * (m_Sq - 1) + 1, 2));
}
//----------------------------------------------------------------------------
float Distribution(uniform int mode, float3 n, float3 h, float roughness) {
    if (mode == BRDF_DISTRIBUTION_BECKMANN)
        return Distribution_Beckmann(n, h, roughness);
    else if (mode == BRDF_DISTRIBUTION_BLINNPHONG)
        return Distribution_BlinnPhong(n, h, roughness);
    else if (mode == BRDF_DISTRIBUTION_GGX)
        return Distribution_GGX(n, h, roughness);
    else
        return 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define BRDF_GEOMETRY_IMPLICIT          0
#define BRDF_GEOMETRY_COOKTORRANCE      1
#define BRDF_GEOMETRY_SCHLICK           2
#define BRDF_GEOMETRY_WALTER            3
//----------------------------------------------------------------------------
float Geometry_Implicit(float3 n, float3 h, float3 v, float3 l, float roughness) {
    return max(dot(n, l), 0.0) * max(dot(n, v), 0.0);
}
//----------------------------------------------------------------------------
float Geometry_CookTorrance(float3 n, float3 h, float3 v, float3 l, float roughness) {
    float NdotH = dot(n, h);
    float NdotL = dot(n, l);
    float NdotV = dot(n, v);
    float VdotH = dot(v, h);
    float NdotL_clamped = max(NdotL, 0.0);
    float NdotV_clamped = max(NdotV, 0.0);
    return min( min( 2.0 * NdotH * NdotV_clamped / VdotH, 2.0 * NdotH * NdotL_clamped / VdotH), 1.0);
}
//----------------------------------------------------------------------------
float Geometry_Schlick(float3 n, float3 h, float3 v, float3 l, float roughness) {
    float NdotL_clamped = max(dot(n, l), 0.0);
    float NdotV_clamped = max(dot(n, v), 0.0);
    float k = roughness * sqrt(2.0 / fPI);
    float one_minus_k = 1.0 - k;
    return ( NdotL_clamped / (NdotL_clamped * one_minus_k + k) ) * ( NdotV_clamped / (NdotV_clamped * one_minus_k + k) );
}
//----------------------------------------------------------------------------
float Geometry_Walter(float3 n, float3 h, float3 v, float3 l, float roughness) {
    float NdotV = dot(n, v);
    float NdotL = dot(n, l);
    float HdotV = dot(h, v);
    float HdotL = dot(h, l);
    float NdotV_clamped = max(NdotV, 0.0);
    float a = 1.0 / ( roughness * tan( acos(NdotV_clamped) ) );
    float a_Sq = a* a;

    float a_term;
    if (a < 1.6)
        a_term = (3.535 * a + 2.181 * a_Sq)/(1.0 + 2.276 * a + 2.577 * a_Sq);
    else
        a_term = 1.0;

    return  ( step(0.0, HdotL/NdotL) * a_term  ) *
            ( step(0.0, HdotV/NdotV) * a_term  ) ;
}
//----------------------------------------------------------------------------
float Geometry(uniform int mode, float3 n, float3 h, float3 v, float3 l, float roughness) {
    if (mode == BRDF_GEOMETRY_IMPLICIT)
        return Geometry_Implicit(n, h, v, l, roughness);
    else if (mode == BRDF_GEOMETRY_COOKTORRANCE)
        return Geometry_CookTorrance(n, h, v, l, roughness);
    else if (mode == BRDF_GEOMETRY_SCHLICK)
        return Geometry_Schlick(n, h, v, l, roughness);
    else if (mode == BRDF_GEOMETRY_WALTER)
        return Geometry_Walter(n, h, v, l, roughness);
    else
        return 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define BRDF_DIFFUSEENERGY_NONE         0
#define BRDF_DIFFUSEENERGY_FRESNELDIFF  1
#define BRDF_DIFFUSEENERGY_FRESNEL0     2
//----------------------------------------------------------------------------
float DiffuseEnergyRatio_None(float f0, float3 n, float3 l) {
    return 1.0;
}
//----------------------------------------------------------------------------
float DiffuseEnergyRatio_FresnelDiff(float f0, float3 n, float3 l) {
    return 1.0 - Fresnel_SchlickApprox(f0, n, l);
}
//----------------------------------------------------------------------------
float DiffuseEnergyRatio_Fresnel0(float f0, float3 n, float3 l) {
    return 1.0 - f0;
}
//----------------------------------------------------------------------------
float DiffuseEnergyRatio(uniform int mode, float f0, float3 n, float3 l) {
    if (mode == BRDF_DIFFUSEENERGY_NONE)
        return DiffuseEnergyRatio_None(f0, n, l);
    else if (mode == BRDF_DIFFUSEENERGY_FRESNELDIFF)
        return DiffuseEnergyRatio_FresnelDiff(f0, n, l);
    else if (mode == BRDF_DIFFUSEENERGY_FRESNEL0)
        return DiffuseEnergyRatio_Fresnel0(f0, n, l);
    else
        return 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace BRDF

#endif //!_LIB_LIGHTING_BRDF_FX_INCLUDED
