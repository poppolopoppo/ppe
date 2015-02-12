#ifndef _LIB_PLATFORM_COMMON_FX_INCLUDED
#define _LIB_PLATFORM_COMMON_FX_INCLUDED

static const float fEpsilon = 1e-10;
static const float fPI      = 3.1415926535897932384626433832795;
static const float fInvPI   = 0.31830988618379067153776752674503;
static const float f2PI     = 6.283185307179586;
static const float fPIOver2 = fPI/2;
static const float fPIOver4 = fPI/4;
static const float fTAU     = f2PI;
static const float fInvLOG2 = 1.4426950408889634073599246810019;

float3 mul_dehomogenize(float3 xyz, float4x4 m) {
    float4 xyzw = mul(float4(xyz, 1), m);
    return xyzw.xyz/xyzw.w;
}

float3 mul_dehomogenize(float4 xyzw, float4x4 worldIT) {
    xyzw = mul(xyzw, worldIT);
    return xyzw.xyz/xyzw.w;
}

float Sqr(float x) {
    return x * x;
}

#define TransformNormal(_Vector, _Mat) mul_dehomogenize(_Vector, _Mat)

#endif //!_LIB_PLATFORM_COMMON_FX_INCLUDED
