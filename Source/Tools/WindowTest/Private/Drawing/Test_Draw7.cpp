#include "stdafx.h"

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Test_Draw7_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FGraphicsPipelineDesc desc;
    desc.AddShader(EShaderType::Vertex, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) out vec3  out_Color1;
layout(location=1) out vec3  out_Color2;

const vec2	g_Positions[3] = vec2[](
	vec2( 0.0,-0.5),
	vec2( 0.5, 0.5),
	vec2(-0.5, 0.5)
);

void main() {
	gl_Position	= vec4( g_Positions[gl_VertexIndex], 0.0, 1.0 );
	out_Color1	= vec3( 1.0, 0.0, 0.0 );
	out_Color2	= vec3( 0.0, 1.0, 0.0 );
}
)#"
ARGS_IF_RHIDEBUG("Test_Draw_VS"));
        desc.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) out vec4  out_Color1;
layout(location=1) out vec4  out_Color2;

layout(location=0) in vec3  in_Color1;
layout(location=1) in vec3  in_Color2;

void main() {
	out_Color1 = vec4(in_Color1, 1.0);
	out_Color2 = vec4(in_Color2, 1.0);
}
)#"
ARGS_IF_RHIDEBUG("Test_Draw_PS"));

    const uint2 viewSize{ 800, 600 };

    TAutoResource<FImageID> image1{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(viewSize)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::ColorAttachment | EImageUsage::TransferSrc),
        Default ARGS_IF_RHIDEBUG("RenderTarget_1")) };
    LOG_CHECK(WindowTest, image1.Valid());

    TAutoResource<FImageID> image2{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(viewSize)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::ColorAttachment | EImageUsage::TransferSrc),
        Default ARGS_IF_RHIDEBUG("RenderTarget_2")) };
    LOG_CHECK(WindowTest, image2.Valid());

    TAutoResource<FGPipelineID> ppln{ fg, fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Test_Draw7")) };
    LOG_CHECK(WindowTest, ppln.Valid());

    const auto testPixel = [](const FImageView& imageData, float x, float y, const FRgba32f& color) -> bool {
        const u32 ix = FPlatformMaths::RoundToUnsigned((x + 1.0f) * 0.5f * static_cast<float>(imageData.Dimensions().x) + 0.5f);
        const u32 iy = FPlatformMaths::RoundToUnsigned((y + 1.0f) * 0.5f * static_cast<float>(imageData.Dimensions().y) + 0.5f);

        FRgba32f texel;
        imageData.Load(&texel, uint3(ix, iy, 0));

        const bool isEqual = DistanceSq(color, texel) < F_LargeEpsilon;
        LOG(WindowTest, Debug, L"Read({0}) -> {1} vs {2} == {3}", uint2(ix, iy), texel, color, isEqual);
        LOG_CHECK(WindowTest, isEqual);
        Assert(isEqual);
        return isEqual;
    };

    bool dataIsCorrect = false;
    const auto onLoaded1 = [&dataIsCorrect, &testPixel](const FImageView& imageData) {
        dataIsCorrect = true;
        dataIsCorrect &= testPixel(imageData,  0.0f,  0.0f, FRgba32f{ 1.0f, 0.0f, 0.0f, 1.0f });
        dataIsCorrect &= testPixel(imageData,  0.0f, -0.7f, FRgba32f{ 0.0f });
        dataIsCorrect &= testPixel(imageData,  0.7f,  0.7f, FRgba32f{ 0.0f });
        dataIsCorrect &= testPixel(imageData, -0.7f,  0.7f, FRgba32f{ 0.0f });
        dataIsCorrect &= testPixel(imageData,  0.0f,  0.7f, FRgba32f{ 0.0f });
        dataIsCorrect &= testPixel(imageData,  0.7f, -0.7f, FRgba32f{ 0.0f });
        dataIsCorrect &= testPixel(imageData, -0.7f, -0.7f, FRgba32f{ 0.0f });
    };
    const auto onLoaded2 = [&dataIsCorrect, &testPixel](const FImageView& imageData) {
        dataIsCorrect = true;
        dataIsCorrect &= testPixel(imageData,  0.0f,  0.0f, FRgba32f{ 0.0f, 1.0f, 0.0f, 1.0f });
        dataIsCorrect &= testPixel(imageData,  0.0f, -0.7f, FRgba32f{ 0.0f });
        dataIsCorrect &= testPixel(imageData,  0.7f,  0.7f, FRgba32f{ 0.0f });
        dataIsCorrect &= testPixel(imageData, -0.7f,  0.7f, FRgba32f{ 0.0f });
        dataIsCorrect &= testPixel(imageData,  0.0f,  0.7f, FRgba32f{ 0.0f });
        dataIsCorrect &= testPixel(imageData,  0.7f, -0.7f, FRgba32f{ 0.0f });
        dataIsCorrect &= testPixel(imageData, -0.7f, -0.7f, FRgba32f{ 0.0f });
    };

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("Test_Draw7")
        .SetDebugFlags(EDebugFlags::Default)) };
    LOG_CHECK(WindowTest, !!cmd);

    FLogicalPassID renderPass = cmd->CreateRenderPass(FRenderPassDesc{ viewSize }
        .AddTarget(ERenderTargetID::Color0, image1, FLinearColor::Transparent(), EAttachmentStoreOp::Store)
        .AddTarget(ERenderTargetID::Color1, image2, FLinearColor::Transparent(), EAttachmentStoreOp::Store)
        .AddViewport(viewSize));
    LOG_CHECK(WindowTest, !!renderPass);

    cmd->Task(renderPass, FDrawVertices{}
        .Draw(3)
        .SetPipeline(ppln)
        .SetTopology(EPrimitiveTopology::TriangleList));

    const PFrameTask tDraw = cmd->Task(FSubmitRenderPass{ renderPass });
    const PFrameTask tRead1 = cmd->Task(FReadImage{}
        .SetImage(image1, int2{}, viewSize)
        .SetCallback(onLoaded1)
        .DependsOn(tDraw));
    const PFrameTask tRead2 = cmd->Task(FReadImage{}
        .SetImage(image2, int2{}, viewSize)
        .SetCallback(onLoaded2)
        .DependsOn(tDraw));
    Unused(tRead1);
    Unused(tRead2);

    LOG_CHECK(WindowTest, fg.Execute(cmd));
    LOG_CHECK(WindowTest, fg.WaitIdle());

    LOG_CHECK(WindowTest, dataIsCorrect);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
