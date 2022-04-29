#include "stdafx.h"

#include "Test_Includes.h"
#include "Thread/ReadWriteLock.h"
#include "Thread/SynchronizationBarrier.h"
#include "Thread/ThreadPool.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FMultithreading2_ {
    STATIC_CONST_INTEGRAL(size_t, MaxCount, 1000);

    RHI::SFrameGraph Fg;
    RHI::FRawGPipelineID Pipeline;

    FReadWriteLock ImageGuard;
    FSynchronizationBarrier Sync{2};

    RHI::FImageID Image;
    RHI::PCommandBatch CmdBuffers[2];
    RHI::PCommandBatch PerFrame[2];

    bool RenderThread1() {
        const uint2 viewSize{ 800, 600 };

        ImageGuard.LockWrite();

        forrange(i, 0, MaxCount) {
            Fg->Wait({ PerFrame[i&1] });

            RHI::FCommandBufferBatch cmd = Fg->Begin(RHI::FCommandBufferDesc{ RHI::EQueueType::Graphics }
                .SetName("RenderThread1"));
            LOG_CHECK(WindowTest, !!cmd);

            PerFrame[i&1] = cmd;
            CmdBuffers[0] = cmd;

            // (1) wake up all threads
            Sync.Wait();

            Image = Fg->CreateImage(RHI::FImageDesc{}
                .SetDimension(viewSize)
                .SetFormat(RHI::EPixelFormat::RGBA8_UNorm)
                .SetUsage(RHI::EImageUsage::ColorAttachment | RHI::EImageUsage::TransferSrc),
                Default ARGS_IF_RHIDEBUG("RenderTarget1"));
            LOG_CHECK(WindowTest, !!Image);

            // notify all threads that image is created
            ImageGuard.UnlockWrite();

            RHI::FLogicalPassID renderPass = cmd->CreateRenderPass(RHI::FRenderPassDesc{ viewSize }
                .AddTarget(RHI::ERenderTargetID::Color0, Image, FLinearColor::Transparent(), RHI::EAttachmentStoreOp::Store)
                .AddViewport(viewSize));
            LOG_CHECK(WindowTest, !!renderPass);

            cmd->Task(renderPass, RHI::FDrawVertices{}
                .Draw(3)
                .SetPipeline(Pipeline)
                .SetTopology(RHI::EPrimitiveTopology::TriangleList));

            RHI::PFrameTask tDraw = cmd->Task(RHI::FSubmitRenderPass{ renderPass });
            Unused(tDraw);

            LOG_CHECK(WindowTest, Fg->Execute(cmd));

            // (2) wait until all threads complete command buffer recording
            Sync.Wait();

            ImageGuard.LockWrite();
            Unused(Fg->ReleaseResource(Image));

            LOG_CHECK(WindowTest, Fg->Flush());
        }

        ImageGuard.UnlockWrite();
        return true;
    }

    bool RenderThread2() {
        forrange(i, 0, MaxCount) {
            RHI::FCommandBufferBatch cmd = Fg->Begin(RHI::FCommandBufferDesc{ RHI::EQueueType::Graphics }
                .SetName("RenderThread2"));
            LOG_CHECK(WindowTest, !!cmd);

            CmdBuffers[1] = cmd;

            // (1) wake for second command buffer
            Sync.Wait();

            cmd->DependsOn(CmdBuffers[0]);

            // wait until image was created
            ImageGuard.LockRead();

            const RHI::FImageDesc& desc = Fg->Description(Image);

            RHI::FLogicalPassID renderPass = cmd->CreateRenderPass(RHI::FRenderPassDesc{ desc.Dimensions.xy }
                .AddTarget(RHI::ERenderTargetID::Color0, Image, RHI::EAttachmentLoadOp::Load, RHI::EAttachmentStoreOp::Store)
                .AddViewport(desc.Dimensions.xy));
            LOG_CHECK(WindowTest, !!renderPass);

            cmd->Task(renderPass, RHI::FDrawVertices{}
                .Draw(3)
                .SetPipeline(Pipeline)
                .SetTopology(RHI::EPrimitiveTopology::TriangleList));

            RHI::PFrameTask tDraw = cmd->Task(RHI::FSubmitRenderPass{ renderPass });
            Unused(tDraw);

            LOG_CHECK(WindowTest, Fg->Execute(cmd));

            // image is no longer needed
            ImageGuard.UnlockRead();

            // (2) notify that thread has already finished recording the command buffer
            Sync.Wait();
        }

        return true;
    }

};
//----------------------------------------------------------------------------
bool Test_Multithreading2_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    const SFrameGraph fg = app.RHI().FrameGraph();

    FGraphicsPipelineDesc desc;
    desc.AddShader(EShaderType::Vertex, EShaderLangFormat::VKSL_100, "main", R"#(
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
ARGS_IF_RHIDEBUG("Test_Multithreading2_VS"));
        desc.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) in  vec3	v_Color;
layout(location=0) out vec4	out_Color;

void main() {
	out_Color = vec4(v_Color, 1.0);
}
)#"
ARGS_IF_RHIDEBUG("Test_Multithreading2_PS"));

    TAutoResource<FGPipelineID> ppln{ *fg, fg->CreatePipeline(desc ARGS_IF_RHIDEBUG("Test_Multithreading2")) };
    LOG_CHECK(WindowTest, ppln.Valid());

    FMultithreading2_ context;
    context.Fg = fg;
    context.Pipeline = ppln;

    bool threadResult1 = false;
    bool threadResult2 = false;

    FTaskFunc tasks[2] = {
        [&threadResult1, &context](ITaskContext&) { threadResult1 = context.RenderThread1(); },
        [&threadResult2, &context](ITaskContext&) { threadResult2 = context.RenderThread2(); } };

    FHighPriorityThreadPool::Get().RunAndWaitFor(MakeView(tasks));

    LOG_CHECK(WindowTest, fg->WaitIdle());
    LOG_CHECK(WindowTest, threadResult1);
    LOG_CHECK(WindowTest, threadResult2);

    Broadcast(MakeView(context.CmdBuffers), Default);
    Broadcast(MakeView(context.PerFrame), Default);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
