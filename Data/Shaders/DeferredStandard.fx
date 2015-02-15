
#include "Lib/Platform/Config.fx"
#include "Lib/Color/SRGB.fx"
#include "Lib/GBuffer/Layout.fx"
#include "Lib/Lighting/Material.fx"
#include "Lib/Lighting/TangentSpace.fx"
#include "Lib/AutoAppIn.fx"

#if 0 && defined(WITH_BUMP_MAPPING)
#   undef WITH_BUMP_MAPPING
#endif

cbuffer PerFrame {
    float4x4    uniViewProjection;
};

cbuffer PerMaterial {
    float       uniOptional_Metallic;
#ifdef WITH_BUMP_MAPPING
    float       uniOptional_NormalDepth;
#endif
    float       uniOptional_RefractiveIndex;
    float       uniOptional_Roughness;
};

cbuffer PerObject {
    float4x4    uniOptional_World;
    float4x4    uniOptional_uniInvertTranspose_World;
};

TEXTURE2D(uniSRGB_uniLinearWrap_DiffuseMap);

TEXTURE2D(uniLinearWrap_AlphaMap);
TEXTURE2D(uniLinearWrap_NormalMap);

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
    float4 clipPos = mul(worldPos, uniViewProjection);

    float3 normal = AppIn_Get_Normal0(appIn);
#if WITH_BUMP_MAPPING
    float3 binormal = AppIn_Get_Binormal0(appIn);
    float3 tangent = AppIn_Get_Tangent0(appIn);
#endif

    normal = mul(float4(normal, 1), uniOptional_uniInvertTranspose_World).xyz;
#if WITH_BUMP_MAPPING
    binormal = mul(float4(binormal, 1), uniOptional_uniInvertTranspose_World).xyz;
    tangent = mul(float4(tangent, 1), uniOptional_uniInvertTranspose_World).xyz;
#endif

    PixelIn o;
    o.HPOS = clipPos.xyzw;
    o.TexCoord = AppIn_Get_TexCoord0(appIn);
    o.Normal = normal;
    o.WorldPos = worldPos;
#if WITH_BUMP_MAPPING
    o.Binormal = binormal;
    o.Tangent = tangent;
#endif

    return o;
}

GBuffer::Layout pmain(PixelIn pixelIn) {
    float alpha = 1.0;
#if WITH_SEPARATE_ALPHA
    alpha = TEX2D(uniLinearWrap_AlphaMap, pixelIn.TexCoord).r;
    clip(alpha - 50.0/255);
#endif

#if WITH_BUMP_MAPPING
    float3 normal = Lighting::TangentSpaceNormal(
        TEXTURE2DPARAM_CALL(uniLinearWrap_NormalMap), pixelIn.TexCoord,
        pixelIn.Tangent, pixelIn.Binormal, pixelIn.Normal,
        uniOptional_NormalDepth );
#else
    float3 normal = normalize(pixelIn.Normal);
#endif

    float3 albedo = TEX2D(uniSRGB_uniLinearWrap_DiffuseMap, pixelIn.TexCoord).rgb;

    float3 specularColor = 1;
    specularColor *= Lighting::RefractiveIndex_to_Fresnel0(uniOptional_RefractiveIndex);

    GBuffer::Layout layout = (GBuffer::Layout)0;
    GBuffer::SetAlbedo(layout, albedo);
    GBuffer::SetMetallic(layout, uniOptional_Metallic);
    GBuffer::SetSpecularColor(layout, specularColor);
    GBuffer::SetRoughness(layout, uniOptional_Roughness);
    GBuffer::SetNormal(layout, normal);

    return layout;
}
