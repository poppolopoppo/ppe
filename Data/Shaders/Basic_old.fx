
#include "Lib/Platform/Config.fx"
#include "Lib/Color/SRGB.fx"
#include "Lib/Lighting/DirectionalLight.fx"
#include "Lib/AutoAppIn.fx"

cbuffer PerFrame    : register(b0) {
    float4x4 View;
    float4x4 Projection;
    float3   EyePosition;
    float3   LightDir;
};

cbuffer PerObject   : register(b1) {
    float4x4 World;
    float4x4 WorldInvertT;
};

SamplerState LinearClampSampler {
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

Texture2D<float3> DiffuseTexture    : register(t0);
Texture2D<float4> BumpTexture       : register(t1);

struct PixelIn {
    float4 HPOS     : SV_Position;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
    float3 Normal   : NORMAL0;
    float3 Viewer   : NORMAL1;
};

PixelIn VSMain(AppIn appIn) {

    float3 position = AppIn_Get_Position0(appIn) * 20;

    float4 worldPos = mul(float4(position, 1), World);
    float4 viewPos = mul(worldPos, View);
    float4 clipPos = mul(viewPos, Projection);

    float3 normal = AppIn_Get_Normal0(appIn);
    normal = mul((float3x3)WorldInvertT, normal);

    PixelIn o;
    o.HPOS = clipPos;
    o.TexCoord = AppIn_Get_TexCoord0(appIn);
    o.Color = AppIn_Get_Color0(appIn);
    o.Normal = normal;
    o.Viewer = normalize(EyePosition - worldPos.xyz);

    return o;
}

float4 PSMain(PixelIn pixelIn ) : SV_Target {

    float4 color = pixelIn.Color;
    float3 normal = normalize(pixelIn.Normal);
    float3 viewer = normalize(pixelIn.Viewer);

    float3 diffuseColor = DiffuseTexture.Sample(LinearClampSampler, pixelIn.TexCoord);
    diffuseColor = Color::SRGB_to_Linear(diffuseColor);

    float3 textureNormal = float3(BumpTexture.Sample(LinearClampSampler, pixelIn.TexCoord, 0).rgb);
    textureNormal = textureNormal * 2 - 1;
    //textureNormal = normalize(textureNormal);
    textureNormal = mul((float3x3)WorldInvertT, textureNormal.xyz);
    textureNormal = normalize(textureNormal);

    normal = textureNormal;

    //color.rgb = 1; // doubles the AO ..
    //diffuseColor.rgb = 0.2;

    BRDF::Material m;
    m.Diffuse = color.rgb * diffuseColor;
    m.Roughness = 0.1;
    m.Fresnel0 = BRDF::RefractiveIndex_to_Fresnel0(2.50);

    BRDF::DirectionalLight l;
    l.Direction = LightDir;
    l.Color = Color::SRGB_to_Linear(float3(1.0, 0.85, 0.8));

    float3 diffuseWLight = BRDF::Eval(m, l, normal, viewer);

    float4 result = float4(diffuseWLight.rgb, color.a);
    result.rgb *= result.a;

    //result.rgb = normal.xyz * 0.5 + 0.5;

    return Color::Linear_to_SRGB(result);
    //return result; // HDR
}
