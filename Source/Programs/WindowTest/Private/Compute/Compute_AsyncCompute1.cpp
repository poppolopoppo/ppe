// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
/*
    Async compute with one render target and with synchronization
    between frames to avoid race condition.

  .--------------------.--------------------.
  |      frame 1       |      frame 2       |
  |----------.---------|----------.---------|
  | graphics |         | graphics |         |
  |----------|---------|----------|---------|
  |          | compute |          | compute |
  '----------'---------'----------'---------'
*/
bool Compute_AsyncCompute1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FGraphicsPipelineDesc gdesc;
    gdesc.AddShader(EShaderType::Vertex, EShaderLangFormat::VKSL_100, "main", R"#(
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
ARGS_IF_RHIDEBUG("Compute_AsyncCompute1_VS"));
    gdesc.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) out vec4  out_Color;

layout(location=0) in  vec3  v_Color;

void main() {
	out_Color = 1.0f - vec4(v_Color, 1.0);
}
)#"
ARGS_IF_RHIDEBUG("Compute_AsyncCompute1_PS"));

    FComputePipelineDesc cdesc;
    cdesc.AddShader(EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(compute)
#extension GL_ARB_shading_language_420pack : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(binding=0, rgba8) uniform image2D  un_Image;

void main ()
{
	ivec2	coord = ivec2(gl_GlobalInvocationID.xy);
	vec4	color = imageLoad( un_Image, coord );

	imageStore( un_Image, coord, 1.0f - color );
}
)#"
ARGS_IF_RHIDEBUG("Compute_AsyncCompute1_CS"));

    const uint2 viewSize{ 800, 600 };

    TAutoResource<FImageID> image{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(viewSize)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::ColorAttachment | EImageUsage::Storage | EImageUsage::TransferSrc)
        .SetQueues(EQueueUsage::Graphics | EQueueUsage::AsyncCompute),
        Default ARGS_IF_RHIDEBUG("RenderTarget")) };
    PPE_LOG_CHECK(WindowTest, image.Valid());

    TAutoResource<FGPipelineID> gppln{ fg, fg.CreatePipeline(gdesc ARGS_IF_RHIDEBUG("Compute_AsyncCompute1_G")) };
    PPE_LOG_CHECK(WindowTest, gppln.Valid());

    TAutoResource<FCPipelineID> cppln{ fg, fg.CreatePipeline(cdesc ARGS_IF_RHIDEBUG("Compute_AsyncCompute1_C")) };
    PPE_LOG_CHECK(WindowTest, cppln.Valid());

    PPipelineResources resources = NEW_REF(RHIPipeline, FPipelineResources);
    PPE_LOG_CHECK(WindowTest, fg.InitPipelineResources(resources.get(), cppln, FDescriptorSetID{ "0" }));

    bool dataIsCorrect = false;
    const auto onLoaded = [&dataIsCorrect](const FImageView& imageData) {
        const auto testPixel = [&imageData](float x, float y, const FRgba32f& color) -> bool {
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
        dataIsCorrect &= testPixel( 0.00f, -0.49f, FRgba32f{1.0f, 0.0f, 0.0f, 1.0f} );
		dataIsCorrect &= testPixel( 0.49f,  0.49f, FRgba32f{0.0f, 1.0f, 0.0f, 1.0f} );
		dataIsCorrect &= testPixel(-0.49f,  0.49f, FRgba32f{0.0f, 0.0f, 1.0f, 1.0f} );
		dataIsCorrect &= testPixel( 0.00f, -0.51f, FRgba32f{0.0f} );
		dataIsCorrect &= testPixel( 0.51f,  0.51f, FRgba32f{0.0f} );
		dataIsCorrect &= testPixel(-0.51f,  0.51f, FRgba32f{0.0f} );
		dataIsCorrect &= testPixel( 0.00f,  0.51f, FRgba32f{0.0f} );
		dataIsCorrect &= testPixel( 0.51f, -0.51f, FRgba32f{0.0f} );
		dataIsCorrect &= testPixel(-0.51f, -0.51f, FRgba32f{0.0f} );
    };

    FCommandBufferBatch cmd1{ fg.Begin(FCommandBufferDesc{ EQueueType::Graphics }
        .SetName("Graphics-1")
        .SetDebugFlags(EDebugFlags::Default)) };
    PPE_LOG_CHECK(WindowTest, !!cmd1);

    FCommandBufferBatch cmd2{ fg.Begin(FCommandBufferDesc{ EQueueType::AsyncCompute }
        .SetName("Compute-1")
        .SetDebugFlags(EDebugFlags::Default),
        { cmd1 }) };
    PPE_LOG_CHECK(WindowTest, !!cmd2);

    // frame 1
    {
        // graphics queue
        {
            FLogicalPassID renderPass = cmd1->CreateRenderPass(FRenderPassDesc{ viewSize }
                .AddTarget(ERenderTargetID::Color0, image, FLinearColor::White(), EAttachmentStoreOp::Store)
                .AddViewport(viewSize));
            PPE_LOG_CHECK(WindowTest, !!renderPass);

            cmd1->Task(renderPass, FDrawVertices{}
                .Draw(3)
                .SetPipeline(gppln)
                .SetTopology(EPrimitiveTopology::TriangleList));

            PFrameTask tDraw = cmd1->Task(FSubmitRenderPass{ renderPass });
            Unused(tDraw);

            PPE_LOG_CHECK(WindowTest, fg.Execute(cmd1));
        }
        // compute queue
        {
            resources->BindImage(FUniformID{ "un_Image" }, image);

            PFrameTask tComp = cmd2->Task(FDispatchCompute{}
                .SetPipeline(cppln)
                .AddResources(FDescriptorSetID{ "0" }, resources)
                .Dispatch(viewSize));

            PFrameTask tRead = cmd2->Task(FReadImage{}
                .SetImage(image, int2::Zero, viewSize)
                .SetCallback(onLoaded)
                .DependsOn(tComp));
            Unused(tRead);

            PPE_LOG_CHECK(WindowTest, fg.Execute(cmd2));
        }

        PPE_LOG_CHECK(WindowTest, fg.Flush());
    }

    FCommandBufferBatch cmd3{ fg.Begin(FCommandBufferDesc{ EQueueType::Graphics }
        .SetName("Graphics-2")
        .SetDebugFlags(EDebugFlags::Default),
        { cmd2  }) };
    PPE_LOG_CHECK(WindowTest, !!cmd3);

    FCommandBufferBatch cmd4{ fg.Begin(FCommandBufferDesc{ EQueueType::AsyncCompute }
        .SetName("Compute-2")
        .SetDebugFlags(EDebugFlags::Default),
        { cmd3 }) };
    PPE_LOG_CHECK(WindowTest, !!cmd4);

    // frame 2
    {
        // graphics queue
        {
            FLogicalPassID renderPass = cmd3->CreateRenderPass(FRenderPassDesc{ viewSize }
                .AddTarget(ERenderTargetID::Color0, image, FLinearColor::White(), EAttachmentStoreOp::Store)
                .AddViewport(viewSize));
            PPE_LOG_CHECK(WindowTest, !!renderPass);

            cmd3->Task(renderPass, FDrawVertices{}
                .Draw(3)
                .SetPipeline(gppln)
                .SetTopology(EPrimitiveTopology::TriangleList));

            PFrameTask tDraw = cmd3->Task(FSubmitRenderPass{ renderPass });
            Unused(tDraw);

            PPE_LOG_CHECK(WindowTest, fg.Execute(cmd3));
        }
        // compute queue
        {
            resources->BindImage(FUniformID{ "un_Image" }, image);

            PFrameTask tComp = cmd4->Task(FDispatchCompute{}
                .SetPipeline(cppln)
                .AddResources(FDescriptorSetID{ "0" }, resources)
                .Dispatch(viewSize));

            PFrameTask tRead = cmd4->Task(FReadImage{}
                .SetImage(image, int2::Zero, viewSize)
                .SetCallback(onLoaded)
                .DependsOn(tComp));
            Unused(tRead);

            PPE_LOG_CHECK(WindowTest, fg.Execute(cmd4));
        }

        PPE_LOG_CHECK(WindowTest, fg.Flush());
    }

    PPE_LOG_CHECK(WindowTest, fg.WaitIdle());

    PPE_LOG_CHECK(WindowTest, dataIsCorrect);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
