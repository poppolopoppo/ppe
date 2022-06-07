#include "stdafx.h"

#include "Test_Includes.h"
#include "Thread/SynchronizationBarrier.h"
#include "Thread/ThreadPool.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FMultithreading4_ {
    STATIC_CONST_INTEGRAL(size_t, MaxCount, 1000);
    STATIC_CONST_INTEGRAL(RHI::EQueueUsage, QueueUsage, RHI::EQueueUsage::All);

    RHI::SFrameGraph Fg;
    RHI::FRawCPipelineID CPipeline;
    RHI::FRawGPipelineID GPipeline;

    FSynchronizationBarrier Sync{4};

    RHI::FImageID Images[4];
    RHI::PCommandBatch CmdBuffers[4];
    RHI::PCommandBatch PerFrame[4];

    bool RenderThread1() {
        const uint2 viewSize{ 800, 600 };

        Assert_NoAssume(not Images[0]);
        Images[0] = Fg->CreateImage(RHI::FImageDesc{}
                    .SetDimension(viewSize)
                    .SetFormat(RHI::EPixelFormat::RGBA8_UNorm)
                    .SetUsage(RHI::EImageUsage::ColorAttachment | RHI::EImageUsage::TransferSrc)
                    .SetQueues(QueueUsage),
                    Default ARGS_IF_RHIDEBUG("RenderTarget1"));
        LOG_CHECK(WindowTest, !!Images[0]);

        ON_SCOPE_EXIT([this]() {
            Unused(Fg->ReleaseResource(Images[0]));
        });

        // (0) wait until all shared resources have been initialized
        Sync.Wait();

        forrange(i, 0, MaxCount) {
            Fg->Wait({ PerFrame[i&1] });

            RHI::FCommandBufferBatch cmd = Fg->Begin(RHI::FCommandBufferDesc{ RHI::EQueueType::Graphics }
                .SetName("RenderThread1"));
            LOG_CHECK(WindowTest, !!cmd);

            PerFrame[i&1] = cmd;
            CmdBuffers[0] = cmd;

            // add dependency from previous frame because those commands share the same data, but used in different queues
            cmd->DependsOn(CmdBuffers[3]);

            // (1) wake up all render threads
            Sync.Wait();

            RHI::FLogicalPassID renderPass = cmd->CreateRenderPass(RHI::FRenderPassDesc{ viewSize }
                .AddTarget(RHI::ERenderTargetID::Color0, Images[0], FLinearColor::Transparent(), RHI::EAttachmentStoreOp::Store)
                .AddViewport(viewSize));
            LOG_CHECK(WindowTest, !!renderPass);

            cmd->Task(renderPass, RHI::FDrawVertices{}
                .Draw(3)
                .SetPipeline(GPipeline)
                .SetTopology(RHI::EPrimitiveTopology::TriangleList));

            RHI::PFrameTask tDraw = cmd->Task(RHI::FSubmitRenderPass{ renderPass });
            Unused(tDraw);

            LOG_CHECK(WindowTest, Fg->Execute(cmd));

            // (2) wait until all threads complete command buffer recording
            Sync.Wait();

            LOG_CHECK(WindowTest, Fg->Flush());
        }

        return true;
    }

    bool RenderThread2() {
        const uint2 viewSize{ 1024, 1024 };
        const uint2 localSize{ 8, 8 };

        Assert_NoAssume(not Images[1]);
        Images[1] = Fg->CreateImage(RHI::FImageDesc{}
           .SetDimension(viewSize)
           .SetFormat(RHI::EPixelFormat::RGBA8_UNorm)
           .SetUsage(RHI::EImageUsage::Storage | RHI::EImageUsage::TransferSrc)
           .SetQueues(QueueUsage),
           Default ARGS_IF_RHIDEBUG("RenderTarget2"));
        LOG_CHECK(WindowTest, !!Images[1]);

        ON_SCOPE_EXIT([this]() {
            Unused(Fg->ReleaseResource(Images[1]));
        });

        RHI::PPipelineResources resources = NEW_REF(RHIPipeline, RHI::FPipelineResources);
        LOG_CHECK(WindowTest, Fg->InitPipelineResources(resources.get(), CPipeline, "0"_descriptorset));
        resources->BindImage("un_OutImage"_uniform, Images[1]);

        // (0) wait until all shared resources have been initialized
        Sync.Wait();

        forrange(i, 0, MaxCount) {
            RHI::FCommandBufferBatch cmd = Fg->Begin(RHI::FCommandBufferDesc{ RHI::EQueueType::AsyncCompute }
                .SetName("RenderThread2"));
            LOG_CHECK(WindowTest, !!cmd);

            CmdBuffers[1] = cmd;

            // (1) wait for first command buffer
            Sync.Wait();

            cmd->DependsOn(CmdBuffers[0]);

            RHI::PFrameTask tComp = cmd->Task(RHI::FDispatchCompute{}
                .SetPipeline(CPipeline)
                .AddResources("0"_descriptorset, resources)
                .SetLocalSize(localSize)
                .Dispatch(IntDivCeil(viewSize, localSize)));
            Unused(tComp);

            LOG_CHECK(WindowTest, Fg->Execute(cmd));

            // (2) notify that thread has already finished recording the command buffer
            Sync.Wait();
        }

        return true;
    }


    bool RenderThread3() {
        const uint2 viewSize{ 500, 1700 };

        Assert_NoAssume(not Images[2]);
        Images[2] = Fg->CreateImage(RHI::FImageDesc{}
            .SetDimension(viewSize)
            .SetFormat(RHI::EPixelFormat::RGBA16_UNorm)
            .SetUsage(RHI::EImageUsage::ColorAttachment | RHI::EImageUsage::TransferSrc)
            .SetQueues(QueueUsage),
            Default ARGS_IF_RHIDEBUG("RenderTarget3"));
        LOG_CHECK(WindowTest, !!Images[2]);

        ON_SCOPE_EXIT([this]() {
            Unused(Fg->ReleaseResource(Images[2]));
        });

        // (0) wait until all shared resources have been initialized
        Sync.Wait();

        forrange(i, 0, MaxCount) {
            // (1) wake for second command buffer
            Sync.Wait();

            RHI::FCommandBufferBatch cmd = Fg->Begin(RHI::FCommandBufferDesc{ RHI::EQueueType::Graphics }
                .SetName("RenderThread3"));
            LOG_CHECK(WindowTest, !!cmd);

            CmdBuffers[2] = cmd;
            cmd->DependsOn(CmdBuffers[1]);

            RHI::FLogicalPassID renderPass = cmd->CreateRenderPass(RHI::FRenderPassDesc{ viewSize }
                .AddTarget(RHI::ERenderTargetID::Color0, Images[2], FLinearColor::Transparent(), RHI::EAttachmentStoreOp::Store)
                .AddViewport(viewSize));
            LOG_CHECK(WindowTest, !!renderPass);

            cmd->Task(renderPass, RHI::FDrawVertices{}
                .Draw(3)
                .SetPipeline(GPipeline)
                .SetTopology(RHI::EPrimitiveTopology::TriangleList));

            RHI::PFrameTask tDraw = cmd->Task(RHI::FSubmitRenderPass{ renderPass });
            Unused(tDraw);

            LOG_CHECK(WindowTest, Fg->Execute(cmd));

            // (2) notify that thread has already finished recording the command buffer
            Sync.Wait();
        }

        return true;
    }

    bool RenderThread4() {
        const uint2 viewSize{ 1024, 1024 };

        Assert_NoAssume(not Images[3]);
        Images[3] = Fg->CreateImage(RHI::FImageDesc{}
            .SetDimension(viewSize)
            .SetFormat(RHI::EPixelFormat::RGBA8_UNorm)
            .SetUsage(RHI::EImageUsage::TransferDst)
            .SetQueues(QueueUsage),
            Default ARGS_IF_RHIDEBUG("RenderTarget4"));
        LOG_CHECK(WindowTest, !!Images[3]);

        ON_SCOPE_EXIT([this]() {
            Unused(Fg->ReleaseResource(Images[3]));
        });

        // (0) wait until all shared resources have been initialized
        Sync.Wait();

        forrange(i, 0, MaxCount) {
            RHI::FCommandBufferBatch cmd = Fg->Begin(RHI::FCommandBufferDesc{ RHI::EQueueType::AsyncTransfer }
                .SetName("RenderThread4"));
            LOG_CHECK(WindowTest, !!cmd);

            // (1) wake for first and second command buffer
            Sync.Wait();

            CmdBuffers[3] = cmd;

            cmd->DependsOn(CmdBuffers[0]);
            cmd->DependsOn(CmdBuffers[1]);

            RHI::PFrameTask tCopy1 = cmd->Task(RHI::FCopyImage{}.From(Images[0]).To(Images[3]).AddRegion(Default, {16,16}, Default, {0,0}, {256,256}));
            LOG_CHECK(WindowTest, !!tCopy1);
            RHI::PFrameTask tCopy2 = cmd->Task(RHI::FCopyImage{}.From(Images[1]).To(Images[3]).AddRegion(Default, {256,256}, Default, {256,256}, {256,256}));
            LOG_CHECK(WindowTest, !!tCopy2);

            Unused(tCopy1, tCopy2);

            LOG_CHECK(WindowTest, Fg->Execute(cmd));

            // (2) notify that thread has already finished recording the command buffer
            Sync.Wait();
        }

        return true;
    }

};
//----------------------------------------------------------------------------
bool Impl_Multithreading4_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    const SFrameGraph fg = app.RHI().FrameGraph();

    FGraphicsPipelineDesc gdesc;
    gdesc.AddShader(EShaderType::Vertex, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) out vec3	v_Color;

