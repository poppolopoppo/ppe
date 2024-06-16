// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Drawing_Draw6_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FGraphicsPipelineDesc desc;
    desc.AddShader(EShaderType::Vertex, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// @set 0 PerObject
layout(set=0, binding=0, std140) uniform VertexPositionsUB {
	vec4	positions[3];
};

// @set 1 PerPass
layout(set=1, binding=0, std140) uniform VertexColorsUB {
	vec4	colors[3];
};

layout(location=0) out vec3  v_Color;
layout(location=1) out vec2  v_Texcoord;

void main() {
	gl_Position	= vec4( positions[gl_VertexIndex].xy, 0.0, 1.0 );
	v_Texcoord	= positions[gl_VertexIndex].xy * 0.5f + 0.5f;
	v_Color		= colors[gl_VertexIndex].rgb;
}
)#"
ARGS_IF_RHIDEBUG("Drawing_Draw_VS"));
        desc.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// @set 0 PerObject
layout(set=0, binding=1) uniform sampler2D  un_ColorTexture;

layout(location=0) out vec4  out_Color;

layout(location=0) in  vec3  v_Color;
layout(location=1) in  vec2  v_Texcoord;

void main() {
	out_Color = vec4(v_Color, 1.0) * texture( un_ColorTexture, v_Texcoord );
}
)#"
ARGS_IF_RHIDEBUG("Drawing_Draw_PS"));

    const uint2 viewSize{ 800, 600 };
    TAutoResource<FImageID> image{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(viewSize)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::ColorAttachment | EImageUsage_BlitTransferSrc),
        Default ARGS_IF_RHIDEBUG("RenderTarget")) };
    PPE_LOG_CHECK(WindowTest, image.Valid());

    const uint2 texSize{ 128, 128 };
    TAutoResource<FImageID> texture{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(texSize)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Sampled | EImageUsage_BlitTransferDst),
        Default ARGS_IF_RHIDEBUG("Texture")) };
    PPE_LOG_CHECK(WindowTest, texture.Valid());

    TAutoResource<FSamplerID> sampler{ fg, fg.CreateSampler(FSamplerDesc{}
        .SetFilter(ETextureFilter::Linear, ETextureFilter::Linear, EMipmapFilter::Nearest)
        ARGS_IF_RHIDEBUG("TextureSampler")) };
    PPE_LOG_CHECK(WindowTest, sampler.Valid());

    const float4 positionsRaw[] = { {0.0f, -0.5f, 0.0f, 0.0f},  {0.5f, 0.5f, 0.0f, 0.0f},  {-0.5f, 0.5f, 0.0f, 0.0f} };
    const float4 colorsRaw[] = { {1.0f, 0.0f, 0.0f, 1.0f},   {0.0f, 1.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f, 1.0f} };

    TAutoResource<FBufferID> positionsUb{ fg, fg.CreateBuffer(
        FBufferDesc{ sizeof(positionsRaw), EBufferUsage::Uniform },
        FMemoryDesc{ EMemoryType::HostWrite }
        ARGS_IF_RHIDEBUG("PositionsUB")) };
    PPE_LOG_CHECK(WindowTest, positionsUb.Valid());

    TAutoResource<FBufferID> colorsUb{ fg, fg.CreateBuffer(
        FBufferDesc{ sizeof(colorsRaw), EBufferUsage::Uniform },
        FMemoryDesc{ EMemoryType::HostWrite }
        ARGS_IF_RHIDEBUG("ColorsUB")) };
    PPE_LOG_CHECK(WindowTest, colorsUb.Valid());

    PPE_LOG_CHECK(WindowTest, fg.UpdateHostBuffer(positionsUb, 0_b, sizeof(positionsRaw), positionsRaw));
    PPE_LOG_CHECK(WindowTest, fg.UpdateHostBuffer(colorsUb, 0_b, sizeof(colorsRaw), colorsRaw));

    TAutoResource<FGPipelineID> ppln{ fg, fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Drawing_Draw6")) };
    PPE_LOG_CHECK(WindowTest, ppln.Valid());

    PPipelineResources resources0 = NEW_REF(RHIPipeline, FPipelineResources);
    PPE_LOG_CHECK(WindowTest, fg.InitPipelineResources(resources0.get(), ppln, FDescriptorSetID{"PerObject"}));

    PPipelineResources resources1 = NEW_REF(RHIPipeline, FPipelineResources);
    PPE_LOG_CHECK(WindowTest, fg.InitPipelineResources(resources1.get(), ppln, FDescriptorSetID{"PerPass"}));

    resources0->BindBuffer(FUniformID{ "VertexPositionsUB" }, positionsUb);
    resources0->BindTexture(FUniformID{ "un_ColorTexture" }, texture, sampler);
    resources1->BindBuffer(FUniformID{ "VertexColorsUB" }, colorsUb);

    bool dataIsCorrect = false;
    const auto onLoaded = [&dataIsCorrect](const FImageView& imageData) {
        const auto testTexel = [&imageData](float x, float y, const FRgba32f& color) -> bool {
            const u32 ix = FPlatformMaths::RoundToUnsigned((x + 1.0f) * 0.5f * static_cast<float>(imageData.Dimensions().x) + 0.5f);
            const u32 iy = FPlatformMaths::RoundToUnsigned((y + 1.0f) * 0.5f * static_cast<float>(imageData.Dimensions().y) + 0.5f);

            FRgba32f texel;
            imageData.Load(&texel, uint3(ix, iy, 0));

            const bool isEqual = DistanceSq(color, texel) < LargeEpsilon;
            PPE_LOG(WindowTest, Debug, "Read({0}) -> {1} vs {2} == {3}", uint2(ix, iy), texel, color, isEqual);
            PPE_LOG_CHECK(WindowTest, isEqual);
            Assert(isEqual);
            return isEqual;
        };

        dataIsCorrect = true;
        dataIsCorrect &= testTexel( 0.00f, -0.49f, FRgba32f{1.0f, 0.0f, 0.0f, 1.0f} );
		dataIsCorrect &= testTexel( 0.49f,  0.49f, FRgba32f{0.0f, 1.0f, 0.0f, 1.0f} );
		dataIsCorrect &= testTexel(-0.49f,  0.49f, FRgba32f{0.0f, 0.0f, 1.0f, 1.0f} );
		dataIsCorrect &= testTexel( 0.00f, -0.51f, FRgba32f{0.0f} );
		dataIsCorrect &= testTexel( 0.51f,  0.51f, FRgba32f{0.0f} );
		dataIsCorrect &= testTexel(-0.51f,  0.51f, FRgba32f{0.0f} );
		dataIsCorrect &= testTexel( 0.00f,  0.51f, FRgba32f{0.0f} );
		dataIsCorrect &= testTexel( 0.51f, -0.51f, FRgba32f{0.0f} );
		dataIsCorrect &= testTexel(-0.51f, -0.51f, FRgba32f{0.0f} );
    };

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("Drawing_Draw6")
        .SetDebugFlags(EDebugFlags_Default)) };
    PPE_LOG_CHECK(WindowTest, !!cmd);

    FLogicalPassID renderPass = cmd->CreateRenderPass(FRenderPassDesc{ viewSize }
        .AddTarget(ERenderTargetID::Color0, image, FLinearColor::Transparent(), EAttachmentStoreOp::Store)
        .AddViewport(viewSize));
    PPE_LOG_CHECK(WindowTest, !!renderPass);

    struct payload_t {
        FRawGPipelineID Pipeline;
        const PPipelineResources& Resources0;
        const PPipelineResources& Resources1;
    }   customPayload{
        *ppln.Get(), resources0, resources1
    };

    auto customDrawFunc_ = [](void* param, IDrawContext& ctx) {
        Unused(param);

        const payload_t& customPayload = *static_cast<payload_t*>(param);

        FInputAssemblyState ia;
        ia.Topology = EPrimitiveTopology::TriangleList;

        ctx.SetInputAssembly(ia);
        ctx.BindPipeline(customPayload.Pipeline);
        ctx.BindResources(FDescriptorSetID{ "PerObject" }, *customPayload.Resources0);
        ctx.BindResources(FDescriptorSetID{ "PerPass" }, *customPayload.Resources1);
        ctx.DrawVertices(3);
    };

    cmd->Task(renderPass, FCustomDraw{ customDrawFunc_, &customPayload }
        .AddImage(texture, EResourceState_ShaderSample));

    auto tClear = cmd->Task(FClearColorImage{}
        .SetImage(texture)
        .AddRange(0_mipmap, 1, 0_layer, 1)
        .Clear(FLinearColor::White()));
    auto tDraw = cmd->Task(FSubmitRenderPass{renderPass}
        .DependsOn(tClear));
    auto tRead = cmd->Task(FReadImage{}
        .SetImage(image, int2{}, viewSize)
        .SetCallback(onLoaded)
        .DependsOn(tDraw));
    Unused(tRead);

    PPE_LOG_CHECK(WindowTest, fg.Execute(cmd));
    PPE_LOG_CHECK(WindowTest, fg.WaitIdle());

    PPE_LOG_CHECK(WindowTest, dataIsCorrect);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
