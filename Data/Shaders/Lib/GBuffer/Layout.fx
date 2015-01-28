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
float3  Read_Albedo(in Layout layout) {
    return layout.Albedo_Metallic.rgb;
}
//----------------------------------------------------------------------------
float   Read_Metallic(in Layout layout) {
    return layout.Albedo_Metallic.a;
}
//----------------------------------------------------------------------------
float3  Read_SpecularColor(in Layout layout) {
    return layout.SpecularColor_Roughness.rgb;
}
//----------------------------------------------------------------------------
float   Read_Roughness(in Layout layout) {
    return layout.SpecularColor_Roughness.a;
}
//----------------------------------------------------------------------------
float3  Read_Normal(in Layout layout) {
    return normalize(layout.Normal.xyz * 2 - 1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void    Write_Albedo(inout Layout layout, float3 albedo) {
    layout.Albedo_Metallic.rgb = albedo;
}
//----------------------------------------------------------------------------
void    Write_Metallic(inout Layout layout, float metallic) {
    layout.Albedo_Metallic.a = metallic;
}
//----------------------------------------------------------------------------
void    Write_SpecularColor(inout Layout layout, float3 specularColor) {
    layout.SpecularColor_Roughness.rgb = specularColor;
}
//----------------------------------------------------------------------------
void    Write_Roughness(inout Layout layout, float roughness) {
    layout.SpecularColor_Roughness.a = roughness;
}
//----------------------------------------------------------------------------
void    Write_Normal(inout Layout layout, float3 normal) {
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
    Write_Albedo(write, albedo);
    Write_Metallic(write, metallic);
    Write_SpecularColor(write, specularColor);
    Write_Roughness(write, roughness);
    Write_Normal(write, normal);
    return write;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lighting

#endif //!_LIB_GBUFFER_LAYOUT_FX_INCLUDED
