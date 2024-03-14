// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Debugger_ShaderDebugger2_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    if (not EnableShaderDebugging) {
        Unused(app);
        PPE_LOG(WindowTest, Warning, "Debugger_ShaderDebugger2_: skipped due to lack of debugger support (USE_PPE_RHIDEBUG={0})", USE_PPE_RHIDEBUG);
        return true;
    }

#if USE_PPE_RHIDEBUG
    IFrameGraph& fg = *app.RHI().FrameGraph();

    FGraphicsPipelineDesc desc;
    desc.AddShader(EShaderType::Vertex, EShaderLangFormat::VKSL_100 | EShaderLangFormat::EnableDebugTrace, "main", R"#(
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

void dbg_EnableTraceRecording (bool b) {}

void main()
{
	dbg_EnableTraceRecording( gl_VertexIndex == 1 || gl_VertexIndex == 2 );

	gl_Position	= vec4( g_Positions[gl_VertexIndex], 0.0, 1.0 );
	v_Color		= g_Colors[gl_VertexIndex];
}
)#"
ARGS_IF_RHIDEBUG("Debugger_ShaderDebugger2_VS"));
    desc.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100 | EShaderLangFormat::EnableDebugTrace, "main", R"#(
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) in  vec3  v_Color;
layout(location=0) out vec4  out_Color;

void dbg_EnableTraceRecording (bool b) {}

