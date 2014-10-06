
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
    float       uniProcessTotalSeconds;
};

cbuffer PerObject {
    float4x4    World;
    float4x4    uniInvertTranspose_World;
    float4      uniSRGB_InstanceColor;
};

TEXTURE2D(uniSRGB_Diffuse);
TEXTURE2D(uniAnisotropicClamp_Bump);

struct PixelIn {
    float4 HPOS     : SV_POSITION;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
    float3 Normal   : NORMAL0;
    float3 Viewer   : NORMAL1;
};

PixelIn vmain(AppIn appIn) {
    float4 worldPos = mul(World, float4(AppIn_Get_Position0(appIn), 1));
    float4 viewPos = mul(uniView, worldPos);
    float4 clipPos = mul(uniProjection, viewPos);

    float3 normal = mul_dehomogenize(uniInvertTranspose_World, AppIn_Get_Normal0(appIn));

    float4 color = AppIn_Get_Color0(appIn);
    color *= uniSRGB_InstanceColor;

    PixelIn o;
    o.HPOS = clipPos;
    o.TexCoord = AppIn_Get_TexCoord0(appIn);
    o.Color = color;
    o.Normal = normal;
    o.Viewer = normalize(uniEyePosition - worldPos.xyz);

    return o;
}

float4 pmain(PixelIn pixelIn) : SV_Target {
    float3 normal = normalize(pixelIn.Normal);
    float3 viewer = normalize(pixelIn.Viewer);

    float4 vertexColor = pixelIn.Color;
    float3 diffuseColor = TEX2D(uniSRGB_Diffuse, pixelIn.TexCoord).rgb;

    diffuseColor.rgb *= vertexColor.rgb;

    float3 textureNormal = TEX2D(uniAnisotropicClamp_Bump, pixelIn.TexCoord).rgb;
    textureNormal = textureNormal * 2 - 1;
    textureNormal = mul_dehomogenize(uniInvertTranspose_World, textureNormal);
    textureNormal = normalize(textureNormal);

    normal = textureNormal;

    BRDF::Material m;
    m.Diffuse = vertexColor.rgb * diffuseColor;
    m.Roughness = 0.025;
    //m.Roughness = pow(abs(cos(uniProcessTotalSeconds*0.5)), 7);
    m.Fresnel0 = BRDF::RefractiveIndex_to_Fresnel0(2.50);

    BRDF::DirectionalLight l;
    l.Direction = uniSunDirection;
    l.Color = uniSRGB_uniSunColor;

    float3 diffuseWLight = BRDF::Eval(m, l, normal, viewer);

    float4 result = float4(diffuseWLight.rgb, vertexColor.a);
    result.rgb *= result.a;

    return result;
}
