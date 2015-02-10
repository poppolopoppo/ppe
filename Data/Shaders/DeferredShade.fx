
#include "Lib/Platform/Config.fx"
#include "Lib/Color/HDR.fx"
#include "Lib/Color/SRGB.fx"
#include "Lib/GBuffer/Depth.fx"
#include "Lib/GBuffer/Layout.fx"
#include "Lib/GBuffer/Reconstruction.fx"
#include "Lib/Lighting/DirectionalLight.fx"
#include "Lib/Lighting/Environment.fx"
#include "Lib/Lighting/Geometry.fx"
#include "Lib/Lighting/Material.fx"
#include "Lib/Lighting/PointLight.fx"
#include "Lib/AutoAppIn.fx"

cbuffer PerFrame {
    float3      uniEyePosition;
    float4x4    uniFarCorners;
    float2      uniNearFarZ;
    float4x4    uniInvertViewProjection;
};

cbuffer Application {
    float4      uniMousePosition;
    float4      uniMouseButtons;
};

cbuffer Light {
    float3      uniSunDirection;
    float3      uniSRGB_uniSunColor;
};

struct PixelIn {
    float4 HPOS         : SV_POSITION;
    float2 TexCoord     : TEXCOORD0;
    float3 FarPos       : TEXCOORD1;
};

TEXTURECUBE(uniSRGB_uniLinearClamp_IrradianceMap);
TEXTURECUBE(uniSRGB_uniLinearClamp_ReflectionMap);

static float3 gFarCorners[4] = {
    uniFarCorners._11_12_13,
    uniFarCorners._21_22_23,
    uniFarCorners._31_32_33,
    uniFarCorners._41_42_43,
};

#include "Lib/Color/Ramp.fx"

PixelIn vmain(AppIn appIn) {

    float3 position = AppIn_Get_Position0(appIn).xyz;
    float2 texCoord = AppIn_Get_TexCoord0(appIn).xy;

    float4 clipPos = float4(position.xy, 0.5, 1.0);

    PixelIn o;
    o.HPOS = clipPos;
    o.TexCoord = texCoord;
    o.FarPos = gFarCorners[(int)position.z].xyz;

    return o;
}

float4 pmain(PixelIn pixelIn) : SV_Target {
    float depth = GBuffer::ReadDepth(pixelIn.TexCoord);
    GBuffer::Layout layout = GBuffer::ReadLayout(pixelIn.TexCoord);

    float linearDepth = GBuffer::LinearizeDepth_Eye_FarPlane(depth, uniNearFarZ.x, uniNearFarZ.y);
    float3 worldPos2 = lerp(uniEyePosition, pixelIn.FarPos, linearDepth);

    float3 worldPos = GBuffer::TextureSpaceToWorldSpace(pixelIn.TexCoord, uniInvertViewProjection);

    //return float4(linearDepth.xxx, 1);
    //return float4(Ramp::Rainbow(linearDepth), 1);
    return float4(Ramp::Rainbow(depth), 1);
    return float4(abs(worldPos - worldPos2), 1);

    Lighting::Geometry g;
    g.Eye = normalize(uniEyePosition - worldPos);
    g.Normal = GBuffer::GetNormal(layout);
    g.Position = worldPos;

    Lighting::DirectionalLight l;
    l.Color = uniSRGB_uniSunColor;
    l.Direction = uniSunDirection;
    l.Intensity = 1.5;

    Lighting::Environment e;
    e.AmbientIntensity = 0.1;
    e.ReflectionIntensity = 1.0;
    TEXTURECUBESTRUCT_ASSIGN(e, IrradianceMap, uniSRGB_uniLinearClamp_IrradianceMap);
    TEXTURECUBESTRUCT_ASSIGN(e, ReflectionMap, uniSRGB_uniLinearClamp_ReflectionMap);

    Lighting::Material m;
    m.Albedo = GBuffer::GetAlbedo(layout);
    m.Metallic = GBuffer::GetMetallic(layout);
    m.Roughness = GBuffer::GetRoughness(layout);
    m.SpecularColor = GBuffer::GetSpecularColor(layout);

    float3 shading = Lighting::Shade(g, m, e, l);

    if (uniMouseButtons.x > 0) {
        float2 mouseDxy = 1 - saturate(abs(uniMousePosition.xy - pixelIn.HPOS.xy)*0.5);
        shading.rg += mouseDxy;

        Lighting::PointLight p;
        p.Color = float3(1,0,0);
        p.Cutoff = 0.5;
        p.Intensity = 1;
        p.Position = GBuffer::TextureSpaceToWorldSpace(uniMousePosition.zw, uniInvertViewProjection);
        p.Radius = 3;

        //m.Albedo = shading;
        //
        shading = Lighting::Shade(g, m, e, p);
        //shading.rgb = linearDepth;
    }

    //shading = HDR::RomBinDaHouseToneMapping(shading);

    float4 result = float4(shading, 1);
    return result;
}
