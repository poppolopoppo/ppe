#ifndef _LIB_GBUFFER_DEPTH_FX_INCLUDED
#define _LIB_GBUFFER_DEPTH_FX_INCLUDED

TEXTURE2D(uniPointClamp_DepthBuffer);

namespace GBuffer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float Read_Depth(float2 uv) {
    return TEX2D(uniPointClamp_DepthBuffer, uv).r;
}
//----------------------------------------------------------------------------
float LinearizeDepth(float depth) {
    return depth; // TODO
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lighting

#endif //!_LIB_GBUFFER_DEPTH_FX_INCLUDED
