#include "stdafx.h"

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Test_DynamicOffset1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FComputePipelineDesc desc;
    desc.AddShader(EShaderLangFormat::VKSL_100, "main", R"#(
#extension GL_ARB_shading_language_420pack : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// @dynamic-offset
layout (binding=0, std140) uniform UB {
	vec4	data[4];
} ub;

// @dynamic-offset
layout (binding=1, std430) writeonly buffer SSB {
	vec4	data[4];
} ssb;

void main ()
{
	ssb.data[0] = ub.data[1];
	ssb.data[3] = ub.data[2];
	ssb.data[1] = ub.data[3];
	ssb.data[2] = ub.data[0];
}
)#"
ARGS_IF_RHIDEBUG("Test_DynamicOffset1_CS"));

    TScopedResource<FCPipelineID> ppln{ fg, fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Test_DynamicOffset1")) };
    LOG_CHECK(WindowTest, ppln.Valid());

    const FDeviceProperties properties = fg.DeviceProperties();

    const float srcData[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        1.1f, 2.2f, 2.3f, 2.4f,
        1.5f, 1.6f, 1.7f, 1.8f };

    const size_t sbAlign = properties.MinStorageBufferOffsetAlignment;
    const size_t ubAlign = properties.MinUniformBufferOffsetAlignment;
    const size_t baseOffset = Max(sbAlign, ubAlign);
    const size_t bufferOffset = baseOffset;
    const size_t srcSize = sizeof(float4) * 4;
    const size_t dstSize = sizeof(float4) * 4;
    const size_t ubOffset = Meta::RoundToNext(baseOffset + bufferOffset, ubAlign);
    const size_t sbOffset = Meta::RoundToNext(baseOffset + bufferOffset + srcSize, ubAlign);

    const TScopedResource<FBufferID> buffer = fg.ScopedResource(fg.CreateBuffer(
        FBufferDesc{
            Max(ubOffset + srcSize, sbOffset + dstSize),
            EBufferUsage::Uniform |
            EBufferUsage::Storage |
            EBufferUsage::Transfer },
        Default ARGS_IF_RHIDEBUG("SharedBuffer")
    ));
    LOG_CHECK(WindowTest, !!buffer);

    PPipelineResources resources = NEW_REF(RHIPipeline, FPipelineResources);
    LOG_CHECK(WindowTest, fg.InitPipelineResources(resources.get(), ppln, FDescriptorSetID{ "0" }));

    bool callbackWasCalled = false;
    bool dataIsCorrect = false;

    const auto onLoaded = [&](const FBufferView& bufferData) {
        callbackWasCalled = true;
        dataIsCorrect = (bufferData.size() == dstSize);

        const TMemoryView<const float> dstData = bufferData.Parts().front().Cast<const float>();

        dataIsCorrect &= (srcData[1 * 4 + 0] == dstData[0 * 4 + 0]);
        dataIsCorrect &= (srcData[1 * 4 + 1] == dstData[0 * 4 + 1]);
        dataIsCorrect &= (srcData[1 * 4 + 2] == dstData[0 * 4 + 2]);
        dataIsCorrect &= (srcData[1 * 4 + 3] == dstData[0 * 4 + 3]);
        dataIsCorrect &= (srcData[2 * 4 + 0] == dstData[3 * 4 + 0]);
        dataIsCorrect &= (srcData[2 * 4 + 1] == dstData[3 * 4 + 1]);
        dataIsCorrect &= (srcData[2 * 4 + 2] == dstData[3 * 4 + 2]);
        dataIsCorrect &= (srcData[2 * 4 + 3] == dstData[3 * 4 + 3]);
        dataIsCorrect &= (srcData[3 * 4 + 0] == dstData[1 * 4 + 0]);
        dataIsCorrect &= (srcData[3 * 4 + 1] == dstData[1 * 4 + 1]);
        dataIsCorrect &= (srcData[3 * 4 + 2] == dstData[1 * 4 + 2]);
        dataIsCorrect &= (srcData[3 * 4 + 3] == dstData[1 * 4 + 3]);
        dataIsCorrect &= (srcData[0 * 4 + 0] == dstData[2 * 4 + 0]);
        dataIsCorrect &= (srcData[0 * 4 + 1] == dstData[2 * 4 + 1]);
        dataIsCorrect &= (srcData[0 * 4 + 2] == dstData[2 * 4 + 2]);
        dataIsCorrect &= (srcData[0 * 4 + 3] == dstData[2 * 4 + 3]);
    };

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("Test_DynamicOffset1")
        .SetDebugFlags(EDebugFlags::Default)) };
    LOG_CHECK(WindowTest, !!cmd);

    resources->SetBufferBase(FUniformID{ "UB" }, 0_b);
    resources->SetBufferBase(FUniformID{ "SSB" }, baseOffset);

    resources->BindBuffer(FUniformID{ "UB" }, buffer, ubOffset, srcSize);
    resources->BindBuffer(FUniformID{ "SSB" }, buffer, sbOffset, dstSize);

    PFrameTask tWrite = cmd->Task(FUpdateBuffer{}
        .SetBuffer(buffer)
        .AddData(MakeView(srcData), ubOffset));
    LOG_CHECK(WindowTest, !!tWrite);

    PFrameTask tDispatch = cmd->Task(FDispatchCompute{}
        .SetPipeline(ppln)
        .AddResources(FDescriptorSetID{ "0" }, resources)
        .Dispatch({1, 1})
        .DependsOn(tWrite));
    LOG_CHECK(WindowTest, !!tDispatch);

    PFrameTask tRead = cmd->Task(FReadBuffer{}
        .SetBuffer(buffer, sbOffset, dstSize)
        .SetCallback(onLoaded)
        .DependsOn(tDispatch));
    LOG_CHECK(WindowTest, !!tRead);
    UNUSED(tRead);

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
