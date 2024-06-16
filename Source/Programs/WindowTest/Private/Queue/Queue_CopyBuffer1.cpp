// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Queue_CopyBuffer1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    const size_t srcBufferSize = 256;
    const size_t dstBufferSize = 512;

    auto srcBuffer = fg.ScopedResource(fg.CreateBuffer(FBufferDesc{ srcBufferSize, EBufferUsage_Transfer }, Default ARGS_IF_RHIDEBUG("SrcBuffer")));
    PPE_LOG_CHECK(WindowTest, srcBuffer.Valid());

    auto dstBuffer = fg.ScopedResource(fg.CreateBuffer(FBufferDesc{ dstBufferSize, EBufferUsage_Transfer }, Default ARGS_IF_RHIDEBUG("DstBuffer")));
    PPE_LOG_CHECK(WindowTest, dstBuffer.Valid());

    RAWSTORAGE(UnitTest, u8) srcData;
    srcData.Resize_DiscardData(srcBufferSize);
    forrange(i, 0, srcBufferSize) {
        srcData[i] = checked_cast<u8>(i);
    }

    bool cbWasCalled = false;
    bool dataIsCorrect = false;

    const auto onLoaded = [&](FBufferView bufferData) {
        cbWasCalled = true;
        dataIsCorrect = (bufferData.size() == dstBufferSize);

        forrange(i, 0, srcBufferSize) {
            const bool isEqual = (srcData[i] == bufferData[i + 128]);
            //PPE_LOG(WindowTest, Debug, "Read({0}) -> {1} vs {2} == {3}", i, srcData[i], bufferData[i+128], isEqual);
            dataIsCorrect &= isEqual;
        }
    };

    auto cmd = fg.Begin(FCommandBufferDesc{}
        .SetName("Queue_CopyBuffer1_")
        .SetDebugFlags(Default));
    PPE_LOG_CHECK(WindowTest, !!cmd);

    const PFrameTask tUpdate = cmd->Task(FUpdateBuffer{}.SetBuffer(srcBuffer).AddData(srcData.MakeView()));
    const PFrameTask tCopy = cmd->Task(FCopyBuffer{}.From(srcBuffer).To(dstBuffer).AddRegion(0,128,256).DependsOn(tUpdate));
    const PFrameTask tRead = cmd->Task(FReadBuffer{}.SetBuffer(dstBuffer, 0, dstBufferSize).SetCallback(onLoaded).DependsOn(tCopy));
    Unused(tRead);

    PPE_LOG_CHECK(WindowTest, fg.Execute(cmd));

    // after execution 'src_data' was copied to 'src_buffer', 'src_buffer' copied to 'dst_buffer', 'dst_buffer' copied to staging buffer...
    PPE_LOG_CHECK(WindowTest, not cbWasCalled);

    // all staging buffers will be synchronized, all 'ReadBuffer' callbacks will be called.
    PPE_LOG_CHECK(WindowTest, fg.WaitIdle());

    PPE_LOG_CHECK(WindowTest, cbWasCalled);
    PPE_LOG_CHECK(WindowTest, dataIsCorrect);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
