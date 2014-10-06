#ifndef _LIB_COLOR_SAMPLING_FX_INCLUDED
#define _LIB_COLOR_SAMPLING_FX_INCLUDED

namespace Sampling {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float4 Bicubic(TEXTURE2DPARAM_DECL(input), float2 uv, float2 dudv) {
    float2 coord_hg = uv / dudv - 0.5;
    float2 index = floor(coord_hg);

    float2 f = coord_hg - index;
    static const float4x4 M = float4x4(
        -1, 3,-3, 1,
         3,-6, 3, 0,
        -3, 0, 3, 0,
         1, 4, 1, 0
        ) / 6.0;

    float4 wx = mul(float4(f.x*f.x*f.x, f.x*f.x, f.x, 1), M);
    float4 wy = mul(float4(f.y*f.y*f.y, f.y*f.y, f.y, 1), M);
    float2 w0 = float2(wx.x, wy.x);
    float2 w1 = float2(wx.y, wy.y);
    float2 w2 = float2(wx.z, wy.z);
    float2 w3 = float2(wx.w, wy.w);

    float2 g0 = w0 + w1;
    float2 g1 = w2 + w3;
    float2 h0 = w1 / g0 - 1;
    float2 h1 = w3 / g1 + 1;

    float2 coord00 = index + h0;
    float2 coord10 = index + float2(h1.x,h0.y);
    float2 coord01 = index + float2(h0.x,h1.y);
    float2 coord11 = index + h1;

    coord00 = (coord00 + 0.5) * dudv;
    coord10 = (coord10 + 0.5) * dudv;
    coord01 = (coord01 + 0.5) * dudv;
    coord11 = (coord11 + 0.5) * dudv;

    float4 tex00 = TEX2D(input, coord00);
    float4 tex10 = TEX2D(input, coord10);
    float4 tex01 = TEX2D(input, coord01);
    float4 tex11 = TEX2D(input, coord11);

    tex00 = lerp(tex01, tex00, g0.y);
    tex10 = lerp(tex11, tex10, g0.y);
    return lerp(tex10, tex00, g0.x);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Sampling

#endif //!_LIB_COLOR_SAMPLING_FX_INCLUDED
