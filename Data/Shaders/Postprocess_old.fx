#include "Lib/Platform/Config.fx"
#include "Lib/Color/HDR.fx"
#include "Lib/Color/PostProcess.fx"
#include "Lib/Color/SRGB.fx"
#include "Lib/Noise/Noise.fx"
#include "Lib/AutoAppIn.fx"

cbuffer PostprocessParams : register(b0) {
    float4 SceneTexture_DuDvDimensions;
    float  TimelineInSeconds;
};

SamplerState LinearClampSampler {
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

Texture2D<float3> SceneTexture : register(t0);
Texture2D<float2> ScreenDuDvTexture : register(t1);

struct PixelIn {
    float4 HPOS     : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

PixelIn VSMain(AppIn appIn) {

    float2 position = AppIn_Get_Position0(appIn);

    float4 clipPos = float4(position.xy, 0.5, 1.0);
    float2 texCoord = position.xy * 0.5 + 0.5;
    texCoord.y = 1 - texCoord.y;

    PixelIn o;
    o.HPOS = clipPos;
    o.TexCoord = texCoord;

    return o;
}

#define ENABLE_POSTPROCESS 1

float3 ApplyPostprocess(float2 uv) {

    float2 bentUV = PostProcess::RadialDistortion(uv, uv);
    float3 sceneColor = SceneTexture.Sample(LinearClampSampler, bentUV, 0).rgb;

    float3 postProcessColor = sceneColor;
    //postProcessColor = PostProcess::Scanlines(uv, SceneTexture_DuDvDimensions.zw, postProcessColor);
    postProcessColor = PostProcess::Vignette(bentUV, postProcessColor);

    return postProcessColor;
}

float4 PSMain(PixelIn pixelIn) : SV_Target {

    float2 uv = pixelIn.TexCoord;
    //uv = ScreenDuDvTexture.Sample(LinearClampSampler, uv, 0).rg;

    if (0) {
        float noisy = Perlin4D(float4(uv * 10, TimelineInSeconds, Perlin3D(float3(1.3 + uv * 20, TimelineInSeconds*1.3))*2-1));;
        uv += SceneTexture_DuDvDimensions.xy * 10 * noisy;
    }

    float3 color;
#if ENABLE_POSTPROCESS
    color = ApplyPostprocess(uv);
#else
    color = SceneTexture.Sample(LinearClampSampler, uv, 0).rgb;
#endif

    HDR::Params hdrParams;
    hdrParams.Exposure = pow(2, 3);
    hdrParams.WhitePoint = 11.2;

    color = Color::SRGB_to_Linear(color);
    color = HDR::ToneMap(color, hdrParams);
    color = Color::Linear_to_SRGB(color);

    float4 result = float4(color, 1);

    //result.rg = 0;//frac(uv*30);
    //result.b = noisy * 0.5 + 0.5;

    return result;
}
