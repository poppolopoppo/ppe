﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compute_InvalidID_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FComputePipelineDesc desc;
    desc.AddShader(EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(compute)
#extension GL_ARB_shading_language_420pack : enable

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
ARGS_IF_RHIDEBUG("Compute_InvalidID_CS"));

    const uint2 imageDim{ 16, 16 };

    TAutoResource<FImageID> image0{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(imageDim)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Storage | EImageUsage_BlitTransferSrc),
        Default ARGS_IF_RHIDEBUG("Image0")) };
    PPE_LOG_CHECK(WindowTest, image0.Valid());

    TAutoResource<FImageID> image1{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(imageDim)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Storage | EImageUsage_BlitTransferSrc),
        Default ARGS_IF_RHIDEBUG("Image1")) };
    PPE_LOG_CHECK(WindowTest, image1.Valid());

    FImageID image2{ fg.AcquireResource(image0->Get()) };
    PPE_LOG_CHECK(WindowTest, image2.Valid());

    // some invalid IDs:
    FImageID image3{ FRawImageID(1111, 0) };
    FImageID image4{ FRawImageID(2222, 0) };

    TAutoResource<FCPipelineID> ppln{ fg, fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Compute_InvalidID")) };
    PPE_LOG_CHECK(WindowTest, ppln.Valid());

    PPipelineResources resources = NEW_REF(RHIPipeline, FPipelineResources);
    PPE_LOG_CHECK(WindowTest, fg.InitPipelineResources(resources.get(), ppln, FDescriptorSetID{ "0" }));

    const FImageDesc& invalidDesc = fg.Description(*image3);
    Unused(invalidDesc);
    PPE_LOG(WindowTest, Info, "invalid image dimension: {0}", invalidDesc.Dimensions);

    // Frame 1
    FCommandBufferBatch cmd1{ fg.Begin(FCommandBufferDesc{}
        .SetName("Compute_InvalidID_Frame1")) };
    PPE_LOG_CHECK(WindowTest, !!cmd1);
    {
        resources->BindImage(FUniformID{ "un_OutImage" }, image0);

        PFrameTask tRun = cmd1->Task(FDispatchCompute{}
            .SetPipeline(ppln)
            .AddResources(FDescriptorSetID{ "0" }, resources));
        PPE_LOG_CHECK(WindowTest, tRun);
        Unused(tRun);

        {
            FExpectAssertInScope expectAssertOnInvalidID;
            PFrameTask tCopy = cmd1->Task(FCopyImage{}
                .From(*image2).To(*image4) // should assert, but not crash
                .AddRegion(Default, int2::Zero, Default, int2::Zero, imageDim));
            PPE_LOG_CHECK(WindowTest, tCopy);
            Unused(tCopy);
            Unused(expectAssertOnInvalidID);
        }

        PPE_LOG_CHECK(WindowTest, fg.Execute(cmd1));
    }

    Unused(fg.ReleaseResource(image2));
    //fg.ReleaseResource(image3); // #TODO

    // Frame 2
    FCommandBufferBatch cmd2{ fg.Begin(FCommandBufferDesc{}
        .SetName("Compute_InvalidID_Frame2")) };
    PPE_LOG_CHECK(WindowTest, !!cmd2);
    {
        PFrameTask tRun = cmd2->Task(FDispatchCompute{}
            .SetPipeline(ppln)
            .AddResources(FDescriptorSetID{ "0" }, resources));
        PPE_LOG_CHECK(WindowTest, tRun);

        PFrameTask tCopy = cmd2->Task(FCopyImage{}
            .From(image0).To(image1)
            .AddRegion(Default, int2::Zero, Default, int2::Zero, imageDim));
        PPE_LOG_CHECK(WindowTest, tCopy);

        Unused(tRun);
        Unused(tCopy);

        PPE_LOG_CHECK(WindowTest, fg.Execute(cmd2));
    }

    PPE_LOG_CHECK(WindowTest, fg.WaitIdle());

    Unused(image3.Release());
    Unused(image4.Release());

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
