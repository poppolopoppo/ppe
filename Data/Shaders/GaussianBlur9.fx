#include "Lib/Platform/Config.fx"
#include "Lib/Color/HDR.fx"
#include "Lib/AutoAppIn.fx"

cbuffer GaussianBlurParams {
    float4 uniDuDvDimensions_Input;
    float2 BlurDuDv;
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
    float2 dudv = BlurDuDv * uniDuDvDimensions_Input.xy;

    const float3 fOffsets = { 0.0, 1.3846153846, 3.2307692308 };
    const float3 fWeights = { 0.2270270270, 0.3162162162, 0.0702702703 };

    float3 blur = TEX2D(uniLinearClamp_Input, uv).rgb * fWeights[0];
    [unroll] for (int i = 1; i < 3; ++i) {
        blur += TEX2D(uniLinearClamp_Input, uv + fOffsets[i] * dudv).rgb * fWeights[i];
        blur += TEX2D(uniLinearClamp_Input, uv - fOffsets[i] * dudv).rgb * fWeights[i];
    }

    return float4(saturate(blur.rgb), 1);
}
