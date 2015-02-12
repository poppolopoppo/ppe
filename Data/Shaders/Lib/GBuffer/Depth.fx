#ifndef _LIB_GBUFFER_DEPTH_FX_INCLUDED
#define _LIB_GBUFFER_DEPTH_FX_INCLUDED

TEXTURE2D(uniPointClamp_DepthBuffer);

namespace GBuffer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float ReadDepth(float2 uv) {
    return TEX2D(uniPointClamp_DepthBuffer, uv).r;
}
//----------------------------------------------------------------------------
// http://www.humus.name/temp/Linearize%20depth.txt
float LinearizeDepth_NearPlane_FarPlane(float depth, float near, float far) {
    return depth / (far - depth * (far - near));
}
//----------------------------------------------------------------------------
float LinearizeDepth_Eye_FarPlane(float depth, float near, float far) {
#if     0
    // https://mynameismjp.wordpress.com/2010/09/05/position-from-depth-3/
    float projectionA = far / (far - near);
    float projectionB = (-far * near) / (far - near);
    return projectionB / (depth - projectionA);
#elif   0
    // http://www.humus.name/temp/Linearize%20depth.txt
    float c1 = far / near;
    float c0 = 1.0 - c1;
    return 1.0 / (c0 * depth + c1);
#elif   0
    // http://www.klayge.org/material/4_1/SSR/S2011_SecretsCryENGINE3Tech_0.pdf
    float2 projRatio = float2(far / (far - near), near / (near - far));
    return projRatio.y / (depth - projRatio.x);
#else
    // http://www.humus.name/temp/Linearize%20depth.txt
    return near / (far - depth * (far - near));
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lighting

#endif //!_LIB_GBUFFER_DEPTH_FX_INCLUDED
