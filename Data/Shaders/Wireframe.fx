
#include "Lib/Platform/Config.fx"
#include "Lib/AutoAppIn.fx"

cbuffer PerFrame {
    float4x4    uniView;
    float4x4    uniProjection;
    float3      uniEyePosition;
};

cbuffer PerObject {
    float4x4    uniOptional_World;
    float4x4    uniOptional_uniInvertTranspose_World;
};

struct GeometryIn {
    float4 HPOS         : SV_POSITION;
};

struct PixelIn {
    float4 HPOS         : SV_POSITION;
    float3 Barycentric  : TEXCOORD0;
};

GeometryIn vmain(AppIn appIn) {
    float4 objectPos = float4(AppIn_Get_Position0(appIn), 1);
    float4 worldPos = mul(objectPos, uniOptional_World);
    float4 viewPos = mul(worldPos, uniView);
    float4 clipPos = mul(viewPos, uniProjection);

    GeometryIn o;
    o.HPOS = clipPos;

    return o;
}

[maxvertexcount(3)]
void gmain(triangle GeometryIn corners[3], inout TriangleStream<PixelIn> stream) {
    PixelIn p;

    p.HPOS = corners[0].HPOS;
    p.Barycentric = float3(1,0,0);
    stream.Append(p);

    p.HPOS = corners[1].HPOS;
    p.Barycentric = float3(0,1,0);
    stream.Append(p);

    p.HPOS = corners[2].HPOS;
    p.Barycentric = float3(0,0,1);
    stream.Append(p);

    stream.RestartStrip();
}

float4 pmain(PixelIn pixelIn) : SV_Target {
    float3 color = 0.025;
    float3 grad = abs(ddx(pixelIn.Barycentric)) + abs(ddy(pixelIn.Barycentric));
    float3 aa = smoothstep(0.0, grad * 2.5, pixelIn.Barycentric);
    float alpha = 1 - min(aa.x, min(aa.y, aa.z));
    float minAlpha = 0.2;
    alpha = minAlpha + (1 - minAlpha) * alpha;
    clip(alpha - 50.0/255);
    float4 result = float4(color, alpha);
    result.rgb *= result.a;
    return result;
}
