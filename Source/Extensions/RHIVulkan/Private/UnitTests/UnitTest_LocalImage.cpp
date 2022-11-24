// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RHIVulkan_fwd.h"

#if USE_PPE_RHIDEBUG

#include "RHIApi.h"

#include "Vulkan/Image/VulkanLocalImage.h"
#include "Vulkan/Command/VulkanBarrierManager.h"

#include "DummyTasks.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void CreateUnitTestImage_(FVulkanImage* pImage, const FImageDesc& desc) {
    Assert(pImage);

    const auto exclusiveImage = pImage->Write_ForDebug();
    exclusiveImage->Desc = desc;
    exclusiveImage->Desc.Validate();
    exclusiveImage->DefaultLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}
//----------------------------------------------------------------------------
static void CheckLayersForDebug_(
    const FVulkanLocalImage::FImageAccess& barrier,
    u32 arrayLayers, u32 baseMipLevel, u32 levelCount, u32 baseArrayLayer, u32 layerCount ) {
    const u32 baseMipLevel_  = (barrier.Range.First / arrayLayers);
    const u32 levelCount_ = ((barrier.Range.Last - barrier.Range.First - 1) / arrayLayers + 1);
    const u32 baseArrayLayer_ = (barrier.Range.First % arrayLayers);
    const u32 layerCount_ = ((barrier.Range.Last - barrier.Range.First - 1) % arrayLayers + 1);

    AssertRelease(baseMipLevel == baseMipLevel_);
    AssertRelease(levelCount == levelCount_);
    AssertRelease(baseArrayLayer == baseArrayLayer_);
    AssertRelease(layerCount == layerCount_);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NO_INLINE static void Test_LocalImage1_() {
    FVulkanBarrierManager barriers;

    const auto tasks = GenerateDummyTasks_(30);
    auto taskIt = tasks.begin();

    FVulkanImage globalImage;
    FVulkanLocalImage localImage;
    FVulkanLocalImage* const pImage = &localImage;

    CreateUnitTestImage_(&globalImage, FImageDesc{}
        .SetDimension({ 64, 64 })
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::ColorAttachment|EImageUsage::Transfer|EImageUsage::Storage|EImageUsage::Sampled)
        .SetMaxMipmaps(11) );

    VerifyRelease( localImage.Construct(&globalImage) );

    { // pass 1
        pImage->AddPendingState(
            EResourceState::TransferDst,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            FImageDataRange{ 0_layer, 1, 0_mipmap, 1 },
            VK_IMAGE_ASPECT_COLOR_BIT, (taskIt++)->get() );
        pImage->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        const auto rwBarriers = pImage->ReadWriteAccess_ForDebug();

        AssertRelease(rwBarriers.size() == 2);

        AssertRelease(rwBarriers[0].Range.First == 0);
        AssertRelease(rwBarriers[0].Range.Last == 1);
        AssertRelease(rwBarriers[0].Stages == VK_PIPELINE_STAGE_TRANSFER_BIT);
        AssertRelease(rwBarriers[0].Access == VK_ACCESS_TRANSFER_WRITE_BIT);
        AssertRelease(rwBarriers[0].IsReadable == false);
        AssertRelease(rwBarriers[0].IsWritable == true);
        AssertRelease(rwBarriers[0].Layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        AssertRelease(rwBarriers[0].Index == 1_execution_order);

        CheckLayersForDebug_(rwBarriers[0], pImage->Read()->ArrayLayers(), 0, 1, 0, pImage->Read()->ArrayLayers());

        AssertRelease(rwBarriers[1].Range.First == 1);
        AssertRelease(rwBarriers[1].Range.Last == 7);
        AssertRelease(rwBarriers[1].Stages == VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        AssertRelease(rwBarriers[1].Access == 0);
        AssertRelease(rwBarriers[1].IsReadable == false);
        AssertRelease(rwBarriers[1].IsWritable == false);
        AssertRelease(rwBarriers[1].Layout == VK_IMAGE_LAYOUT_UNDEFINED);
        AssertRelease(rwBarriers[1].Index == EVulkanExecutionOrder::Initial);

        CheckLayersForDebug_(rwBarriers[1], pImage->Read()->ArrayLayers(), 1, pImage->Read()->MipmapLevels() - 1, 0, pImage->Read()->ArrayLayers());
    }

    localImage.ResetState(EVulkanExecutionOrder::Final, barriers ARGS_IF_RHIDEBUG(nullptr));
    localImage.TearDown();

    // globalImage.TearDown();
}
//----------------------------------------------------------------------------
NO_INLINE static void Test_LocalImage2_() {
    FVulkanBarrierManager barriers;

    const auto tasks = GenerateDummyTasks_(30);
    auto taskIt = tasks.begin();

    FVulkanImage globalImage;
    FVulkanLocalImage localImage;
    FVulkanLocalImage* const pImage = &localImage;

    CreateUnitTestImage_(&globalImage, FImageDesc{}
        .SetDimension({ 64, 64 })
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::ColorAttachment|EImageUsage::Transfer|EImageUsage::Storage|EImageUsage::Sampled)
        .SetMaxMipmaps(11)
        .SetArrayLayers(8) );

    VerifyRelease( localImage.Construct(&globalImage) );

    { // pass 1
        pImage->AddPendingState(
            EResourceState::TransferDst,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            FImageDataRange{ 0_layer, 2, 0_mipmap, pImage->Read()->MipmapLevels() },
            VK_IMAGE_ASPECT_COLOR_BIT,
            (taskIt++)->get() );
        pImage->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        const auto rwBarriers = pImage->ReadWriteAccess_ForDebug();

        AssertRelease(rwBarriers.size() == 14);

        for (u32 i = 0; i < rwBarriers.size(); ++i) {
            const u32 j = (i/2);

            AssertRelease(rwBarriers[i].Range.First == j*8);
            AssertRelease(rwBarriers[i].Range.Last == 2+j*8);
            AssertRelease(rwBarriers[i].Stages == VK_PIPELINE_STAGE_TRANSFER_BIT);
            AssertRelease(rwBarriers[i].Access == VK_ACCESS_TRANSFER_WRITE_BIT);
            AssertRelease(rwBarriers[i].IsReadable == false);
            AssertRelease(rwBarriers[i].IsWritable == true);
            AssertRelease(rwBarriers[i].Layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            AssertRelease(rwBarriers[i].Index == 1_execution_order);

            CheckLayersForDebug_(rwBarriers[i], pImage->Read()->ArrayLayers(), j, 1, 0, 2);

            ++i;
            AssertRelease(rwBarriers[i].Range.First == 2+j*8);
            AssertRelease(rwBarriers[i].Range.Last == 8+j*8);
            AssertRelease(rwBarriers[i].Stages == VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
            AssertRelease(rwBarriers[i].Access == 0);
            AssertRelease(rwBarriers[i].IsReadable == false);
            AssertRelease(rwBarriers[i].IsWritable == false);
            AssertRelease(rwBarriers[i].Layout == VK_IMAGE_LAYOUT_UNDEFINED);
            AssertRelease(rwBarriers[i].Index == EVulkanExecutionOrder::Initial);

            CheckLayersForDebug_(rwBarriers[i], pImage->Read()->ArrayLayers(), j, 1, 2, pImage->Read()->ArrayLayers() - 2);
        }
    }
    { // pass 2
        pImage->AddPendingState(
            EResourceState::ShaderSample | EResourceState::_FragmentShader,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            FImageDataRange{ 0_layer, pImage->Read()->ArrayLayers(), 0_mipmap, 2 },
            VK_IMAGE_ASPECT_COLOR_BIT,
            (taskIt++)->get() );
        pImage->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        const auto rwBarriers = pImage->ReadWriteAccess_ForDebug();

        AssertRelease(rwBarriers.size() == 12);

        for (u32 i = 0; i < 2; ++i) {
            AssertRelease(rwBarriers[i].Range.First == 8*i+0);
            AssertRelease(rwBarriers[i].Range.Last == 8*i+8);
            AssertRelease(rwBarriers[i].Stages == VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            AssertRelease(rwBarriers[i].Access == VK_ACCESS_SHADER_READ_BIT);
            AssertRelease(rwBarriers[i].IsReadable == true);
            AssertRelease(rwBarriers[i].IsWritable == false);
            AssertRelease(rwBarriers[i].Layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            AssertRelease(rwBarriers[i].Index == 2_execution_order);

            CheckLayersForDebug_(rwBarriers[i], pImage->Read()->ArrayLayers(), i, 1, 0, pImage->Read()->ArrayLayers());
        }

        // from previous pass:

        for (u32 i = 2; i < rwBarriers.size(); ++i) {
            const u32 j = (i/2) + 1;

            AssertRelease(rwBarriers[i].Range.First == j*8);
            AssertRelease(rwBarriers[i].Range.Last == 2+j*8);
            AssertRelease(rwBarriers[i].Stages == VK_PIPELINE_STAGE_TRANSFER_BIT);
            AssertRelease(rwBarriers[i].Access == VK_ACCESS_TRANSFER_WRITE_BIT);
            AssertRelease(rwBarriers[i].IsReadable == false);
            AssertRelease(rwBarriers[i].IsWritable == true);
            AssertRelease(rwBarriers[i].Layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            AssertRelease(rwBarriers[i].Index == 1_execution_order);

            CheckLayersForDebug_(rwBarriers[i], pImage->Read()->ArrayLayers(), j, 1, 0, 2);

            ++i;
            AssertRelease(rwBarriers[i].Range.First == 2+j*8);
            AssertRelease(rwBarriers[i].Range.Last == 8+j*8);
            AssertRelease(rwBarriers[i].Stages == VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
            AssertRelease(rwBarriers[i].Access == 0);
            AssertRelease(rwBarriers[i].IsReadable == false);
            AssertRelease(rwBarriers[i].IsWritable == false);
            AssertRelease(rwBarriers[i].Layout == VK_IMAGE_LAYOUT_UNDEFINED);
            AssertRelease(rwBarriers[i].Index == EVulkanExecutionOrder::Initial);

            CheckLayersForDebug_(rwBarriers[i], pImage->Read()->ArrayLayers(), j, 1, 2, pImage->Read()->ArrayLayers() - 2);
        }
    }
    { // pass 3
        pImage->AddPendingState(
            EResourceState::ShaderWrite | EResourceState::_ComputeShader,
            VK_IMAGE_LAYOUT_GENERAL,
            FImageDataRange{ 0_layer, pImage->Read()->ArrayLayers(), 0_mipmap, pImage->Read()->MipmapLevels() },
            VK_IMAGE_ASPECT_COLOR_BIT,
            (taskIt++)->get() );
        pImage->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        const auto rwBarriers = pImage->ReadWriteAccess_ForDebug();

        AssertRelease(rwBarriers.size() == 7);

        for (u32 i = 0; i < rwBarriers.size(); ++i) {
            AssertRelease(rwBarriers[i].Range.First == 8*i+0);
            AssertRelease(rwBarriers[i].Range.Last == 8*(i+1));
            AssertRelease(rwBarriers[i].Stages == VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            AssertRelease(rwBarriers[i].Access == VK_ACCESS_SHADER_WRITE_BIT);
            AssertRelease(rwBarriers[i].IsReadable == false);
            AssertRelease(rwBarriers[i].IsWritable == true);
            AssertRelease(rwBarriers[i].Layout == VK_IMAGE_LAYOUT_GENERAL);
            AssertRelease(rwBarriers[i].Index == 3_execution_order);

            CheckLayersForDebug_(rwBarriers[i], pImage->Read()->ArrayLayers(), i, 1, 0, pImage->Read()->ArrayLayers());
        }
    }

    localImage.ResetState(EVulkanExecutionOrder::Final, barriers ARGS_IF_RHIDEBUG(nullptr));
    localImage.TearDown();

    // globalImage.TearDown();
}
//----------------------------------------------------------------------------
void UnitTest_LocalImage() {
    Test_LocalImage1_();
    Test_LocalImage2_();

    LOG(RHI, Info, L"UnitTest_LocalImage [PASSED]");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIDEBUG
