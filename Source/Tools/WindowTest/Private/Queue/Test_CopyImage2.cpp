#include "stdafx.h"

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Test_CopyImage2_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    const uint2	srcDim = { 64, 64 };
    const uint2	dstDim = { 128, 128 };
    const int2 imgOffset = { 16, 27 };
    const size_t bpp = 4_b;
    const size_t srcRowPitch = srcDim.x * bpp;

    auto srcImage = fg.ScopedResource(fg.CreateImage(FImageDesc{}
        .SetDimension(srcDim)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Transfer),
        Default ARGS_IF_RHIDEBUG("SrcImage")));
    LOG_CHECK(WindowTest, srcImage.Valid());

    auto dstImage = fg.ScopedResource(fg.CreateImage(FImageDesc{}
        .SetDimension(dstDim)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Transfer),
        Default ARGS_IF_RHIDEBUG("DstImage")));
    LOG_CHECK(WindowTest, dstImage.Valid());

    RAWSTORAGE(UnitTest, u8) srcData;
    srcData.Resize_DiscardData(srcRowPitch * srcDim.y);
    forrange(y, 0, srcDim.y) {
        forrange(x, 0, srcDim.x) {
            u8* const texel = &srcData[x * bpp + y * srcRowPitch];
            texel[0] = static_cast<u8>(x);
            texel[1] = static_cast<u8>(y);
            texel[2] = static_cast<u8>(Max(x,y));
            texel[3] = 0_u8;
        }
    }

    bool cbWasCalled = false;
    bool dataIsCorrect = false;

    const auto onLoaded = [&](const FImageView& imageData) {
        cbWasCalled = true;
        dataIsCorrect = true;

        forrange(y, 0, srcDim.y) {
            const auto dstRow = imageData.Row(y + imgOffset.y);
            forrange(x, 0, srcDim.x) {
                const u8* src = &srcData[x * bpp + y * srcRowPitch];
                const u8* dst = &dstRow[(x + imgOffset.x) * bpp];

                const bool isEqual = (
                    src[0] == dst[0] &&
                    src[1] == dst[1] &&
                    src[2] == dst[2] &&
                    src[3] == dst[3]);

                /*LOG(WindowTest, Debug, L"Read({0}) -> {1} vs {2} == {3}",
                    uint2(x,y),
                    ubyte4(src[0], src[1], src[2], src[3]),
                    ubyte4(dst[0], dst[1], dst[2], dst[3]),
                    isEqual);*/
                dataIsCorrect &= isEqual;
            }
        }
    };

    {
        auto cmd = fg.Begin(FCommandBufferDesc{}
            .SetName("Test_CopyImage2_::Frame1")
            .SetDebugFlags(Default));
        LOG_CHECK(WindowTest, !!cmd);

        const uint2 testDim = { srcDim.x, srcDim.y / 2 };
        const auto testData = srcData.MakeView().CutBefore(srcData.size() / 2);

        const PFrameTask tUpdate = cmd->Task(FUpdateImage{}
            .SetImage(srcImage)
            .SetData(testData, testDim));
        const PFrameTask tCopy = cmd->Task(FCopyImage{}
            .From(srcImage).To(dstImage)
            .AddRegion({}, int2(0), {}, imgOffset, testDim)
            .DependsOn(tUpdate));
        UNUSED(tCopy);

        LOG_CHECK(WindowTest, fg.Execute(cmd));
    }

    {
        auto cmd = fg.Begin(FCommandBufferDesc{}
            .SetName("Test_CopyImage2_::Frame2")
            .SetDebugFlags(Default));
        LOG_CHECK(WindowTest, !!cmd);

        const uint2 testDim = { srcDim.x, srcDim.y / 2 };
        const int2 testOff = { 0, checked_cast<int>(srcDim.y / 2) };
        const auto testData = srcData.MakeView().CutStartingAt(srcData.size() / 2);

        const PFrameTask tUpdate = cmd->Task(FUpdateImage{}
            .SetImage(srcImage, testOff)
            .SetData(testData, testDim));
        const PFrameTask tCopy = cmd->Task(FCopyImage{}
            .From(srcImage).To(dstImage)
            .AddRegion({}, testOff, {}, testOff + imgOffset, testDim)
            .DependsOn(tUpdate));
        const PFrameTask tRead = cmd->Task(FReadImage{}
            .SetImage(dstImage, int2(0), dstDim)
            .SetCallback(onLoaded)
            .DependsOn(tCopy));
        UNUSED(tRead);

        LOG_CHECK(WindowTest, fg.Execute(cmd));
    }

    // after execution 'src_data' was copied to 'src_image', 'src_image' copied to 'dst_image', 'dst_image' copied to staging buffer...
    LOG_CHECK(WindowTest, not cbWasCalled);

    // all staging buffers will be synchronized, all 'ReadImage' callbacks will be called.
    LOG_CHECK(WindowTest, fg.WaitIdle());

    LOG_CHECK(WindowTest, cbWasCalled);
    LOG_CHECK(WindowTest, dataIsCorrect);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
