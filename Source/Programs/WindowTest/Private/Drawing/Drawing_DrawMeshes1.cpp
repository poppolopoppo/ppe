// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Drawing_DrawMeshes1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    if (not (app.RHI().Features() & ERHIFeature::MeshDraw)) {
        PPE_LOG(WindowTest, Warning, "Drawing_DrawMeshes1_: skipped due to lack of mesh shader support");
        return true;
    }

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FMeshPipelineDesc desc;
    desc.AddShader(EShaderType::Mesh, EShaderLangFormat::VKSL_100, "main", R"#(
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_NV_mesh_shader : require

layout(local_size_x=3) in;
layout(triangles) out;
layout(max_vertices=3, max_primitives=1) out;

out gl_MeshPerVertexNV {
	vec4	gl_Position;
} gl_MeshVerticesNV[]; // [max_vertices]

layout(location=0) out MeshOutput {
	vec4	color;
} Output[]; // [max_vertices]

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

void main ()
{
	const uint I = gl_LocalInvocationID.x;

	gl_MeshVerticesNV[I].gl_Position	= vec4( g_Positions[I], 0.0, 1.0 );
	Output[I].color						= vec4( g_Colors[I], 1.0 );
	gl_PrimitiveIndicesNV[I]			= I;

	if ( I == 0 )
		gl_PrimitiveCountNV = 1;
}
)#"
ARGS_IF_RHIDEBUG("Drawing_DrawMeshes_MS"));
    desc.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100, "main", R"#(
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) in MeshOutput {
	vec4	color;
} Input;

layout(location=0) out vec4  out_Color;

void main() {
	out_Color = Input.color;
}
)#"
ARGS_IF_RHIDEBUG("Drawing_DrawMeshes_PS"));

    const uint2 viewSize{ 800, 600 };

    const TAutoResource<FImageID> image{ fg.ScopedResource(fg.CreateImage(
        FImageDesc{}.SetDimension(viewSize).SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::ColorAttachment | EImageUsage_BlitTransferSrc),
        Default ARGS_IF_RHIDEBUG("RenderTarget"))) };
    PPE_LOG_CHECK(WindowTest, image.Valid());

    const TAutoResource<FMPipelineID> ppln{ fg.ScopedResource(fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Drawing_DrawMeshes1"))) };
    PPE_LOG_CHECK(WindowTest, ppln.Valid());

    bool dataIsCorrect = false;
    const auto onLoaded = [&dataIsCorrect](const FImageView& imageData) {
        const auto testPixel = [&imageData](float x, float y, const FRgba32f& color) -> bool {
            const u32 ix = FPlatformMaths::RoundToUnsigned((x + 1.0f) * 0.5f * float(imageData.Dimensions().x) + 0.5f);
            const u32 iy = FPlatformMaths::RoundToUnsigned((y + 1.0f) * 0.5f * float(imageData.Dimensions().y) + 0.5f);

            FRgba32f texel;
            imageData.Load(&texel, uint3(ix, iy, 0));

            const bool isEqual = DistanceSq(color, texel) < LargeEpsilon;
            PPE_LOG(WindowTest, Debug, "Read({0}) -> {1} vs {2} == {3}", float2(x, y), texel, color, isEqual);
            PPE_LOG_CHECK(WindowTest, isEqual);
            Assert(isEqual);
            return true;
        };

        dataIsCorrect = true;
        dataIsCorrect &= testPixel( 0.00f, -0.49f, FLinearColor{ 1.0f, 0.0f, 0.0f, 1.0f });
        dataIsCorrect &= testPixel( 0.49f,  0.49f, FLinearColor{ 0.0f, 1.0f, 0.0f, 1.0f });
        dataIsCorrect &= testPixel(-0.49f,  0.49f, FLinearColor{ 0.0f, 0.0f, 1.0f, 1.0f });
        dataIsCorrect &= testPixel( 0.00f, -0.51f, Zero);
        dataIsCorrect &= testPixel( 0.51f,  0.51f, Zero);
        dataIsCorrect &= testPixel(-0.51f,  0.51f, Zero);
        dataIsCorrect &= testPixel( 0.00f,  0.51f, Zero);
        dataIsCorrect &= testPixel( 0.51f, -0.51f, Zero);
        dataIsCorrect &= testPixel(-0.51f, -0.51f, Zero);
    };

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("Drawing_DrawMeshes1")
        .SetDebugFlags(EDebugFlags_Default)) };
    PPE_LOG_CHECK(WindowTest, !!cmd);

    FLogicalPassID renderPass = cmd->CreateRenderPass(FRenderPassDesc{ viewSize }
        .AddTarget(ERenderTargetID::Color0, image, FLinearColor::Transparent(), EAttachmentStoreOp::Store)
        .AddViewport(viewSize));
    PPE_LOG_CHECK(WindowTest, !!renderPass);

    cmd->Task(renderPass, FDrawMeshes{}.Draw(1).SetPipeline(ppln));

    const PFrameTask tDraw = cmd->Task(FSubmitRenderPass{ renderPass });
    const PFrameTask tRead = cmd->Task(FReadImage{}.SetImage(image, Default, viewSize).SetCallback(onLoaded).DependsOn(tDraw));
    Unused(tRead);

    PPE_LOG_CHECK(WindowTest, fg.Execute(cmd));
    PPE_LOG_CHECK(WindowTest, fg.WaitIdle());

    PPE_LOG_CHECK(WindowTest, dataIsCorrect);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------->
} //!namespace PPE
