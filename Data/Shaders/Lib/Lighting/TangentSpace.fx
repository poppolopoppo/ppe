#ifndef _LIB_LIGHTING_TANGENTSPACE_FX_INCLUDED
#define _LIB_LIGHTING_TANGENTSPACE_FX_INCLUDED

namespace Lighting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ENABLE_BUMP_MAPPING 1
#define ENABLE_3DC_NORMAL_PACKING 1
//----------------------------------------------------------------------------
float3 TangentSpaceNormal(
    TEXTURE2DPARAM_DECL(normalMap), float2 texcoord,
    float3 tangent, float3 binormal, float3 normal,
    float normalDepth = 1.0 ) {
#if ENABLE_BUMP_MAPPING

    float3x3 tangentSpace;
    tangentSpace[0] = normalize(tangent);
    tangentSpace[1] = normalize(binormal);
    tangentSpace[2] = normalize(normal);

    float4 textureNormal = TEX2D(normalMap, texcoord).rgba;

#   if ENABLE_3DC_NORMAL_PACKING
    // 3Dc/DXN unpacking
    normal.x = textureNormal.r * textureNormal.a;
    normal.y = textureNormal.g;
    normal.xy = (normal.xy * 2 - 1) * normalDepth;
    normal.z = sqrt(1.0 + 1e-4 - saturate(normal.x*normal.x + normal.y*normal.y));
#   else
    normal.xyz = 2 * textureNormal.xyz - 1;
#   endif

    return mul(normal, tangentSpace);

#else
    return normal;

#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Packing tangent space in a quaternion
// https://dl.dropboxusercontent.com/u/16861957/gdc2015_rendering_the_world_of_far_cry_4.pdf
//----------------------------------------------------------------------------
float4 TangentSpaceToQuaternion(float3 tangent, float3 binormal, float3 normal) {
    float4 quaternion;
    quaternion.x = normal.y - binormal.z;
    quaternion.y = tangent.z - normal.x;
    quaternion.z = binormal.x - tangent.y;
    quaternion.w = 1.0 + tangent.x + binormal.y + normal.z;
    return normalize(quaternion);
}
//----------------------------------------------------------------------------
void QuaternionToTangentSpace(  float4 quaternion,
                                out float3 tangent,
                                out float3 binormal,
                                out float3 normal ) {
    tangent     =   float3( 1.0, 0.0, 0.0)
                +   float3(-2.0, 2.0, 2.0) * quaternion.y * quaternion.yxw
                +   float3(-2.0,-2.0, 2.0) * quaternion.z * quaternion.zwx;
    binormal    =   float3( 0.0, 1.0, 0.0)
                +   float3( 2.0,-2.0, 2.0) * quaternion.z * quaternion.wzy
                +   float3( 2.0,-2.0,-2.0) * quaternion.x * quaternion.yxw;
    normal      =   float3( 0.0, 0.0, 1.0)
                +   float3( 2.0, 2.0,-2.0) * quaternion.x * quaternion.zwx
                +   float3(-2.0, 2.0,-2.0) * quaternion.y * quaternion.wzy;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lighting

#endif //!_LIB_LIGHTING_TANGENTSPACE_FX_INCLUDED
