#ifndef _LIB_GBUFFER_RECONSTRUCTION_FX_INCLUDED
#define _LIB_GBUFFER_RECONSTRUCTION_FX_INCLUDED

#include "Lib/GBuffer/Depth.fx"

namespace GBuffer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float3 ClipSpaceToWorldSpace(float2 clipPos, float depth, float4x4 invertViewProjection) {
    float4 hpos = float4(clipPos, depth, 1);
    float4 wpos = mul(hpos, invertViewProjection);
    return wpos.xyz / wpos.w;
}
//----------------------------------------------------------------------------
float3 ClipSpaceToWorldSpace(float2 clipPos, float4x4 invertViewProjection) {
    float2 uv = clipPos.xy * 0.5 + 0.5;
    uv.y = 1 - uv.y;
    float depth = ReadDepth(uv);
    return ClipSpaceToWorldSpace(clipPos, depth, invertViewProjection);
}
//----------------------------------------------------------------------------
float3 TextureSpaceToWorldSpace(float2 uv, float4x4 invertViewProjection) {
    float depth = ReadDepth(uv);
    float2 clipPos = float2(uv.x, 1 - uv.y) * 2 - 1;
    return ClipSpaceToWorldSpace(clipPos, depth, invertViewProjection);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lighting

#endif //!_LIB_GBUFFER_RECONSTRUCTION_FX_INCLUDED
