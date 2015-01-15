
#include "Lib/Platform/Config.fx"
#include "Lib/Color/SRGB.fx"
#include "Lib/Lighting/DirectionalLight.fx"
#include "Lib/AutoAppIn.fx"

cbuffer PerFrame {
    float4x4    uniView;
    float4x4    uniProjection;
    float3      uniEyePosition;
    float3      uniSunDirection;
    float3      uniSRGB_uniSunColor;
};

/*
cbuffer PerObject {
};
*/

TEXTURE2D(uniSRGB_uniLinearWrap_DiffuseMap);

TEXTURE2D(uniLinearWrap_AlphaMap);
TEXTURE2D(uniLinearWrap_NormalMap);

struct PixelIn {
    float4 HPOS     : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal   : NORMAL0;
    float3 Viewer   : NORMAL1;
#if WITH_BUMP_MAPPING
    float3 Tangent  : NORMAL2;
    float3 Binormal : NORMAL3;
#endif
};

PixelIn vmain(AppIn appIn) {
    float4 worldPos = float4(AppIn_Get_Position0(appIn)*0.025, 1);
    float4 viewPos = mul(uniView, worldPos);
    float4 clipPos = mul(uniProjection, viewPos);

    PixelIn o;
    o.HPOS = clipPos;
    o.TexCoord = AppIn_Get_TexCoord0(appIn);
    o.Normal = AppIn_Get_Normal0(appIn);
    o.Viewer = uniEyePosition - worldPos.xyz;

#if WITH_BUMP_MAPPING
    o.Tangent = AppIn_Get_Tangent0(appIn);
    o.Binormal = cross(o.Tangent, o.Normal);
#endif

    return o;
}

float2 LightingFuncGGX_FV(float dotLH, float roughness)
{
    float alpha = roughness*roughness;

    // F
    float F_a, F_b;
    float dotLH5 = pow(1.0f-dotLH,5);
    F_a = 1.0f;
    F_b = dotLH5;

    // V
    float vis;
    float k = alpha/2.0f;
    float k2 = k*k;
    float invK2 = 1.0f-k2;
    vis = rcp(dotLH*dotLH*invK2 + k2);

    return float2(F_a*vis,F_b*vis);
}

float LightingFuncGGX_D(float dotNH, float roughness)
{
    float alpha = roughness*roughness;
    float alphaSqr = alpha*alpha;
    float pi = 3.14159f;
    float denom = dotNH * dotNH *(alphaSqr-1.0) + 1.0f;

    float D = alphaSqr/(pi * denom * denom);
    return D;
}

float LightingFuncGGX_OPT3(float3 N, float3 V, float3 L, float roughness, float F0)
{
    float3 H = normalize(V+L);

    float dotNL = saturate(dot(N,L));
    float dotLH = saturate(dot(L,H));
    float dotNH = saturate(dot(N,H));

    float D = LightingFuncGGX_D(dotNH,roughness);
    float2 FV_helper = LightingFuncGGX_FV(dotLH,roughness);
    float FV = F0*FV_helper.x + (1.0f-F0)*FV_helper.y;
    float specular = dotNL * D * FV;

    return specular;
}

float4 pmain(PixelIn pixelIn) : SV_Target {
    float3 normal = normalize(pixelIn.Normal);
    float3 viewer = normalize(pixelIn.Viewer);

    float3 diffuseColor = TEX2D(uniSRGB_uniLinearWrap_DiffuseMap, pixelIn.TexCoord).rgb;

    BRDF::Material m;
    m.Diffuse = diffuseColor;
    m.Roughness = 0.3;
    m.Fresnel0 = BRDF::RefractiveIndex_to_Fresnel0(3.5);

    BRDF::DirectionalLight l;
    l.Ambient = 0.2;
    l.Direction = uniSunDirection;
    l.Color = uniSRGB_uniSunColor;

#if WITH_BUMP_MAPPING
    float3 tangent = normalize(pixelIn.Tangent);
    float3 binormal = normalize(pixelIn.Binormal);

    float3x3 tangentSpace;
    tangentSpace._11_12_13 = tangent;
    tangentSpace._21_22_23 = binormal;
    tangentSpace._31_32_33 = normal;
    tangentSpace = transpose(tangentSpace);

    float4 normalMap = TEX2D(uniLinearWrap_NormalMap, pixelIn.TexCoord).rgba;

    normal.x = normalMap.r * normalMap.a;
    normal.y = normalMap.g;
    normal.z = sqrt(1.001 - normal.x*normal.x - normal.y*normal.y);
    normal = mul(tangentSpace, normal);
#endif

    float3 diffuseWLight = BRDF::Eval(m, l, normal, viewer);

    //diffuseColor = 0.5;
    //diffuseWLight = saturate(max(0, dot(normal, uniSunDirection)) + 0.15) * diffuseColor;

    float alpha = 1.0;
#if WITH_SEPARATE_ALPHA
    alpha = TEX2D(uniLinearWrap_AlphaMap, pixelIn.TexCoord).r;
    clip(alpha - 35.0/255);
#endif

    float4 result = float4(diffuseWLight.rgb, alpha);
    result.rgb *= result.a;

    return result;
}
