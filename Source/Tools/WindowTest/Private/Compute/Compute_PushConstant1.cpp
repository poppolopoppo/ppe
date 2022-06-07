#include "stdafx.h"

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compute_PushConstant1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FComputePipelineDesc desc;
    desc.AddShader(EShaderLangFormat::VKSL_100, "main", R"#(
#extension GL_ARB_shading_language_420pack : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (push_constant, std140) uniform MyPushConstant {
						vec3	f3;
						ivec2	i2;
	layout(offset = 32) float	f1;
} pc;

layout (binding=0, std430) writeonly buffer SSB {
	vec4	data[2];
} ssb;

void main ()
{
	ssb.data[0] = vec4(pc.f3.x, pc.f3.y, pc.f3.z, pc.f1);
	ssb.data[1] = vec4(float(pc.i2.x), float(pc.i2.y), 0.0f, 1.0f);
}
)#"
ARGS_IF_RHIDEBUG("Compute_PushConstant1_CS"));


    struct FPushConstant_ {
        float3 f3;          // offset:   0
        float _padding1;    // offset:  12
        int2 i2;            // offset:  16
        float2 _padding2;   // offset:  24
        float f1;           // offset:  32
                            // size  :  36
    };

    const size_t dstBufferSize{ 32_b };

    const TAutoResource<FBufferID> dstBuffer{ fg, fg.CreateBuffer(FBufferDesc{
        dstBufferSize,
        EBufferUsage::Storage | EBufferUsage::TransferSrc },
        Default ARGS_IF_RHIDEBUG("DstBuffer")) };
    LOG_CHECK(WindowTest, dstBuffer.Valid());

    const TAutoResource<FCPipelineID> ppln{ fg, fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Compute_PushConstant1")) };
    LOG_CHECK(WindowTest, ppln.Valid());

    FPushConstant_ pc;
    pc.f3 = { 10.1f, 11.2f, 18.5f };
    pc.i2 = { 11, 22 };
    pc.f1 = 33.0f;

    PPipelineResources resources = NEW_REF(RHIPipeline, FPipelineResources);
    LOG_CHECK(WindowTest, fg.InitPipelineResources(resources.get(), ppln, FDescriptorSetID{ "0" }));

    bool callbackWasCalled = false;
    bool dataIsCorrect = false;

    const auto onLoaded = [&](const FBufferView& bufferData) {
        callbackWasCalled = true;
        dataIsCorrect = (bufferData.size() == dstBufferSize);
        AssertRelease(bufferData.Parts().size() == 1);

        const TMemoryView<const float> dstData = bufferData.Parts().front().Cast<const float>();

        dataIsCorrect &= (dstData[0] == pc.f3.x);
        dataIsCorrect &= (dstData[1] == pc.f3.y);
        dataIsCorrect &= (dstData[2] == pc.f3.z);
        dataIsCorrect &= (dstData[3] == pc.f1);
        dataIsCorrect &= (dstData[4] == float(pc.i2.x));
        dataIsCorrect &= (dstData[5] == float(pc.i2.y));
        dataIsCorrect &= (dstData[6] == 0.0f);
        dataIsCorrect &= (dstData[7] == 1.0f);
    };

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("Compute_PushConstant1")
        .SetDebugFlags(EDebugFlags::Default)) };
    LOG_CHECK(WindowTest, !!cmd);

    resources->BindBuffer(FUniformID{ "SSB" }, dstBuffer);

    PFrameTask tDispatch = cmd->Task(FDispatchCompute{}
        .SetPipeline(ppln)
        .Dispatch({ 1, 1 })
        .AddPushConstant(FPushConstantID{ "MyPushConstant" }, pc)
        .AddResources(FDescriptorSetID{ "0" }, resources));
    LOG_CHECK(WindowTest, tDispatch);

    PFrameTask tRead = cmd->Task(FReadBuffer{}
        .SetBuffer(dstBuffer, 0_b, dstBufferSize)
        .DependsOn(tDispatch)
        .SetCallback(onLoaded));
    LOG_CHECK(WindowTest, tRead);
    Unused(tRead);

    LOG_CHECK(WindowTest, fg.Execute(cmd));

    LOG_CHECK(WindowTest, not callbackWasCalled);

    LOG_CHECK(WindowTest, fg.WaitIdle());

    LOG_CHECK(WindowTest, callbackWasCalled);
    LOG_CHECK(WindowTest, dataIsCorrect);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
