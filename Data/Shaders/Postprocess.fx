#include "Lib/Platform/Config.fx"
#include "Lib/Color/HDR.fx"
#include "Lib/Color/PostProcess.fx"
#include "Lib/Color/Sampling.fx"
#include "Lib/Color/SRGB.fx"
#include "Lib/Noise/Noise.fx"
#include "Lib/AutoAppIn.fx"

cbuffer PostprocessParams {
    float2  uniDuDv_Principal;
    float2  uniDuDv_Bloom;
    float2  uniDuDv_PrincipalBlur;
    float   uniProcessTotalSeconds;
};

TEXTURE2D(uniLinearClamp_Principal);
TEXTURE2D(uniLinearClamp_Bloom);
TEXTURE2D(uniLinearClamp_PrincipalBlur);

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

#if 0
    if (uv.x < 0.5)
        /*return TEX2DLOD(uniLinearClamp_Bloom, uv, 0).rgba;
    else //if (uv.x < 0.75)
        return TEX2DLOD(uniLinearClamp_Bloom, uv, 0).rgba;
    else*/
        return Sampling::Bicubic(TEXTURE2DPARAM_CALL(uniLinearClamp_Bloom), uv, uniDuDv_Bloom);
#endif

    PostProcess::Params pp = PostProcess::DefaultParams();

    float2 bentUV = uv;
    float3 color = PostProcess::CubicLensDistortion(
            bentUV,
            TEXTURE2DPARAM_CALL(uniLinearClamp_Principal),
            uniDuDv_Principal,
            TEXTURE2DPARAM_CALL(uniLinearClamp_PrincipalBlur),
            uniDuDv_PrincipalBlur,
            pp.CubicLensK, pp.CubicLensKCube,
            pp.CubicLensDime, pp.CubicLensBlur,
            pp.AberrationChannels );

    //return float4(color, 1);

#if 1
    bentUV = uv;
    color = TEX2D(uniLinearClamp_Principal, uv).rgb;
#endif

    HDR::Params hdrParams;
    hdrParams.Exposure = pp.Exposure;
    hdrParams.WhitePoint = pp.WhitePoint;

    color = HDR::ToneMap(color, hdrParams);

    float3 bloom = Sampling::Bicubic(TEXTURE2DPARAM_CALL(uniLinearClamp_Bloom), bentUV, uniDuDv_Bloom.xy).rgb;
    color = lerp(color, color + bloom, pp.BloomIntensity);
    color = saturate(color);

    //color = PostProcess::Grid(bentUV, uniDuDv_Principal, color, 28, 0.05);
    //color = PostProcess::Scanlines(bentUV, 1/uniDuDv_Principal.xy, color);
    color = (pp.CubicLensK > 0.0)
        ? PostProcess::Vignette(bentUV, uniDuDv_Principal.xy, color)
        : PostProcess::Vignette(uv, uniDuDv_Principal.xy, color);

    float4 result = float4(color, 1);

    //result.rg = 0;//frac(uv*30);
    //result.b = noisy * 0.5 + 0.5;

    return result;
}
