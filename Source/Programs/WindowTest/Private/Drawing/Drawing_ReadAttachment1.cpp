// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Debugger_ReadAttachment1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FGraphicsPipelineDesc desc;
    desc.AddShader(EShaderType::Vertex, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) out vec3  v_Color;

const vec2	g_Positions[3] = vec2[](
	vec2(0.0, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
);

const vec3	g_Colors[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

void main() {
	gl_Position	= vec4( g_Positions[gl_VertexIndex], 0.0, 1.0 );
	v_Color		= g_Colors[gl_VertexIndex];
}
)#"
ARGS_IF_RHIDEBUG("Drawing_Draw_VS"));
        desc.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding=0) uniform sampler2D  un_DepthImage;

layout(location=0) out vec4  out_Color;

layout(location=0) in  vec3  v_Color;

void main() {
	out_Color = vec4(v_Color * texelFetch(un_DepthImage, ivec2(gl_FragCoord.xy), 0).r, 1.0);
}
)#"
ARGS_IF_RHIDEBUG("Drawing_Draw_PS"));

    TAutoResource<FGPipelineID> ppln{ fg.ScopedResource(fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Debugger_ReadAttachment1"))) };
    PPE_LOG_CHECK(WindowTest, ppln.Valid());

    const uint2 viewSize{ 800, 600 };

    EPixelFormat depthFormat{ Default };
    const EPixelFormat GAllDepthFormats[] = {
        EPixelFormat::Depth24_Stencil8,
        EPixelFormat::Depth32F_Stencil8,
        EPixelFormat::Depth16_Stencil8,
        EPixelFormat::Depth16 };
    for (EPixelFormat fmt : GAllDepthFormats) {
        FImageDesc test;
        test.SetDimension(viewSize).SetFormat(fmt).SetUsage(
            EImageUsage::Sampled |
            EImageUsage_BlitTransferDst |
            EImageUsage::DepthStencilAttachment );

        if (fg.IsSupported(test)) {
            depthFormat = fmt;
            PPE_LOG(WindowTest, Verbose, "selected depth format: {0}", fmt);
            break;
        }
    }
    PPE_LOG_CHECK(WindowTest, depthFormat != Default);

    TAutoResource<FImageID> colorImage{ fg.ScopedResource(
        fg.CreateImage(FImageDesc{}
            .SetDimension(viewSize)
            .SetFormat(EPixelFormat::RGBA8_UNorm)
            .SetUsage(EImageUsage::ColorAttachment | EImageUsage_BlitTransferSrc),
            Default ARGS_IF_RHIDEBUG("ColorTarget"))) };
    PPE_LOG_CHECK(WindowTest, !!colorImage);

    TAutoResource<FImageID> depthImage{ fg.ScopedResource(
        fg.CreateImage(FImageDesc{}
            .SetDimension(viewSize)
            .SetFormat(depthFormat)
            .SetUsage(EImageUsage::DepthStencilAttachment | EImageUsage_BlitTransferDst | EImageUsage::Sampled),
            Default ARGS_IF_RHIDEBUG("DepthTarget"))) };
    PPE_LOG_CHECK(WindowTest, !!depthImage);

    TAutoResource<FSamplerID> sampler{ fg.ScopedResource(
        fg.CreateSampler(FSamplerDesc{} ARGS_IF_RHIDEBUG("Sampler"))) };
    PPE_LOG_CHECK(WindowTest, !!sampler);

    PPipelineResources resources = NEW_REF(RHIPipeline, FPipelineResources);
    PPE_LOG_CHECK(WindowTest, fg.InitPipelineResources(resources.get(), ppln, FDescriptorSetID{ "0" }));

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
        .SetName("Debugger_ReadAttachment1")
        .SetDebugFlags(EDebugFlags_Default)) };
    PPE_LOG_CHECK(WindowTest, !!cmd);

    FLogicalPassID renderPass = cmd->CreateRenderPass(FRenderPassDesc{ viewSize }
        .AddTarget(ERenderTargetID::Color0, colorImage, FLinearColor::Transparent(), EAttachmentStoreOp::Store)
        .AddTarget(ERenderTargetID::Depth, depthImage, EAttachmentLoadOp::Load, EAttachmentStoreOp::Keep)
        .SetDepthTestEnabled(true)
        .SetDepthWriteEnabled(false)
        .AddViewport(viewSize));
    PPE_LOG_CHECK(WindowTest, !!renderPass);

    FImageViewDesc viewDesc;
    viewDesc.SetAspect(EImageAspect::Depth);
    resources->BindTexture(FUniformID{ "un_DepthImage" }, depthImage, sampler, viewDesc);

    cmd->Task(renderPass, FDrawVertices{}
        .Draw(3)
        .SetPipeline(ppln)
        .SetTopology(EPrimitiveTopology::TriangleList)
        .AddResources(FDescriptorSetID{ "0" }, resources));

    const PFrameTask tClear = cmd->Task(FClearDepthStencilImage{}.SetImage(depthImage).Clear(FDepthValue(1.f)).AddRange(0_mipmap, 1, 0_layer, 1));
    const PFrameTask tDraw = cmd->Task(FSubmitRenderPass{ renderPass }.DependsOn(tClear));
    const PFrameTask tRead = cmd->Task(FReadImage{}.SetImage(colorImage, int2::Zero, viewSize).SetCallback(onLoaded).DependsOn(tDraw));
    Unused(tRead);

    PPE_LOG_CHECK(WindowTest, fg.Execute(cmd));
    PPE_LOG_CHECK(WindowTest, fg.WaitIdle());

    PPE_LOG_CHECK(WindowTest, !!dataIsCorrect);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
