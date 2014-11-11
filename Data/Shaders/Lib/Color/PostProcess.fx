#ifndef _LIB_COLOR_POSTPROCESS_FX_INCLUDED
#define _LIB_COLOR_POSTPROCESS_FX_INCLUDED

#include "Lib/Color/Sampling.fx"

namespace PostProcess {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct Params {
    float   Exposure;
    float   WhitePoint;

    float   BloomIntensity;

    float   CubicLensK;
    float   CubicLensKCube;
    float   CubicLensDime;
    float   CubicLensBlur;
    float3  AberrationChannels;

    float   Vignette;
};
//----------------------------------------------------------------------------
Params DefaultParams() {
    Params p;
    p.Exposure = pow(2, 3);
    p.WhitePoint = 11.2;
    p.BloomIntensity = 0.7;
    p.CubicLensK = -0.3;
    p.CubicLensKCube = 0.05;
    p.CubicLensDime = 12.0;
    p.CubicLensBlur = 25.0;
    p.AberrationChannels = float3(0.9,1.1,1.0);
    p.Vignette = 1;
    return p;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Apply radial distortion to the given coordinate.
float2 RadialDistortion(float2 coord, float2 pos, float distortion = 0.15) {
    float2 cc = pos - 0.5;
    float dist = dot(cc, cc) * distortion;
    return coord * (pos + cc * (1.0 + dist) * dist) / pos;
}
//----------------------------------------------------------------------------
float3 Vignette(float2 uv, float2 dudv, float3 color) {
    float2 vignette2 = saturate(min(uv, 1 - uv) * float2(1, dudv.x/dudv.y));
    vignette2 = 1 - pow(1 - abs(vignette2), 80);
    float vignette = saturate(dot(vignette2, vignette2.yx).x);

    return color * vignette;
}
//----------------------------------------------------------------------------
float3 Scanlines(float2 uv, float2 dimensions, float3 color) {
    float scanline = ((uv.y * dimensions.y) % 3) > 1;

    return lerp(color, scanline * color, 0.2);
}
//----------------------------------------------------------------------------
float3 Grid(float2 uv, float2 dudv, float3 color, float size = 32, float opacity = 0.05) {
    int2 grid2 = frac((uv.xy / dudv)/(2*size))>0.5;
    float grid = ((int)grid2.x ^ (int)grid2.y) * opacity + (1 - opacity);
    return color * grid;
}
//----------------------------------------------------------------------------
// http://www.francois-tarlier.com/blog/cubic-lens-distortion-shader/
float3 CubicLensDistortion(
        inout float2 uv
    //  input textures
    ,   TEXTURE2DPARAM_DECL(input)
    ,   float2 inputDuDv
    ,   TEXTURE2DPARAM_DECL(blured)
    ,   float2 bluredDuDv
    // lens distortion coefficient (between
    ,   float k = -0.15
    // cubic distortion value
    ,   float kcube = 0.5
    // dime on edges
    ,   float dime = 0.5
    // blur on edges
    ,   float blur = 1.0
    // which channels will be distorted (chromatic aberration)
    ,   float3 channels = float3(1,0,0)
    ) {
    // r2 = image_aspect*image_aspect*u*u + v*v
    // f = 1 + r2*(k + kcube*sqrt(r2))
    // u' = f*u
    // v' = f*v

    float r2 = (uv.x-0.5) * (uv.x-0.5) + (uv.y-0.5) * (uv.y-0.5);
    float f = 0;

    //only compute the cubic distortion if necessary
    if (kcube == 0.0) {
        f = 1 + r2 * k;
    }
    else {
        f = 1 + r2 * (k + kcube * sqrt(r2));
    }

    // get the right pixel for the current position
    float x = f*(uv.x-0.5)+0.5;
    float y = f*(uv.y-0.5)+0.5;
    float2 distorduv = float2(x, y);

    // chromatic aberration and distorsion
    float3 pdistord = 0;
    pdistord.r = TEX2D(input, lerp(uv, distorduv, channels.r)).r;
    pdistord.g = TEX2D(input, lerp(uv, distorduv, channels.g)).g;
    pdistord.b = TEX2D(input, lerp(uv, distorduv, channels.b)).b;

#if 1
    float3 pblured = Sampling::Bicubic(TEXTURE2DPARAM_CALL(blured), distorduv, bluredDuDv).rgb;
#else
    float3 pblured = TEX2D(blured, distorduv).rgb;
#endif

    float curve = saturate(abs(1 - f)/(0.0015+abs(1 - f)));

    float3 result = lerp(pdistord, pblured, pow(curve, blur));
    result *= 1 - pow(curve, dime);

    uv = distorduv;

    return result.rgb;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PostProcess

#endif //!_LIB_COLOR_POSTPROCESS_FX_INCLUDED
