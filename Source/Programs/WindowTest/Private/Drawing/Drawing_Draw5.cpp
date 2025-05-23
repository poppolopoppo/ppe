﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Drawing_Draw5_(FWindowTestApp& app) {
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

layout(location=0) out vec4  out_Color;

layout(location=0) in  vec3  v_Color;

void main() {
	out_Color = vec4(v_Color, 1.0);
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

    TAutoResource<FGPipelineID> ppln{ fg, fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Drawing_Draw5")) };
    PPE_LOG_CHECK(WindowTest, ppln.Valid());

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
        .SetName("Drawing_Draw5")
        .SetDebugFlags(EDebugFlags_Default)) };
    PPE_LOG_CHECK(WindowTest, !!cmd);

    FLogicalPassID renderPass = cmd->CreateRenderPass(FRenderPassDesc{ viewSize }
        .AddTarget(ERenderTargetID::Color0, image, FLinearColor::Transparent(), EAttachmentStoreOp::Store)
        .AddViewport(viewSize));
    PPE_LOG_CHECK(WindowTest, !!renderPass);

    auto customDrawFunc_ = [](void* param, IDrawContext& ctx) {
        FInputAssemblyState ia;
        ia.Topology = EPrimitiveTopology::TriangleList;

        ctx.BindPipeline(static_cast<TAutoResource<FGPipelineID>*>(param)->Raw());
        ctx.SetInputAssembly(ia);
        ctx.DrawVertices(3);
    };

    cmd->Task(renderPass, FCustomDraw{ customDrawFunc_, &ppln });

    auto tDraw = cmd->Task(FSubmitRenderPass{renderPass});
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
