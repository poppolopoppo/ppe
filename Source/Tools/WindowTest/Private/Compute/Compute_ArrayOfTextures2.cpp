#include "stdafx.h"

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compute_ArrayOfTextures2_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FComputePipelineDesc desc;
    desc.AddShader(EShaderLangFormat::VKSL_100, "main", R"#(
#version 460 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_nonuniform_qualifier : require

layout (local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

layout(binding=0) uniform sampler2D  un_Textures[];

layout(binding=1) writeonly uniform image2D  un_OutImage;

void main ()
{
	const int	i		= int(gl_LocalInvocationIndex);
	const vec2	coord	= vec2(gl_GlobalInvocationID.xy) / vec2(gl_WorkGroupSize.xy * gl_NumWorkGroups.xy - 1);
		  vec4	color	= texture( un_Textures[nonuniformEXT(i)], coord );

	imageStore( un_OutImage, ivec2(gl_GlobalInvocationID.xy), color );
}
)#"
ARGS_IF_RHIDEBUG("Compute_ArrayOfTextures2_CS"));

    const uint2 imageDim{ 32, 32 };
    const uint2 textureDim{ 16, 16 };

    FImageID textures[8];
    forrange(i, 0, lengthof(textures)) {
        textures[i] = fg.CreateImage(FImageDesc{}
            .SetDimension(textureDim)
            .SetFormat(EPixelFormat::RGBA8_UNorm)
            .SetUsage(EImageUsage::Sampled | EImageUsage::TransferDst),
            Default ARGS_IF_RHIDEBUG(INLINE_FORMAT(16, "Texture-{0}", i)) );
        LOG_CHECK(WindowTest, textures[i].Valid());
    }


    ON_SCOPE_EXIT([&]() {
        forrange(i, 0, lengthof(textures)) {
            LOG_CHECKVOID(WindowTest, fg.ReleaseResource(textures[i]));
        }
    });

    TAutoResource<FImageID> imageDst{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(imageDim)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Storage | EImageUsage::TransferSrc),
        Default ARGS_IF_RHIDEBUG("Output")) };
    LOG_CHECK(WindowTest, imageDst.Valid());

    TAutoResource<FSamplerID> sampler{ fg, fg.CreateSampler(FSamplerDesc{} ARGS_IF_RHIDEBUG("Sampler_ArrayOfTextures2")) };
    LOG_CHECK(WindowTest, sampler.Valid());

    TAutoResource<FCPipelineID> ppln{ fg, fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Compute_ArrayOfTextures2")) };
    LOG_CHECK(WindowTest, ppln.Valid());

    PPipelineResources resources = NEW_REF(RHIPipeline, FPipelineResources);
    LOG_CHECK(WindowTest, fg.InitPipelineResources(resources.get(), ppln, FDescriptorSetID{ "0" }));

    bool dataIsCorrect = false;
    const auto onLoaded = [&dataIsCorrect](const FImageView& imageData) {
        const auto testPixel = [&imageData](u32 x, u32 y, const FRgba32f& color) -> bool
        {
            FRgba32f texel;
            imageData.Load(&texel, uint3{ x, y, 0 });

            const bool isEqual = DistanceSq(color, texel) < LargeEpsilon;
            LOG(WindowTest, Debug, L"Read({0}) -> {1} vs {2} == {3}", uint2(x, y), texel, color, isEqual);
            LOG_CHECK(WindowTest, isEqual);
            Assert(isEqual);
            return isEqual;
        };

        dataIsCorrect = true;

        dataIsCorrect &= testPixel( 0,  4, NormUnpack(ubyte4n{ 255_u8,   0_u8,   0_u8, 255_u8 }));
        dataIsCorrect &= testPixel( 1,  5, NormUnpack(ubyte4n{ 255_u8, 191_u8,   0_u8, 255_u8 }));
        dataIsCorrect &= testPixel( 2,  6, NormUnpack(ubyte4n{ 127_u8, 255_u8,   0_u8, 255_u8 }));
        dataIsCorrect &= testPixel( 3,  7, NormUnpack(ubyte4n{   0_u8, 255_u8,  64_u8, 255_u8 }));
        dataIsCorrect &= testPixel( 4,  8, NormUnpack(ubyte4n{   0_u8, 255_u8, 255_u8, 255_u8 }));
        dataIsCorrect &= testPixel( 5,  1, NormUnpack(ubyte4n{   0_u8,  64_u8, 255_u8, 255_u8 }));
        dataIsCorrect &= testPixel( 6,  2, NormUnpack(ubyte4n{ 127_u8,   0_u8, 255_u8, 255_u8 }));
        dataIsCorrect &= testPixel( 7,  3, NormUnpack(ubyte4n{ 255_u8,   0_u8, 191_u8, 255_u8 }));

        dataIsCorrect &= testPixel( 8, 20, NormUnpack(ubyte4n{ 255_u8,   0_u8,   0_u8, 255_u8 }));
        dataIsCorrect &= testPixel( 9, 26, NormUnpack(ubyte4n{ 255_u8, 191_u8,   0_u8, 255_u8 }));
        dataIsCorrect &= testPixel(10, 13, NormUnpack(ubyte4n{ 127_u8, 255_u8,   0_u8, 255_u8 }));
        dataIsCorrect &= testPixel(11,  9, NormUnpack(ubyte4n{   0_u8, 255_u8,  64_u8, 255_u8 }));
        dataIsCorrect &= testPixel(12, 18, NormUnpack(ubyte4n{   0_u8, 255_u8, 255_u8, 255_u8 }));
        dataIsCorrect &= testPixel(13,  7, NormUnpack(ubyte4n{   0_u8,  64_u8, 255_u8, 255_u8 }));
        dataIsCorrect &= testPixel(14,  8, NormUnpack(ubyte4n{ 127_u8,   0_u8, 255_u8, 255_u8 }));
        dataIsCorrect &= testPixel(15, 22, NormUnpack(ubyte4n{ 255_u8,   0_u8, 191_u8, 255_u8 }));
    };

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("Compute_ArrayOfTextures2")
        .SetDebugFlags(EDebugFlags::Default)) };
    LOG_CHECK(WindowTest, !!cmd);

    resources->BindTextures(FUniformID{ "un_Textures" }, textures, sampler);
    resources->BindImage(FUniformID{ "un_OutImage" }, imageDst);

    PFrameTask tUpdate;
    forrange(i, 0, lengthof(textures)) {
        FLinearColor color{ Hue_to_RGB(float(i) / lengthof(textures)), 1.0f };

        tUpdate = cmd->Task(FClearColorImage{}
            .SetImage(*textures[i])
            .AddRange(0_mipmap, 1, 0_layer, 1)
            .Clear(color)
            .DependsOn(tUpdate));
    }

    const PFrameTask tComp = cmd->Task(FDispatchCompute{}
        .SetPipeline(ppln)
        .Dispatch({ imageDim.x / 8, imageDim.y })
        .AddResources(FDescriptorSetID{ "0" }, resources)
        .DependsOn(tUpdate));

    const PFrameTask tRead = cmd->Task(FReadImage{}
        .SetImage(imageDst, int2{}, imageDim)
        .SetCallback(onLoaded)
        .DependsOn(tComp));
    Unused(tRead);

    LOG_CHECK(WindowTest, fg.Execute(cmd));
    LOG_CHECK(WindowTest, fg.WaitIdle());

    LOG_CHECK(WindowTest, dataIsCorrect);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
