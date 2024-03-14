// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Debugger_ShaderDebugger1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    if (not EnableShaderDebugging) {
        Unused(app);
        PPE_LOG(WindowTest, Warning, "Debugger_ShaderDebugger1_: skipped due to lack of debugger support (USE_PPE_RHIDEBUG={0})", USE_PPE_RHIDEBUG);
        return true;
    }

#if USE_PPE_RHIDEBUG
    IFrameGraph& fg = *app.RHI().FrameGraph();

    FComputePipelineDesc desc;
    desc.AddShader(EShaderLangFormat::VKSL_100 | EShaderLangFormat::EnableDebugTrace, "main", R"#(
#pragma shader_stage(compute)
#extension GL_ARB_shading_language_420pack : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(binding=0, r32f) writeonly uniform image2D  un_OutImage;

void main ()
{
	uint	index	= gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
	uint	size	= gl_NumWorkGroups.x * gl_NumWorkGroups.y * gl_WorkGroupSize.x * gl_WorkGroupSize.y;
	float	value	= fract( float(index) / size );
	imageStore( un_OutImage, ivec2(gl_GlobalInvocationID.xy), vec4(value) );
}
)#"
ARGS_IF_RHIDEBUG("Debugger_ShaderDebugger1_CS"));

    const uint2 imageDim{ 16, 16 };
    const uint2 debugCoord{ imageDim / 2_u32 };

    TAutoResource<FImageID> imageDst{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(imageDim)
        .SetFormat(EPixelFormat::R32f)
        .SetUsage(EImageUsage::Storage | EImageUsage_BlitTransferSrc),
        Default ARGS_IF_RHIDEBUG("Output")) };
    PPE_LOG_CHECK(WindowTest, imageDst.Valid());

    TAutoResource<FCPipelineID> ppln{ fg, fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Debugger_ShaderDebugger1")) };
    PPE_LOG_CHECK(WindowTest, ppln.Valid());

    PPipelineResources resources = NEW_REF(RHIPipeline, FPipelineResources);
    PPE_LOG_CHECK(WindowTest, fg.InitPipelineResources(resources.get(), ppln, FDescriptorSetID{ "0" }));

    bool dataIsCorrect = false;
    bool shaderOutputIsCorrect = false;

    const auto onShaderTraceReady = [&shaderOutputIsCorrect](FStringView taskName, FStringView shaderName, EShaderStages stages, TMemoryView<const FString> output) {
        const FStringView ref{ R"#(//> gl_GlobalInvocationID: uint3 {8, 8, 0}
//> gl_LocalInvocationID: uint3 {0, 0, 0}
//> gl_WorkGroupID: uint3 {1, 1, 0}
no source

//> index: uint {136}
//  gl_GlobalInvocationID: uint3 {8, 8, 0}
11. index	= gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;

//> size: uint {256}
12. size	= gl_NumWorkGroups.x * gl_NumWorkGroups.y * gl_WorkGroupSize.x * gl_WorkGroupSize.y;

//> value: float {0.531250}
//  index: uint {136}
//  size: uint {256}
13. value	= fract( float(index) / size );

//> imageStore(): void
//  value: float {0.531250}
//  gl_GlobalInvocationID: uint3 {8, 8, 0}
14. 	imageStore( un_OutImage, ivec2(gl_GlobalInvocationID.xy), vec4(value) );

)#" };

        shaderOutputIsCorrect = true;

        shaderOutputIsCorrect &= (stages == EShaderStages::Compute);
        shaderOutputIsCorrect &= (taskName == "DebuggableCompute");
        shaderOutputIsCorrect &= (shaderName == "Debugger_ShaderDebugger1_CS");
        shaderOutputIsCorrect &= (output.size() == 1);
        shaderOutputIsCorrect &= (output.size() == 1 ? output[0] == ref : false);

        PPE_LOG_CHECKVOID(WindowTest, shaderOutputIsCorrect);
    };

    fg.SetShaderDebugCallback(onShaderTraceReady);

    const auto onLoaded = [&dataIsCorrect, &debugCoord](const FImageView& imageData) {
        FRgba32f texel;
        imageData.Load(&texel, uint3(debugCoord, 0));
        dataIsCorrect = (texel.x == 0.53125f);
        PPE_LOG_CHECKVOID(WindowTest, dataIsCorrect);
    };

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("Debugger_ShaderDebugger1")
        .SetDebugFlags(EDebugFlags_Default)) };
    PPE_LOG_CHECK(WindowTest, !!cmd);

    resources->BindImage(FUniformID{ "un_OutImage" }, imageDst);

    const PFrameTask tComp = cmd->Task(FDispatchCompute{}
        .SetPipeline(ppln)
        .AddResources(FDescriptorSetID{ "0" }, resources)
        .Dispatch({ 2, 2 })
        .SetName("DebuggableCompute")
        .EnableShaderDebugTrace(uint3(debugCoord, 0)) );

    const PFrameTask tRead = cmd->Task(FReadImage{}
        .SetImage(imageDst, int2{}, imageDim)
        .SetCallback(onLoaded)
        .DependsOn(tComp));
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
