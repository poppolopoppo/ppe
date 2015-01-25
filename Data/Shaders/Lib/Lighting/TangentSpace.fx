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
} //!namespace Lighting

#endif //!_LIB_LIGHTING_TANGENTSPACE_FX_INCLUDED
