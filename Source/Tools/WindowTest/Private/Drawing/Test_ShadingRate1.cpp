#include "stdafx.h"

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Test_ShadingRate1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    if (not (app.RHI().Features() & ERHIFeature::VariableShadingRate)) {
        LOG(WindowTest, Warning, L"Test_ShadingRate1_: skipped due to lack of shading rate support");
        return true;
    }

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FGraphicsPipelineDesc desc;
    desc.AddShader(EShaderType::Vertex, EShaderLangFormat::VKSL_100, "main", R"#(
const vec2	g_Positions[3] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0,  3.0),
	vec2( 3.0, -1.0)
);

void main() {
	gl_Position	= vec4( g_Positions[gl_VertexIndex], 0.0, 1.0 );
}
)#"
ARGS_IF_RHIDEBUG("Test_Draw_VS"));
        desc.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100, "main", R"#(
#extension GL_NV_shading_rate_image : require

layout(location=0) out vec4  out_Color;

//in ivec2 gl_FragmentSizeNV;
//in int   gl_InvocationsPerPixelNV;

const vec3 g_Colors[] = {
	vec3( 1.0, 0.0, 0.0 ),	// 0
	vec3( 0.5, 0.0, 0.0 ),	// 1
	vec3( 0.0 ),
	vec3( 0.0, 1.0, 0.0 ),	// 3
	vec3( 0.0, 0.5, 0.0 ),	// 4
	vec3( 0.0, 0.0, 1.0 ),	// 5
	vec3( 0.0 ),
	vec3( 0.0, 0.0, 0.5 ),	// 7
	vec3( 0.0 ),
	vec3( 0.0 ),
	vec3( 0.0 ),
	vec3( 0.0 ),
	vec3( 1.0, 1.0, 0.0 ),	// 12
	vec3( 0.5, 1.0, 0.0 ),	// 13
	vec3( 0.0 ),
	vec3( 0.0, 1.0, 1.0 )	// 15
};

void main ()
{
	int	index = (gl_FragmentSizeNV.x-1) + (gl_FragmentSizeNV.y-1) * 4;	// 0, 1, 3, 4, 5, 7, 12, 13, 15

	out_Color = vec4( g_Colors[ clamp(index, 0, 16) ], 1.0f );
}
)#"
ARGS_IF_RHIDEBUG("Test_Draw_PS"));

    const FDeviceProperties& properties = fg.DeviceProperties();

    const EShadingRatePalette shadingRatePalette[] = {
        EShadingRatePalette::Block_1x1_2,
        EShadingRatePalette::Block_2x1_1,
        EShadingRatePalette::Block_2x2_1,
        EShadingRatePalette::Block_2x4_1,
        EShadingRatePalette::Block_4x4_1
    };
    LOG_CHECK(WindowTest, lengthof(shadingRatePalette) < properties.ShadingRatePaletteSize);

    TScopedResource<FGPipelineID> ppln{ fg.ScopedResource(fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Test_ShadingRate1"))) };
    LOG_CHECK(WindowTest, ppln.Valid());

    const uint2 viewSize{ 256, 256 };
    const uint2 shadingRateSize = (viewSize / properties.ShadingRateTexelSize);

    TScopedResource<FImageID> image{ fg.ScopedResource(
        fg.CreateImage(FImageDesc{}
            .SetDimension(viewSize)
            .SetFormat(EPixelFormat::RGBA8_UNorm)
            .SetUsage(EImageUsage::ColorAttachment | EImageUsage::TransferSrc),
            Default ARGS_IF_RHIDEBUG("RenderTarget"))) };
    LOG_CHECK(WindowTest, !!image);

    TScopedResource<FImageID> shadingRateImage{ fg.ScopedResource(
        fg.CreateImage(FImageDesc{}
            .SetDimension(shadingRateSize)
            .SetFormat(EPixelFormat::R8u)
            .SetUsage(EImageUsage::ShadingRate | EImageUsage::TransferDst),
            Default ARGS_IF_RHIDEBUG("ShadingRate"))) };
    LOG_CHECK(WindowTest, !!shadingRateImage);

    STACKLOCAL_POD_ARRAY(u8, shadingRateData, static_cast<size_t>(shadingRateSize.x) * shadingRateSize.y);
    forrange(y, 0, shadingRateSize.y) {
        forrange(x, 0, shadingRateSize.x) {
            shadingRateData[x + y * shadingRateSize.x] = checked_cast<u8>(
                (x / 2) % lengthof(shadingRatePalette));
        }
    }

    bool dataIsCorrect = false;
    const auto onLoaded = [&](const FImageView& imageData) {
        const FRgba32f colors[] = {
            FRgba32f{ 1.0f, 0.0f, 0.0f, 1.0f },
            FRgba32f{ 0.5f, 0.0f, 0.0f, 1.0f },
            FRgba32f{ 0.0f, 0.0f, 1.0f, 1.0f },
            FRgba32f{ 0.5f, 1.0f, 0.0f, 1.0f },
            FRgba32f{ 0.0f, 1.0f, 1.0f, 1.0f }
        };

        dataIsCorrect = true;

        forrange(y, 0, imageData.Dimensions().y) {
            forrange(x, 0, imageData.Dimensions().x) {
                FRgba32f rhs;
                imageData.Load(&rhs, uint3(x, y, 0));

                const u32 i = (
                    (x / properties.ShadingRateTexelSize.x) +
                    (y / properties.ShadingRateTexelSize.y) *
                    (imageData.Dimensions().x / properties.ShadingRateTexelSize.x));

                const FRgba32f lhs = colors[shadingRateData[i]];

                const bool isEqual = DistanceSq(lhs, rhs) < F_LargeEpsilon;
                //LOG(WindowTest, Debug, L"Read({0}) -> {1} vs {2} == {3}", float2(x, y), lhs, rhs, isEqual);
                LOG_CHECKVOID(WindowTest, isEqual);
                Assert(isEqual);

                dataIsCorrect &= isEqual;
            }
        }
    };

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("Test_ShadingRate1")
        .SetDebugFlags(EDebugFlags::Default)) };
    LOG_CHECK(WindowTest, !!cmd);

    FLogicalPassID renderPass = cmd->CreateRenderPass(FRenderPassDesc{ viewSize }
        .AddTarget(ERenderTargetID::Color0, image, FLinearColor::Transparent(), EAttachmentStoreOp::Store)
        .AddViewport(viewSize, 0.f, 1.f, shadingRatePalette)
        .SetShadingRateImage(shadingRateImage) );
    LOG_CHECK(WindowTest, !!renderPass);

    cmd->Task(renderPass, FDrawVertices{}
        .Draw(3)
        .SetPipeline(ppln)
        .SetTopology(EPrimitiveTopology::TriangleList));

    const PFrameTask tUpdate = cmd->Task(FUpdateImage{}.SetImage(shadingRateImage).SetData(shadingRateData, shadingRateSize, shadingRateSize.x));
    const PFrameTask tDraw = cmd->Task(FSubmitRenderPass{ renderPass }.DependsOn(tUpdate));
    const PFrameTask tRead = cmd->Task(FReadImage{}.SetImage(image, int2::Zero, viewSize).SetCallback(onLoaded).DependsOn(tDraw));
    UNUSED(tRead);

    LOG_CHECK(WindowTest, fg.Execute(cmd));
    LOG_CHECK(WindowTest, fg.WaitIdle());

    LOG_CHECK(WindowTest, !!dataIsCorrect);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
