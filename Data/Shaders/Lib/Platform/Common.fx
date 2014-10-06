#ifndef _LIB_PLATFORM_COMMON_FX_INCLUDED
#define _LIB_PLATFORM_COMMON_FX_INCLUDED

#define fPI 3.1415926535897932384626433832795

float3 TransformNormal(float4x4 worldIT, float3 normal) {
    float4 n = mul(worldIT, float4(normal, 1));
    return n.xyz/n.w;
}

#define mul_dehomogenize(_Mat, _Vector) \
    TransformNormal(_Mat, _Vector)

#endif //!_LIB_PLATFORM_COMMON_FX_INCLUDED
