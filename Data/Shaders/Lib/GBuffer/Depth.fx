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
// http://www.humus.name/temp/Linearize%20depth.txt
float LinearizeDepth_Eye_FarPlane(float depth, float near, float far) {
    return near / (far - depth * (far - near));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lighting

#endif //!_LIB_GBUFFER_DEPTH_FX_INCLUDED
