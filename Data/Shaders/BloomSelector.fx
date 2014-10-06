#include "Lib/Platform/Config.fx"
#include "Lib/Color/HDR.fx"
#include "Lib/Color/PostProcess.fx"
#include "Lib/AutoAppIn.fx"

cbuffer GaussianBlurParams {
    float4 uniDuDvDimensions_Input;
};

TEXTURE2D(uniLinearClamp_Input);

struct PixelIn {
    float4 HPOS     : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

PixelIn vmain(AppIn appIn) {

    float3 position = AppIn_Get_Position0(appIn).xyz;
    float2 texCoord = AppIn_Get_TexCoord0(appIn).xy;

    float4 clipPos = float4(position.xy, 0.5, 1.0);

    PixelIn o;
    o.HPOS = clipPos;
    o.TexCoord = texCoord;

    return o;
}

float4 pmain(PixelIn pixelIn) : SV_Target {

    float2 uv = pixelIn.TexCoord;

    float3 color = TEX2D(uniLinearClamp_Input, uv).rgb;

    PostProcess::Params pp = PostProcess::DefaultParams();

    HDR::Params hdrParams;
    hdrParams.Exposure = pp.Exposure;
    hdrParams.WhitePoint = pp.WhitePoint;

    float3 tone = HDR::Uncharted2Tonemap(hdrParams.Exposure * color);
    float3 selection = saturate(color / HDR::Uncharted2Tonemap(hdrParams.WhitePoint) - 1);

    return float4(selection.rgb, 1);
}