void main()
{
	dbg_EnableTraceRecording( int(gl_FragCoord.x) == 400 && int(gl_FragCoord.y) == 300 );

	out_Color.rgb = v_Color.rgb;
	out_Color.a   = fract(v_Color.r + v_Color.g + v_Color.b + 0.5f);
}
)#"
ARGS_IF_RHIDEBUG("Debugger_ShaderDebugger2_PS"));

    const uint2 viewSize{ 800, 600 };

    TAutoResource<FImageID> image{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(viewSize)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::ColorAttachment | EImageUsage_BlitTransferSrc),
        Default ARGS_IF_RHIDEBUG("RenderTarget")) };
    PPE_LOG_CHECK(WindowTest, image.Valid());

    TAutoResource<FGPipelineID> ppln{ fg, fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Debugger_ShaderDebugger2")) };
    PPE_LOG_CHECK(WindowTest, ppln.Valid());

    bool dataIsCorrect = false;
    bool shaderOutputIsCorrect = false;

    const auto onShaderTraceReady = [&shaderOutputIsCorrect](FStringView taskName, FStringView shaderName, EShaderStages stages, TMemoryView<const FString> output) {
        shaderOutputIsCorrect = true;

        shaderOutputIsCorrect &= (stages == EShaderStages::Vertex+EShaderStages::Fragment);
        shaderOutputIsCorrect &= (taskName == "DebuggableDraw");

        if (shaderName == "Debugger_ShaderDebugger2_VS") {
            const FStringView ref0 = R"#(//> gl_VertexIndex: int {1}
//> gl_InstanceIndex: int {0}
no source

//> (out): float4 {0.500000, 0.500000, 0.000000, 1.000000}
//  gl_VertexIndex: int {1}
26. gl_Position	= vec4( g_Positions[gl_VertexIndex], 0.0, 1.0 );

//> v_Color: float3 {0.000000, 1.000000, 0.000000}
//  gl_VertexIndex: int {1}
27. v_Color		= g_Colors[gl_VertexIndex];

)#";
            const FStringView ref1 = R"#(//> gl_VertexIndex: int {2}
//> gl_InstanceIndex: int {0}
no source

//> (out): float4 {-0.500000, 0.500000, 0.000000, 1.000000}
//  gl_VertexIndex: int {2}
26. gl_Position	= vec4( g_Positions[gl_VertexIndex], 0.0, 1.0 );

//> v_Color: float3 {0.000000, 0.000000, 1.000000}
//  gl_VertexIndex: int {2}
27. v_Color		= g_Colors[gl_VertexIndex];

)#";

            shaderOutputIsCorrect &= (output.size() == 2 and (
                (output[0] == ref0 and output[1] == ref1) or
                (output[1] == ref0 and output[0] == ref1) ));
            PPE_LOG_CHECKVOID(WindowTest, shaderOutputIsCorrect);
        }
        else if (shaderName == "Debugger_ShaderDebugger2_PS") {
            const FStringView ref2 = R"#(//> gl_FragCoord: float4 {400.500000, 300.500000, 0.000000, 1.000000}
//> gl_PrimitiveID: int {0}
//> v_Color: float3 {0.498333, 0.252083, 0.249583}
no source

//> out_Color: float3 {0.498333, 0.252083, 0.249583}
//  v_Color: float3 {0.498333, 0.252083, 0.249583}
15. out_Color.rgb = v_Color.rgb;

//> out_Color: float4 {0.498333, 0.252083, 0.249583, 0.500000}
//  v_Color: float3 {0.498333, 0.252083, 0.249583}
16. out_Color.a   = fract(v_Color.r + v_Color.g + v_Color.b + 0.5f);

)#";

            shaderOutputIsCorrect &= (output.size() == 1 and output[0] == ref2);
            PPE_LOG_CHECKVOID(WindowTest, shaderOutputIsCorrect);
        }
        else {
            shaderOutputIsCorrect = false;
            PPE_LOG_CHECKVOID(WindowTest, shaderOutputIsCorrect);
        }
    };

    fg.SetShaderDebugCallback(onShaderTraceReady);

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
        dataIsCorrect &= testPixel( 0.00f, -0.49f, FRgba32f{1.0f, 0.0f, 0.0f, 0.5f} );
        dataIsCorrect &= testPixel( 0.49f,  0.49f, FRgba32f{0.0f, 1.0f, 0.0f, 0.5f} );
        dataIsCorrect &= testPixel(-0.49f,  0.49f, FRgba32f{0.0f, 0.0f, 1.0f, 0.5f} );
        dataIsCorrect &= testPixel( 0.00f, -0.51f, FRgba32f{0.0f} );
        dataIsCorrect &= testPixel( 0.51f,  0.51f, FRgba32f{0.0f} );
        dataIsCorrect &= testPixel(-0.51f,  0.51f, FRgba32f{0.0f} );
        dataIsCorrect &= testPixel( 0.00f,  0.51f, FRgba32f{0.0f} );
        dataIsCorrect &= testPixel( 0.51f, -0.51f, FRgba32f{0.0f} );
        dataIsCorrect &= testPixel(-0.51f, -0.51f, FRgba32f{0.0f} );
    };

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("Debugger_ShaderDebugger2")
        .SetDebugFlags(EDebugFlags_Default)) };
    PPE_LOG_CHECK(WindowTest, !!cmd);

    FLogicalPassID renderPass = cmd->CreateRenderPass(FRenderPassDesc{ viewSize }
        .AddTarget(ERenderTargetID::Color0, image, FLinearColor(Zero), EAttachmentStoreOp::Store)
        .AddViewport(viewSize));
    PPE_LOG_CHECK(WindowTest, !!renderPass);

    cmd->Task(renderPass, FDrawVertices{}
        .Draw(3)
        .SetPipeline(ppln)
        .SetTopology(EPrimitiveTopology::TriangleList)
        .SetName("DebuggableDraw")
        .EnableShaderDebugTrace(EShaderStages::Vertex|EShaderStages::Fragment));

    const PFrameTask tDraw = cmd->Task(FSubmitRenderPass{ renderPass });
    const PFrameTask tRead = cmd->Task(FReadImage{}
        .SetImage(image, Zero, viewSize)
        .SetCallback(onLoaded)
        .DependsOn(tDraw));
    Unused(tRead);

    PPE_LOG_CHECK(WindowTest, fg.Execute(cmd));
    PPE_LOG_CHECK(WindowTest, fg.WaitIdle());

    PPE_LOG_CHECK(WindowTest, shaderOutputIsCorrect);
    PPE_LOG_CHECK(WindowTest, dataIsCorrect);
#endif

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