const vec2	g_Positions[3] = vec2[](
	vec2(0.0, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
);

const vec3	g_Colors[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

void main() {
	gl_Position	= vec4( g_Positions[gl_VertexIndex], 0.0, 1.0 );
	v_Color		= g_Colors[gl_VertexIndex];
}
)#"
ARGS_IF_RHIDEBUG("Impl_Multithreading4_VS"));
    gdesc.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) in  vec3	v_Color;
layout(location=0) out vec4	out_Color;

void main() {
	out_Color = vec4(v_Color, 1.0);
}
)#"
ARGS_IF_RHIDEBUG("Impl_Multithreading4_PS"));

    TAutoResource<FGPipelineID> gppln{ *fg, fg->CreatePipeline(gdesc ARGS_IF_RHIDEBUG("Impl_Multithreading4_Graphics")) };
    LOG_CHECK(WindowTest, gppln.Valid());

    FComputePipelineDesc cdesc;
    cdesc.AddShader(EShaderLangFormat::VKSL_100, "main", R"#(
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
ARGS_IF_RHIDEBUG("Impl_Multithreading4_VS"));

    TAutoResource<FCPipelineID> cppln{ *fg, fg->CreatePipeline(cdesc ARGS_IF_RHIDEBUG("Impl_Multithreading4_Compute")) };
    LOG_CHECK(WindowTest, cppln.Valid());

    FMultithreading4_ context;
    context.Fg = fg;
    context.GPipeline = gppln;
    context.CPipeline = cppln;

    bool threadResult1 = false;
    bool threadResult2 = false;
    bool threadResult3 = false;
    bool threadResult4 = false;

    FTaskFunc tasks[4] = {
        [&threadResult1, &context](ITaskContext&) { threadResult1 = context.RenderThread1(); },
        [&threadResult2, &context](ITaskContext&) { threadResult2 = context.RenderThread2(); },
        [&threadResult3, &context](ITaskContext&) { threadResult3 = context.RenderThread3(); },
        [&threadResult4, &context](ITaskContext&) { threadResult4 = context.RenderThread4(); } };

    FHighPriorityThreadPool::Get().RunAndWaitFor(MakeView(tasks));

    LOG_CHECK(WindowTest, fg->WaitIdle());
    LOG_CHECK(WindowTest, threadResult1);
    LOG_CHECK(WindowTest, threadResult2);
    LOG_CHECK(WindowTest, threadResult3);
    LOG_CHECK(WindowTest, threadResult4);

    Broadcast(MakeView(context.CmdBuffers), Default);
    Broadcast(MakeView(context.PerFrame), Default);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
