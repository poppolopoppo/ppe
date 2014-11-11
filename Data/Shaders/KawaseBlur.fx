#include "Lib/Platform/Config.fx"
#include "Lib/Color/HDR.fx"
#include "Lib/Noise/Rand.fx"
#include "Lib/AutoAppIn.fx"

cbuffer KawaseBlurParams {
    float4 uniDuDvDimensions_Input;
    float  uniPass;
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

#if 1
    // enhanced with randomness to avoid banding :
    float2 dudv = (0.5 + uniPass + Rand2(uv + uniPass) * 2.0) * uniDuDvDimensions_Input.xy * 1.5;
#else
    float2 dudv = (0.5 + uniPass) * uniDuDvDimensions_Input.xy;
#endif

    float2 du = float2(dudv.x, 0);
    float2 dv = float2(0, dudv.y);

    float3 output = 0;

    output += TEX2D(uniLinearClamp_Input, uv + du + dv).rgb;
    output += TEX2D(uniLinearClamp_Input, uv - du + dv).rgb;
    output += TEX2D(uniLinearClamp_Input, uv - du - dv).rgb;
    output += TEX2D(uniLinearClamp_Input, uv + du - dv).rgb;

    output *= 0.25;

    return float4(output.rgb, 1);
}
