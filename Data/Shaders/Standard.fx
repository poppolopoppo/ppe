
#include "Lib/Platform/Config.fx"
#include "Lib/Color/SRGB.fx"
#include "Lib/Lighting/DirectionalLight.fx"
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

/*
cbuffer PerObject {
};
*/

TEXTURE2D(uniSRGB_uniLinearWrap_DiffuseMap);

TEXTURE2D(uniLinearWrap_AlphaMap);
TEXTURE2D(uniLinearWrap_NormalMap);

TEXTURECUBE(uniLinearWrap_IrradianceMap);
TEXTURECUBE(uniLinearWrap_ReflectionMap);

struct PixelIn {
    float4 HPOS     : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal   : NORMAL0;
    float3 Eye      : NORMAL1;
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
    o.HPOS = clipPos.xyzw;
    o.TexCoord = AppIn_Get_TexCoord0(appIn)*3;
    o.Normal = AppIn_Get_Normal0(appIn);
    o.Eye = worldPos.xyz - uniEyePosition;

#if WITH_BUMP_MAPPING
    o.Tangent = AppIn_Get_Tangent0(appIn);
    o.Binormal = AppIn_Get_Binormal0(appIn);
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

float3 TangentSpaceNormal(float2 texcoord, float3 tangent, float3 binormal, float3 normal) {
#if 0
    return normal;

#else
    float3x3 tangentSpace = {normalize(tangent), normalize(binormal), normal};
    tangentSpace = transpose(tangentSpace);

    float4 normalMap = TEX2D(uniLinearWrap_NormalMap, texcoord).rgba;

    // 3Dc/DXN unpacking
    normal.x = normalMap.r * normalMap.a;
    normal.y = normalMap.g;
    normal.xy = normal.xy * 2 - 1;
    normal.z = sqrt(1.0 + 1e-4 - saturate(normal.x*normal.x + normal.y*normal.y));

    return mul(tangentSpace, normal);
#endif
}

struct Material {
    float3  Albedo;
    float   Metallic;
    float   RefractiveIndex;
    float   Roughness;
};

struct DirectionalLight {
    float3  Color;
    float3  Direction;
};

struct Geometry {
    float3  Eye;
    float3  Normal;
};

float Schlick(float F0, float HdotV) {
    return F0 + (1.0 - F0) * pow((1.0 - HdotV), 5.0);
}

float RefractiveIndex_to_Fresnel0(float refractiveIndex) {
    return pow((refractiveIndex - 1.0)/(refractiveIndex + 1.0), 2.0);
}

float Schlick_RefractiveIndex(float ior, float HdotV) {
    float F0 = RefractiveIndex_to_Fresnel0(ior);
    return Schlick(F0, HdotV);
}

float D_Gtr2(float roughness, float NdotH) {
    float a2 = roughness * roughness;
    float term1 = fPI;
    float term2 = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    float deno = term1 * (term2 * term2);
    return a2 / deno;
}

float3 D_Gtr2_Sample(float roughness, float3 x, float3 y, float3 n, float3 v, float r) {
    float ax = roughness;
    float ay = ax;

    // Make up some kind of rx and ry.

    float rx = (r + n.x + n.y) / 3.0;
    float ry = (1.0 - r + n.z + n.x + rx) / (3.0 + rx);

    float  term1 = sqrt(ry / (1.0 - ry));
    float3 term2 = (ax * cos(2.0 * fPI * rx) * x) + (ay * sin(2.0 * fPI * rx) * y);

    float3 h = normalize(term1 * term2 + n);

    float3 L = (2.0 * dot(v, h) * h) - v;

    return TEXCUBELOD(uniLinearWrap_ReflectionMap, normalize(L), roughness * 2.0).xyz;
}

float Distribution_Beckmann(float roughness, float NdotH) {
    float r_sq = roughness * roughness;
    float roughness_a = 1.0f / ( 4.0f * r_sq * pow( NdotH, 4 ) );
    float roughness_b = NdotH * NdotH - 1.0f;
    float roughness_c = r_sq * NdotH * NdotH;
    return roughness_a * exp( roughness_b / roughness_c );
}

float Distribution_GGX(float roughness, float NdotH) {
    float r_sq = roughness * roughness;
    float NdotH_sq = NdotH * NdotH;
    return r_sq / (fPI * pow(NdotH_sq * (r_sq - 1) + 1, 2));
}

float Distribution_Gaussian(float roughness, float NdotH) {
   // This variable could be exposed as a variable
    // for the application to control:
    float c = 1.0f;
    float alpha = acos(NdotH);
    float r_sq = roughness * roughness;
    return c * exp( -(alpha / r_sq) );
}

// http://content.gpwiki.org/D3DBook:(Lighting)_Cook-Torrance
float3 SpecularTerm(float roughness, float3 H, float3 N, float3 V, float3 L, float F) {
    //  Cook-Torrance Microfacet model.
    //  D = microfacet slope distribution.
    //  G = geometric attenuation.
    //  F = fresnel coefficient.
    float NdotH = max(0.0000001, dot(N, H));
    float NdotV = max(0.0001, dot(N, V));
    float VdotH = max(0.0001, dot(V, H));
    float NdotL = max(0.0001, dot(N, L));

    //  Sample environment.
    /*float3 x = N.zyx;
    float3 y = normalize(cross(x, N));
    float3 refl = D_Gtr2_Sample(roughness, x, y, N, V, 0.01)
                + D_Gtr2_Sample(roughness, x, y, N, V, 0.11)
                + D_Gtr2_Sample(roughness, x, y, N, V, 0.21)
                + D_Gtr2_Sample(roughness, x, y, N, V, 0.31)
                + D_Gtr2_Sample(roughness, x, y, N, V, 0.41)
                + D_Gtr2_Sample(roughness, x, y, N, V, 0.51)
                + D_Gtr2_Sample(roughness, x, y, N, V, 0.61)
                + D_Gtr2_Sample(roughness, x, y, N, V, 0.71)
                + D_Gtr2_Sample(roughness, x, y, N, V, 0.81)
                + D_Gtr2_Sample(roughness, x, y, N, V, 0.91);
    refl *= F/10.0;*/

    float3 refl = TEXCUBELOD(uniLinearWrap_ReflectionMap, reflect(V, N), roughness*9).rgb;
    refl *= F;

    float G = min(  (2.0 * NdotH * NdotV) / VdotH,
                    (2.0 * NdotH * NdotL) / VdotH );
    G = min(1.0, G);

    //float D = D_Gtr2(roughness, NdotH);
    //float D = Distribution_Beckmann(roughness, NdotH);
    //float D = Distribution_Gaussian(roughness, NdotH);
    float D = Distribution_GGX(roughness, NdotH);

    //return ((F * D * G) / (NdotL * NdotV)) * float3(1,1,1) + refl;
    return ((F * D * G) / (4.0 * NdotL * NdotV)) * float3(1,1,1) + refl;
}

float3 DiffuseTerm(float3 N, float3 L, float F) {
    return (1.0 - F) * max(dot(N, L), 0) * float3(1,1,1);
}

// https://github.com/skurmedel/webglmat/blob/master/src/shaders/metal_fs.glsl
float3 Shade(Geometry g, Material m, DirectionalLight l) {
    float3 N = g.Normal;
    float3 V = g.Eye;
    float3 L = l.Direction;
    float3 H = normalize(L + V);
    float  F = Schlick_RefractiveIndex(m.RefractiveIndex, saturate(dot(H, V)) );

    float3 diffuse  = DiffuseTerm(N, L, F) * (m.Albedo * (1.0 - m.Metallic));
    float3 specular = SpecularTerm(m.Roughness, H, N, V, L, F) * lerp(float3(1,1,1), m.Albedo, m.Metallic);

    return diffuse + specular;
}

float3 ToneMap(float3 color, float gamma) {
    // kudos to Roman Galashov, aka RomBinDaHouse
    float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
    float3 toned = exp(-1.0 / (2.72 * color + 0.15));
    toned = pow(toned, 1.0 / gamma);
    return lerp(color, toned, 1.0 - luma);
}

float4 pmain(PixelIn pixelIn) : SV_Target {
    float alpha = 1.0;
#if WITH_SEPARATE_ALPHA
    alpha = TEX2D(uniLinearWrap_AlphaMap, pixelIn.TexCoord).r;
    clip(alpha - 50.0/255);
#endif

    Material m;
    m.Albedo = TEX2D(uniSRGB_uniLinearWrap_DiffuseMap, pixelIn.TexCoord).rgb;
    m.Metallic = 0.0;
    m.RefractiveIndex = 0.4;
    m.Roughness = 0.9;

    Geometry g;
    g.Normal = normalize(pixelIn.Normal);
    g.Eye = normalize(pixelIn.Eye);

#if WITH_BUMP_MAPPING
    g.Normal = TangentSpaceNormal(pixelIn.TexCoord, pixelIn.Tangent, pixelIn.Binormal, g.Normal);
#endif

    DirectionalLight l;
    l.Direction = uniSunDirection;
    l.Color = uniSRGB_uniSunColor;

    float3 shading = Shade(g, m, l);

    float4 result = float4(shading, alpha);
    //result.rgb = ToneMap(result.rgb, 2.3);
    result.rgb *= result.a;

    return result;
}

/**********************************/
/*
varying vec3 N;
varying vec3 p;
varying vec2 UV;
varying vec3 TN;
varying vec3 BTN;

uniform samplerCube env;
uniform float roughness;
uniform float ior;
uniform vec3 light_pos;
uniform float metallic;
uniform vec3 baseColor;

const float PI      = 3.141592;
const float PIOVER2 = PI / 2.0;
const float PIOVER4 = PI / 4.0;
const float TAU     = 6.283185;

struct directions
{
    vec3 H;
    vec3 L;
    vec3 N;
    vec3 V;

    vec3 TN;
    vec3 BTN;
};

float schlick(float F0, float HdotV)
{
    return F0 + (1.0 - F0) * pow((1.0 - HdotV), 5.0);
}

float schlick_ior(float ior1, float ior2, float HdotV)
{
    float F0 = (ior1 - ior2) / (ior1 + ior2);
    return schlick(F0 * F0, HdotV);
}

float d_gtr2(float roughness, float NdotH)
{
    float a2 = roughness * roughness;
    float term1 = PI;
    float term2 = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    float deno = term1 * (term2 * term2);
    return a2 / deno;
}

vec3 d_gtr2_sample(float roughness, vec3 x, vec3 y, vec3 n, vec3 v, float r)
{
    float ax = roughness;
    float ay = ax;

    float rx = (r + n.x + n.y) / 3.0;
    float ry = (1.0 - r + n.z + n.x + rx) / (3.0 + rx);

    float term1 = sqrt(ry / (1.0 - ry));
    vec3  term2 = (ax * cos(2.0 * PI * rx) * x) + (ay * sin(2.0 * PI * rx) * y);

    vec3 h = normalize(term1 * term2 + n);

    vec3 L = (2.0 * dot(v, h) * h) - v;

    return textureCube(env, normalize(L), roughness * 2.0).xyz;
}

vec3 compute_specular(float ior, float roughness, directions dir, float F)
{
    float NdotH = max(0.0, dot(dir.N, dir.H));
    float NdotV = max(0.0, dot(dir.N, dir.V));
    float VdotH = max(0.0, dot(dir.V, dir.H));
    float NdotL = max(0.0, dot(dir.N, dir.L));

    vec3 x = dir.N.zyx;
    vec3 y = normalize(cross(dir.N.zyx, dir.N));
    vec3 refl = d_gtr2_sample(roughness, x, y, dir.N, dir.V, 0.01)
              + d_gtr2_sample(roughness, x, y, dir.N, dir.V, 0.11)
              + d_gtr2_sample(roughness, x, y, dir.N, dir.V, 0.21)
              + d_gtr2_sample(roughness, x, y, dir.N, dir.V, 0.31)
              + d_gtr2_sample(roughness, x, y, dir.N, dir.V, 0.41)
              + d_gtr2_sample(roughness, x, y, dir.N, dir.V, 0.51)
              + d_gtr2_sample(roughness, x, y, dir.N, dir.V, 0.61)
              + d_gtr2_sample(roughness, x, y, dir.N, dir.V, 0.71)
              + d_gtr2_sample(roughness, x, y, dir.N, dir.V, 0.81)
              + d_gtr2_sample(roughness, x, y, dir.N, dir.V, 0.91);
         refl = refl * 1.0 * F;

    float G = min(
        1.0,
        min(
            (2.0 * NdotH * NdotV) / VdotH,
            (2.0 * NdotH * NdotL) / VdotH));
    float a = acos(NdotH);
    float D = d_gtr2(roughness, NdotH);

    return ((F * D * G) / (4.0 * NdotL * NdotV)) * vec3(1.0) + refl;
}

vec3 compute_diffuse(directions dir, float F)
{
    return (1.0 - F) * dot(dir.N, dir.L) * vec3(1.0);
}

void main()
{
    directions dir;

    dir.V = normalize((viewMatrix * vec4(cameraPosition, 1.0)).xyz - p);
    vec3 Lp = (viewMatrix * vec4(light_pos, 1.0)).xyz;
    dir.L = normalize(p - Lp);
    dir.H = normalize(dir.L + dir.V);
    // The normalized-normal, interpolated normals
    // might not be unit vectors.
    dir.N = normalize(N);
    dir.TN = normalize(TN);
    dir.BTN = normalize(BTN);

    float F = schlick_ior(ior, 1.0, dot(normalize(dir.V + dir.L), dir.V));

    vec3 diffuse = compute_diffuse(dir, F) * (baseColor * (1.0 - metallic));
    vec3 spec    = compute_specular(ior, roughness, dir, F) * mix(vec3(1.0), baseColor, metallic);

    gl_FragColor = vec4(diffuse + spec, 1.0);
}*/
