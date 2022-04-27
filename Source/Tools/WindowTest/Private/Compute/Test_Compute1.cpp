#include "stdafx.h"

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Test_Compute1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FComputePipelineDesc desc;
    desc.AddShader(EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(compute)
#extension GL_ARB_shading_language_420pack : enable

layout (local_size_x_id = 0, local_size_y_id = 1, local_size_z = 1) in;
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(binding=0, rgba8) writeonly uniform image2D  un_OutImage;

void main ()
{
	vec4 fragColor = vec4(float(gl_LocalInvocationID.x) / float(gl_WorkGroupSize.x),
						  float(gl_LocalInvocationID.y) / float(gl_WorkGroupSize.y),
						  1.0, 0.0);

	imageStore( un_OutImage, ivec2(gl_GlobalInvocationID.xy), fragColor );
}
)#"
ARGS_IF_RHIDEBUG("Test_Compute1_CS"));

    const uint2 imageDim{ 16, 16 };

    TAutoResource<FImageID> image0{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(imageDim)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Storage | EImageUsage::TransferSrc),
        Default ARGS_IF_RHIDEBUG("Image0")) };
    LOG_CHECK(WindowTest, image0.Valid());

    TAutoResource<FImageID> image1{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(imageDim)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Storage | EImageUsage::TransferSrc),
        Default ARGS_IF_RHIDEBUG("Image1")) };
    LOG_CHECK(WindowTest, image1.Valid());

    TAutoResource<FImageID> image2{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(imageDim)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Storage | EImageUsage::TransferSrc),
        Default ARGS_IF_RHIDEBUG("Image2")) };
    LOG_CHECK(WindowTest, image2.Valid());

    TAutoResource<FCPipelineID> ppln{ fg, fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Test_Compute1")) };
    LOG_CHECK(WindowTest, ppln.Valid());

    PPipelineResources resources = NEW_REF(RHIPipeline, FPipelineResources);
    LOG_CHECK(WindowTest, fg.InitPipelineResources(resources.get(), ppln, FDescriptorSetID{ "0" }));

    const auto checkImageData = [](const FImageView& imageData, u32 blockSize) -> bool {
        bool dataIsCorrect = true;

        forrange(y, 0, imageData.Dimensions().y) {
            forrange(x, 0, imageData.Dimensions().x) {
                FRgba32u texel;
                imageData.Load(&texel, uint3(x,y,0));

                const u32 r = Float01_to_UByte0255(static_cast<float>(x % blockSize) / blockSize);
                const u32 g = Float01_to_UByte0255(static_cast<float>(y % blockSize) / blockSize);

                const bool isEqual = (
                    Abs(static_cast<int>(texel.x) - static_cast<int>(r)) <= 1 &&
                    Abs(static_cast<int>(texel.y) - static_cast<int>(g)) <= 1 &&
                    texel.z == 255 &&
                    texel.w == 0 );

                //LOG(WindowTest, Debug, L"Read({0}) -> {1} vs {2} == {3}", uint2(x, y), texel, FRgba32u(r, g, 0, 255), isEqual);
                LOG_CHECK(WindowTest, isEqual);
                dataIsCorrect &= isEqual;
            }

        }

        return dataIsCorrect;
    };

    bool dataIsCorrect0 = false;
    bool dataIsCorrect1 = false;
    bool dataIsCorrect2 = false;

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("Test_Compute1")
        .SetDebugFlags(EDebugFlags::Default)) };
    LOG_CHECK(WindowTest, !!cmd);

    resources->BindImage(FUniformID{ "un_OutImage" }, image0);
    PFrameTask tRun0 = cmd->Task(FDispatchCompute{}
        .SetPipeline(ppln)
        .AddResources(FDescriptorSetID{ "0" }, resources)
        .Dispatch({ 2, 2 }));
    LOG_CHECK(WindowTest, tRun0);

    resources->BindImage(FUniformID{ "un_OutImage" }, image1);
    PFrameTask tRun1 = cmd->Task(FDispatchCompute{}
        .SetPipeline(ppln)
        .AddResources(FDescriptorSetID{ "0" }, resources)
        .Dispatch({ 4, 4 })
        .SetLocalSize({ 4, 4 }));
    LOG_CHECK(WindowTest, tRun1);

    resources->BindImage(FUniformID{ "un_OutImage" }, image2);
    PFrameTask tRun2 = cmd->Task(FDispatchCompute{}
        .SetPipeline(ppln)
        .AddResources(FDescriptorSetID{ "0" }, resources)
        .Dispatch({ 2, 2 })
        .SetLocalSize({ 8, 8 }));
    LOG_CHECK(WindowTest, tRun2);

    PFrameTask tRead0 = cmd->Task(FReadImage{}
        .SetImage(image0, int2(0), imageDim)
        .DependsOn(tRun0)
        .SetCallback([&](const FImageView& imageData) {
            dataIsCorrect0 = checkImageData(imageData, 8);
        }));
    LOG_CHECK(WindowTest, tRead0);
    Unused(tRead0);

    PFrameTask tRead1 = cmd->Task(FReadImage{}
        .SetImage(image1, int2(0), imageDim)
        .DependsOn(tRun1)
        .SetCallback([&](const FImageView& imageData) {
            dataIsCorrect1 = checkImageData(imageData, 4);
        }));
    LOG_CHECK(WindowTest, tRead1);
    Unused(tRead1);

    PFrameTask tRead2 = cmd->Task(FReadImage{}
        .SetImage(image2, int2(0), imageDim)
        .DependsOn(tRun2)
        .SetCallback([&](const FImageView& imageData) {
            dataIsCorrect2 = checkImageData(imageData, 8);
        }));
    LOG_CHECK(WindowTest, tRead2);
    Unused(tRead2);

    LOG_CHECK(WindowTest, fg.Execute(cmd));
    LOG_CHECK(WindowTest, fg.WaitIdle());

    LOG_CHECK(WindowTest, dataIsCorrect0);
    LOG_CHECK(WindowTest, dataIsCorrect1);
    LOG_CHECK(WindowTest, dataIsCorrect2);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
