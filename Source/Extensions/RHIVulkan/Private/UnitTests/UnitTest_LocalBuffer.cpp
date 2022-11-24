// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RHIVulkan_fwd.h"

#if USE_PPE_RHIDEBUG

#include "RHIApi.h"

#include "Vulkan/Buffer/VulkanLocalBuffer.h"
#include "Vulkan/Command/VulkanBarrierManager.h"

#include "DummyTasks.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NO_INLINE static void Test_LocalBuffer1_() {
    FVulkanBarrierManager barriers;

    const auto tasks = GenerateDummyTasks_(30);
    auto taskIt = tasks.begin();

    FVulkanBuffer globalBuffer;
    FVulkanLocalBuffer localBuffer;
    FVulkanLocalBuffer* const pBuffer = &localBuffer;

    globalBuffer.Write_ForDebug()->Desc = FBufferDesc{ 1024, EBufferUsage::All };

    VerifyRelease( localBuffer.Construct(&globalBuffer) );

    { // pass 1
        pBuffer->AddPendingState(EResourceState::TransferDst, 0, 512, (taskIt++)->get());
        pBuffer->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        auto writeBarriers = pBuffer->WriteAccess_ForDebug();

        AssertRelease(writeBarriers.size() == 1);
        AssertRelease(writeBarriers.back().Range.First == 0);
        AssertRelease(writeBarriers.back().Range.Last == 512);
        AssertRelease(writeBarriers.back().Stages == VK_PIPELINE_STAGE_TRANSFER_BIT);
        AssertRelease(writeBarriers.back().Access == VK_ACCESS_TRANSFER_WRITE_BIT);
        AssertRelease(writeBarriers.back().IsReadable == false);
        AssertRelease(writeBarriers.back().IsWritable == true);
    }
    { // pass 2
        pBuffer->AddPendingState(EResourceState::TransferSrc, 0, 64, (taskIt++)->get());
        pBuffer->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        auto readBarriers = pBuffer->ReadAccess_ForDebug();

        AssertRelease(readBarriers.size() == 1);
        AssertRelease(readBarriers.back().Range.First == 0);
        AssertRelease(readBarriers.back().Range.Last == 64);
        AssertRelease(readBarriers.back().Stages == VK_PIPELINE_STAGE_TRANSFER_BIT);
        AssertRelease(readBarriers.back().Access == VK_ACCESS_TRANSFER_READ_BIT);
        AssertRelease(readBarriers.back().IsReadable == true);
        AssertRelease(readBarriers.back().IsWritable == false);
    }
    { // pass 3
        pBuffer->AddPendingState(EResourceState::UniformRead | EResourceState::_VertexShader, 64, 64+64, (taskIt++)->get());
        pBuffer->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        auto readBarriers = pBuffer->ReadAccess_ForDebug();

        AssertRelease(readBarriers.size() == 2);
        AssertRelease(readBarriers.back().Range.First == 64);
        AssertRelease(readBarriers.back().Range.Last == 64+64);
        AssertRelease(readBarriers.back().Stages == VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
        AssertRelease(readBarriers.back().Access == VK_ACCESS_UNIFORM_READ_BIT);
        AssertRelease(readBarriers.back().IsReadable == true);
        AssertRelease(readBarriers.back().IsWritable == false);
    }
    { // pass 4
        pBuffer->AddPendingState(EResourceState::UniformRead | EResourceState::_FragmentShader, 256, 256+64, (taskIt++)->get());
        pBuffer->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        auto readBarriers = pBuffer->ReadAccess_ForDebug();

        AssertRelease(readBarriers.size() == 3);
        AssertRelease(readBarriers.back().Range.First == 256);
        AssertRelease(readBarriers.back().Range.Last == 256+64);
        AssertRelease(readBarriers.back().Stages == VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        AssertRelease(readBarriers.back().Access == VK_ACCESS_UNIFORM_READ_BIT);
        AssertRelease(readBarriers.back().IsReadable == true);
        AssertRelease(readBarriers.back().IsWritable == false);
    }
    { // pass 5
        pBuffer->AddPendingState(EResourceState::ShaderWrite | EResourceState::_ComputeShader, 512, 512+64, (taskIt++)->get());
        pBuffer->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        auto readBarriers = pBuffer->ReadAccess_ForDebug();
        auto writeBarriers = pBuffer->WriteAccess_ForDebug();

        AssertRelease(readBarriers.size() == 3);
        AssertRelease(writeBarriers.size() == 2);

        AssertRelease(writeBarriers.back().Range.First == 512);
        AssertRelease(writeBarriers.back().Range.Last == 512+64);
        AssertRelease(writeBarriers.back().Stages == VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        AssertRelease(writeBarriers.back().Access == VK_ACCESS_SHADER_WRITE_BIT);
        AssertRelease(writeBarriers.back().IsReadable == false);
        AssertRelease(writeBarriers.back().IsWritable == true);
    }
    { // pass 6
        pBuffer->AddPendingState(EResourceState::UniformRead | EResourceState::_VertexShader, 256+32, 256+64, (taskIt++)->get());
        pBuffer->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        auto readBarriers = pBuffer->ReadAccess_ForDebug();

        AssertRelease(readBarriers.size() == 4);

        AssertRelease(readBarriers[2].Range.First == 256);
        AssertRelease(readBarriers[2].Range.Last == 256+32);
        AssertRelease(readBarriers[2].Stages == VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        AssertRelease(readBarriers[2].Access == VK_ACCESS_UNIFORM_READ_BIT);
        AssertRelease(readBarriers[2].IsReadable == true);
        AssertRelease(readBarriers[2].IsWritable == false);
        AssertRelease(readBarriers[2].Index == 4_execution_order);

        AssertRelease(readBarriers[3].Range.First == 256+32);
        AssertRelease(readBarriers[3].Range.Last == 256+64);
        AssertRelease(readBarriers[3].Stages == VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
        AssertRelease(readBarriers[3].Access == VK_ACCESS_UNIFORM_READ_BIT);
        AssertRelease(readBarriers[3].IsReadable == true);
        AssertRelease(readBarriers[3].IsWritable == false);
        AssertRelease(readBarriers[3].Index == 6_execution_order);
    }
    { // pass 7
        pBuffer->AddPendingState(EResourceState::UniformRead | EResourceState::_GeometryShader, 256+16, 256+16+32, (taskIt++)->get());
        pBuffer->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        auto readBarriers = pBuffer->ReadAccess_ForDebug();

        AssertRelease(readBarriers.size() == 5);

        AssertRelease(readBarriers[2].Range.First == 256);
        AssertRelease(readBarriers[2].Range.Last == 256+16);
        AssertRelease(readBarriers[2].Stages == VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        AssertRelease(readBarriers[2].Access == VK_ACCESS_UNIFORM_READ_BIT);
        AssertRelease(readBarriers[2].IsReadable == true);
        AssertRelease(readBarriers[2].IsWritable == false);
        AssertRelease(readBarriers[2].Index == 4_execution_order);

        AssertRelease(readBarriers[3].Range.First == 256+16);
        AssertRelease(readBarriers[3].Range.Last == 256+16+32);
        AssertRelease(readBarriers[3].Stages == VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT);
        AssertRelease(readBarriers[3].Access == VK_ACCESS_UNIFORM_READ_BIT);
        AssertRelease(readBarriers[3].IsReadable == true);
        AssertRelease(readBarriers[3].IsWritable == false);
        AssertRelease(readBarriers[3].Index == 7_execution_order);

        AssertRelease(readBarriers[4].Range.First == 256+16+32);
        AssertRelease(readBarriers[4].Range.Last == 256+64);
        AssertRelease(readBarriers[4].Stages == VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
        AssertRelease(readBarriers[4].Access == VK_ACCESS_UNIFORM_READ_BIT);
        AssertRelease(readBarriers[4].IsReadable == true);
        AssertRelease(readBarriers[4].IsWritable == false);
        AssertRelease(readBarriers[4].Index == 6_execution_order);
    }
    { // pass 8
        pBuffer->AddPendingState(EResourceState::UniformRead | EResourceState::_GeometryShader, 16, 32, (taskIt++)->get());
        pBuffer->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        auto readBarriers = pBuffer->ReadAccess_ForDebug();

        AssertRelease(readBarriers.size() == 7);

        AssertRelease(readBarriers[0].Range.First == 0);
        AssertRelease(readBarriers[0].Range.Last == 16);
        AssertRelease(readBarriers[0].Stages == VK_PIPELINE_STAGE_TRANSFER_BIT);
        AssertRelease(readBarriers[0].Access == VK_ACCESS_TRANSFER_READ_BIT);
        AssertRelease(readBarriers[0].IsReadable == true);
        AssertRelease(readBarriers[0].IsWritable == false);
        AssertRelease(readBarriers[0].Index == 2_execution_order);

        AssertRelease(readBarriers[1].Range.First == 16);
        AssertRelease(readBarriers[1].Range.Last == 32);
        AssertRelease(readBarriers[1].Stages == VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT);
        AssertRelease(readBarriers[1].Access == VK_ACCESS_UNIFORM_READ_BIT);
        AssertRelease(readBarriers[1].IsReadable == true);
        AssertRelease(readBarriers[1].IsWritable == false);
        AssertRelease(readBarriers[1].Index == 8_execution_order);

        AssertRelease(readBarriers[2].Range.First == 32);
        AssertRelease(readBarriers[2].Range.Last == 64);
        AssertRelease(readBarriers[2].Stages == VK_PIPELINE_STAGE_TRANSFER_BIT);
        AssertRelease(readBarriers[2].Access == VK_ACCESS_TRANSFER_READ_BIT);
        AssertRelease(readBarriers[2].IsReadable == true);
        AssertRelease(readBarriers[2].IsWritable == false);
        AssertRelease(readBarriers[2].Index == 2_execution_order);

        AssertRelease(readBarriers[3].Range.First == 64);
        AssertRelease(readBarriers[3].Range.Last == 64+64);
        AssertRelease(readBarriers[3].Stages == VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
        AssertRelease(readBarriers[3].Access == VK_ACCESS_UNIFORM_READ_BIT);
        AssertRelease(readBarriers[3].IsReadable == true);
        AssertRelease(readBarriers[3].IsWritable == false);
        AssertRelease(readBarriers[3].Index == 3_execution_order);
    }
    { // pass 9
        pBuffer->AddPendingState(EResourceState::ShaderRead | EResourceState::_ComputeShader, 0, 256+32, (taskIt++)->get());
        pBuffer->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        auto readBarriers = pBuffer->ReadAccess_ForDebug();

        AssertRelease(readBarriers.size() == 3);

        AssertRelease(readBarriers[0].Range.First == 0);
        AssertRelease(readBarriers[0].Range.Last == 256+32);
        AssertRelease(readBarriers[0].Stages == VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        AssertRelease(readBarriers[0].Access == VK_ACCESS_SHADER_READ_BIT);
        AssertRelease(readBarriers[0].IsReadable == true);
        AssertRelease(readBarriers[0].IsWritable == false);
        AssertRelease(readBarriers[0].Index == 9_execution_order);

        AssertRelease(readBarriers[1].Range.First == 256+32);
        AssertRelease(readBarriers[1].Range.Last == 256+16+32);
        AssertRelease(readBarriers[1].Stages == VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT);
        AssertRelease(readBarriers[1].Access == VK_ACCESS_UNIFORM_READ_BIT);
        AssertRelease(readBarriers[1].IsReadable == true);
        AssertRelease(readBarriers[1].IsWritable == false);
        AssertRelease(readBarriers[1].Index == 7_execution_order);

        AssertRelease(readBarriers[2].Range.First == 256+16+32);
        AssertRelease(readBarriers[2].Range.Last == 256+64);
        AssertRelease(readBarriers[2].Stages == VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
        AssertRelease(readBarriers[2].Access == VK_ACCESS_UNIFORM_READ_BIT);
        AssertRelease(readBarriers[2].IsReadable == true);
        AssertRelease(readBarriers[2].IsWritable == false);
        AssertRelease(readBarriers[2].Index == 6_execution_order);
    }
    { // pass 10
        pBuffer->AddPendingState(EResourceState::TransferDst, 64, 512, (taskIt++)->get());
        pBuffer->CommitBarrier(barriers ARGS_IF_RHIDEBUG(nullptr));

        auto readBarriers = pBuffer->ReadAccess_ForDebug();
        auto writeBarriers = pBuffer->WriteAccess_ForDebug();

        AssertRelease(readBarriers.size() == 1);
        AssertRelease(writeBarriers.size() == 3);

        AssertRelease(readBarriers[0].Range.First == 0);
        AssertRelease(readBarriers[0].Range.Last == 64);
        AssertRelease(readBarriers[0].Stages == VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        AssertRelease(readBarriers[0].Access == VK_ACCESS_SHADER_READ_BIT);
        AssertRelease(readBarriers[0].IsReadable == true);
        AssertRelease(readBarriers[0].IsWritable == false);
        AssertRelease(readBarriers[0].Index == 9_execution_order);

        AssertRelease(writeBarriers[0].Range.First == 0);
        AssertRelease(writeBarriers[0].Range.Last == 64);
        AssertRelease(writeBarriers[0].Stages == VK_PIPELINE_STAGE_TRANSFER_BIT);
        AssertRelease(writeBarriers[0].Access == VK_ACCESS_TRANSFER_WRITE_BIT);
        AssertRelease(writeBarriers[0].IsReadable == false);
        AssertRelease(writeBarriers[0].IsWritable == true);
        AssertRelease(writeBarriers[0].Index == 1_execution_order);

        AssertRelease(writeBarriers[1].Range.First == 64);
        AssertRelease(writeBarriers[1].Range.Last == 512);
        AssertRelease(writeBarriers[1].Stages == VK_PIPELINE_STAGE_TRANSFER_BIT);
        AssertRelease(writeBarriers[1].Access == VK_ACCESS_TRANSFER_WRITE_BIT);
        AssertRelease(writeBarriers[1].IsReadable == false);
        AssertRelease(writeBarriers[1].IsWritable == true);
        AssertRelease(writeBarriers[1].Index == 10_execution_order);

        AssertRelease(writeBarriers[2].Range.First == 512);
        AssertRelease(writeBarriers[2].Range.Last == 512+64);
        AssertRelease(writeBarriers[2].Stages == VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        AssertRelease(writeBarriers[2].Access == VK_ACCESS_SHADER_WRITE_BIT);
        AssertRelease(writeBarriers[2].IsReadable == false);
        AssertRelease(writeBarriers[2].IsWritable == true);
        AssertRelease(writeBarriers[2].Index == 5_execution_order);
    }

    localBuffer.ResetState(EVulkanExecutionOrder::Final, barriers ARGS_IF_RHIDEBUG(nullptr));
    localBuffer.TearDown();
    //globalBuffer.TearDown();
}
//----------------------------------------------------------------------------
void UnitTest_LocalBuffer() {
    Test_LocalBuffer1_();

    LOG(RHI, Info, L"UnitTest_LocalBuffer [PASSED]");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIDEBUG
