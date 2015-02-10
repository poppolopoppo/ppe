#ifndef _LIB_GBUFFER_LAYOUT_FX_INCLUDED
#define _LIB_GBUFFER_LAYOUT_FX_INCLUDED

TEXTURE2D(uniPointClamp_GBuffer0);
TEXTURE2D(uniPointClamp_GBuffer1);
TEXTURE2D(uniPointClamp_GBuffer2);

namespace GBuffer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct Layout {
    float4  Albedo_Metallic             : SV_Target0;
    float4  SpecularColor_Roughness     : SV_Target1;
    float3  Normal                      : SV_Target2;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float3  GetAlbedo(in Layout layout) {
    return layout.Albedo_Metallic.rgb;
}
//----------------------------------------------------------------------------
float   GetMetallic(in Layout layout) {
    return layout.Albedo_Metallic.a;
}
//----------------------------------------------------------------------------
float3  GetSpecularColor(in Layout layout) {
    return layout.SpecularColor_Roughness.rgb;
}
//----------------------------------------------------------------------------
float   GetRoughness(in Layout layout) {
    return layout.SpecularColor_Roughness.a;
}
//----------------------------------------------------------------------------
float3  GetNormal(in Layout layout) {
    return normalize(layout.Normal.xyz * 2 - 1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void    SetAlbedo(inout Layout layout, float3 albedo) {
    layout.Albedo_Metallic.rgb = albedo;
}
//----------------------------------------------------------------------------
void    SetMetallic(inout Layout layout, float metallic) {
    layout.Albedo_Metallic.a = metallic;
}
//----------------------------------------------------------------------------
void    SetSpecularColor(inout Layout layout, float3 specularColor) {
    layout.SpecularColor_Roughness.rgb = specularColor;
}
//----------------------------------------------------------------------------
void    SetRoughness(inout Layout layout, float roughness) {
    layout.SpecularColor_Roughness.a = roughness;
}
//----------------------------------------------------------------------------
void    SetNormal(inout Layout layout, float3 normal) {
    layout.Normal.xyz = normal * 0.5 + 0.5;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Layout ReadLayout(float2 uv) {
    Layout read;
    read.Albedo_Metallic = TEX2D(uniPointClamp_GBuffer0, uv).rgba;
    read.SpecularColor_Roughness = TEX2D(uniPointClamp_GBuffer1, uv).rgba;
    read.Normal = TEX2D(uniPointClamp_GBuffer2, uv).rgb;
    return read;
}
//----------------------------------------------------------------------------
Layout WriteLayout(float3 albedo, float metallic, float roughness, float3 specularColor, float3 normal) {
    Layout write;
    SetAlbedo(write, albedo);
    SetMetallic(write, metallic);
    SetSpecularColor(write, specularColor);
    SetRoughness(write, roughness);
    SetNormal(write, normal);
    return write;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lighting

#endif //!_LIB_GBUFFER_LAYOUT_FX_INCLUDED
