// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compute_Compute2_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FComputePipelineDesc desc;
    desc.AddShader(EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(compute)
#extension GL_ARB_shading_language_420pack : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set=2, binding=0, rgba8) writeonly uniform image2D  un_OutImage;

void main ()
{
	vec4 fragColor = vec4(float(gl_LocalInvocationID.x) / float(gl_WorkGroupSize.x),
						  float(gl_LocalInvocationID.y) / float(gl_WorkGroupSize.y),
						  1.0, 0.0);

	imageStore( un_OutImage, ivec2(gl_GlobalInvocationID.xy), fragColor );
}
)#"
ARGS_IF_RHIDEBUG("Compute_Compute2_CS"));

    const uint2 imageDim{ 16, 16 };

    TAutoResource<FImageID> image{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(imageDim)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Storage | EImageUsage::TransferSrc),
        Default ARGS_IF_RHIDEBUG("Image")) };
    LOG_CHECK(WindowTest, image.Valid());

    TAutoResource<FCPipelineID> ppln{ fg, fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Compute_Compute2")) };
    LOG_CHECK(WindowTest, ppln.Valid());

    PPipelineResources resources = NEW_REF(RHIPipeline, FPipelineResources);
    LOG_CHECK(WindowTest, fg.InitPipelineResources(resources.get(), ppln, FDescriptorSetID{ "2" }));

    bool dataIsCorrect = false;
    const auto onLoaded = [&dataIsCorrect](const FImageView& imageData) {
        dataIsCorrect = true;

        forrange(y, 0, imageData.Dimensions().y) {
            forrange(x, 0, imageData.Dimensions().x) {
                FRgba32u texel;
                imageData.Load(&texel, uint3(x,y,0));

                constexpr u32 blockSize = 8;
                const u32 r = Float01_to_UByte0255(static_cast<float>(x % blockSize) / blockSize);
                const u32 g = Float01_to_UByte0255(static_cast<float>(y % blockSize) / blockSize);

                const bool isEqual = (
                    Abs(static_cast<int>(texel.x) - static_cast<int>(r)) <= 1 &&
                    Abs(static_cast<int>(texel.y) - static_cast<int>(g)) <= 1 &&
                    texel.z == 255 &&
                    texel.w == 0 );

                //LOG(WindowTest, Debug, L"Read({0}) -> {1} vs {2} == {3}", uint2(x, y), texel, FRgba32u(r, g, 0, 255), isEqual);
                LOG_CHECKVOID(WindowTest, isEqual);
                dataIsCorrect &= isEqual;
            }
        }
    };

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("Compute_Compute2")
        .SetDebugFlags(EDebugFlags::Default)) };
    LOG_CHECK(WindowTest, !!cmd);

    resources->BindImage(FUniformID{ "un_OutImage" }, image);

    PFrameTask tRun = cmd->Task(FDispatchCompute{}
        .SetPipeline(ppln)
        .AddResources(FDescriptorSetID{ "2" }, resources)
        .Dispatch({ 2, 2 }));
    LOG_CHECK(WindowTest, tRun);

    PFrameTask tRead = cmd->Task(FReadImage{}
        .SetImage(image, int2(0), imageDim)
        .DependsOn(tRun)
        .SetCallback(onLoaded));
    LOG_CHECK(WindowTest, tRead);
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
