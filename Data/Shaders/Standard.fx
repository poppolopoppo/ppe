
#include "Lib/Platform/Config.fx"
#include "Lib/Color/SRGB.fx"
#include "Lib/Lighting/DirectionalLight.fx"
#include "Lib/Lighting/Environment.fx"
#include "Lib/Lighting/Geometry.fx"
#include "Lib/Lighting/Material.fx"
#include "Lib/Lighting/TangentSpace.fx"
#include "Lib/AutoAppIn.fx"

#if 0 && defined(WITH_BUMP_MAPPING)
#   undef WITH_BUMP_MAPPING
#endif

cbuffer PerFrame {
    float4x4    uniView;
    float4x4    uniProjection;
    float3      uniEyePosition;
    float3      uniSunDirection;
    float3      uniSRGB_uniSunColor;
    float       uniWorldTotalSeconds;
};

cbuffer PerObject {
    float       uniOptional_Metallic;
#ifdef WITH_BUMP_MAPPING
    float       uniOptional_NormalDepth;
#endif
    float       uniOptional_RefractiveIndex;
    float       uniOptional_Roughness;
    float4x4    uniOptional_World;
    float4x4    uniOptional_uniInvertTranspose_World;
};

TEXTURE2D(uniSRGB_uniLinearWrap_DiffuseMap);

TEXTURE2D(uniLinearWrap_AlphaMap);
TEXTURE2D(uniLinearWrap_NormalMap);

TEXTURECUBE(uniSRGB_uniLinearClamp_IrradianceMap);
TEXTURECUBE(uniSRGB_uniLinearClamp_ReflectionMap);

struct PixelIn {
    float4 HPOS     : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal   : NORMAL0;
    float4 WorldPos : NORMAL1;
#if WITH_BUMP_MAPPING
    float3 Binormal : BINORMAL0;
    float3 Tangent  : TANGENT0;
#endif
};

PixelIn vmain(AppIn appIn) {
    float4 objectPos = float4(AppIn_Get_Position0(appIn), 1);
    float4 worldPos = mul(objectPos, uniOptional_World);
    float4 viewPos = mul(worldPos, uniView);
    float4 clipPos = mul(viewPos, uniProjection);

    PixelIn o;
    o.HPOS = clipPos.xyzw;
    o.TexCoord = AppIn_Get_TexCoord0(appIn);
    o.Normal = AppIn_Get_Normal0(appIn);
    o.WorldPos = worldPos;
#if WITH_BUMP_MAPPING
    o.Binormal = AppIn_Get_Binormal0(appIn);
    o.Tangent = AppIn_Get_Tangent0(appIn);
#endif

    return o;
}

float4 pmain(PixelIn pixelIn) : SV_Target {
    float alpha = 1.0;
#if WITH_SEPARATE_ALPHA
    alpha = TEX2D(uniLinearWrap_AlphaMap, pixelIn.TexCoord).r;
    clip(alpha - 50.0/255);
#endif

    Lighting::DirectionalLight l;
    l.Color = uniSRGB_uniSunColor;
    l.Direction = uniSunDirection;
    l.Intensity = 1.0;

    Lighting::Environment e;
    e.AmbientIntensity = 0.03;
    e.ReflectionIntensity = 1.0;
    TEXTURECUBESTRUCT_ASSIGN(e, IrradianceMap, uniSRGB_uniLinearClamp_IrradianceMap);
    TEXTURECUBESTRUCT_ASSIGN(e, ReflectionMap, uniSRGB_uniLinearClamp_ReflectionMap);

    Lighting::Geometry g;
    g.Eye = normalize(uniEyePosition - pixelIn.WorldPos.xyz/pixelIn.WorldPos.w);
#if WITH_BUMP_MAPPING
    g.Normal = Lighting::TangentSpaceNormal(TEXTURE2DPARAM_CALL(uniLinearWrap_NormalMap), pixelIn.TexCoord,
                                            pixelIn.Tangent, pixelIn.Binormal, pixelIn.Normal,
                                            uniOptional_NormalDepth );
#else
    g.Normal = normalize(pixelIn.Normal);
#endif

    Lighting::Material m;
    m.Albedo = TEX2D(uniSRGB_uniLinearWrap_DiffuseMap, pixelIn.TexCoord).rgb;
    m.Metallic = uniOptional_Metallic;
    m.RefractiveIndex = uniOptional_RefractiveIndex;
    m.Roughness = uniOptional_Roughness;
    m.SpecularColor = float3(1,1,1);

    float3 shading = Lighting::Shade(g, m, e, l);

    float4 result = float4(shading, alpha);
    result.rgb *= result.a;

    return result;
}
