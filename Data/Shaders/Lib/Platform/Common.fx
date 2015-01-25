#ifndef _LIB_PLATFORM_COMMON_FX_INCLUDED
#define _LIB_PLATFORM_COMMON_FX_INCLUDED

static const float fPI      = 3.1415926535897932384626433832795;
static const float fInvPI   = 0.31830988618379067153776752674503;
static const float f2PI     = 6.283185307179586;
static const float fPIOver2 = fPI/2;
static const float fPIOver4 = fPI/4;
static const float fTAU     = f2PI;
static const float fInvLOG2 = 1.4426950408889634073599246810019;

float3 TransformNormal(float4x4 worldIT, float3 normal) {
    float4 n = mul(worldIT, float4(normal, 1));
    return n.xyz/n.w;
}

float Sqr(float x) {
    return x * x;
}

#define mul_dehomogenize(_Mat, _Vector) \
    TransformNormal(_Mat, _Vector)

#endif //!_LIB_PLATFORM_COMMON_FX_INCLUDED
