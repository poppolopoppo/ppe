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
float LinearizeDepth(float depth, float near, float far) {
    return (2.0 * near) / (far + near - depth * (far - near));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lighting

#endif //!_LIB_GBUFFER_DEPTH_FX_INCLUDED
