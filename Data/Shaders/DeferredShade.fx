
#include "Lib/Platform/Config.fx"
#include "Lib/Color/SRGB.fx"
#include "Lib/GBuffer/Depth.fx"
#include "Lib/GBuffer/Layout.fx"
#include "Lib/Lighting/DirectionalLight.fx"
#include "Lib/Lighting/Environment.fx"
#include "Lib/Lighting/Geometry.fx"
#include "Lib/Lighting/Material.fx"
#include "Lib/AutoAppIn.fx"

cbuffer PerFrame {
    float4x4    uniFarCorners;
    float4x4    uniNearCorners;
    float2      uniNearFarZ;
    float3      uniSunDirection;
    float3      uniSRGB_uniSunColor;
};

struct PixelIn {
    float4 HPOS         : SV_POSITION;
    float2 TexCoord     : TEXCOORD0;
    float3 FarPos       : TEXCOORD1;
    float3 NearPos      : TEXCOORD2;
};

static const float3 gFarCorners[4] = {
    uniFarCorners._11_21_31,
    uniFarCorners._12_22_32,
    uniFarCorners._13_23_33,
    uniFarCorners._14_24_34,
};

static const float3 gNearCorners[4] = {
    uniNearCorners._11_21_31,
    uniNearCorners._12_22_32,
    uniNearCorners._13_23_33,
    uniNearCorners._14_24_34,
};

TEXTURECUBE(uniSRGB_uniLinearClamp_IrradianceMap);
TEXTURECUBE(uniSRGB_uniLinearClamp_ReflectionMap);

PixelIn vmain(AppIn appIn) {

    float3 position = AppIn_Get_Position0(appIn).xyz;
    float2 texCoord = AppIn_Get_TexCoord0(appIn).xy;

    float4 clipPos = float4(position.xy, 0.5, 1.0);

    PixelIn o;
    o.HPOS = clipPos;
    o.TexCoord = texCoord;
    o.FarPos = gFarCorners[(int)position.z];
    o.NearPos = gNearCorners[(int)position.z];

    return o;
}

float4 pmain(PixelIn pixelIn) : SV_Target {

    GBuffer::Layout layout = GBuffer::ReadLayout(pixelIn.TexCoord);

    float depth = GBuffer::ReadDepth(pixelIn.TexCoord);
    float linearDepth = GBuffer::LinearizeDepth(depth, uniNearFarZ.x, uniNearFarZ.y);
    float3 worldPos = lerp(pixelIn.NearPos, pixelIn.FarPos, linearDepth);

    Lighting::Geometry g;
    g.Eye = normalize(pixelIn.NearPos - pixelIn.FarPos);
    g.Normal = GBuffer::Read_Normal(layout);

    Lighting::DirectionalLight l;
    l.Color = uniSRGB_uniSunColor;
    l.Direction = uniSunDirection;
    l.Intensity = 1.0;

    Lighting::Environment e;
    e.AmbientIntensity = 0.03;
    e.ReflectionIntensity = 1.0;
    TEXTURECUBESTRUCT_ASSIGN(e, IrradianceMap, uniSRGB_uniLinearClamp_IrradianceMap);
    TEXTURECUBESTRUCT_ASSIGN(e, ReflectionMap, uniSRGB_uniLinearClamp_ReflectionMap);

    Lighting::Material m;
    m.Albedo = GBuffer::Read_Albedo(layout);
    m.Metallic = GBuffer::Read_Metallic(layout);
    m.Roughness = GBuffer::Read_Roughness(layout);
    m.SpecularColor = GBuffer::Read_SpecularColor(layout);

    float3 shading = Lighting::Shade(g, m, e, l);

    float4 result = float4(shading, 1);
    return result;
}
