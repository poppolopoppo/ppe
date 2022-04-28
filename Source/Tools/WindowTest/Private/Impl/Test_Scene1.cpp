#include "stdafx.h"

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Test_Scene1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FGraphicsPipelineDesc pplnDesc1;
    pplnDesc1.AddShader(EShaderType::Vertex, EShaderLangFormat::VKSL_100, "main", R"#(
#version 450 core
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout (binding=0, std140) uniform un_ConstBuf
{
    mat4	mvp;
    mat4	projection;
    mat4	view;
    vec4	color;
    vec4	color1;
    vec4	color2;
    vec4	color3;

} ub;

layout(location=0) in  vec3	at_Position;
layout(location=1) in  vec2	at_Texcoord;

layout(location=0) out vec2	v_Texcoord;

void main() {
    gl_Position	= vec4( at_Position, 1.0 ) * ub.mvp;
    v_Texcoord	= at_Texcoord;
}
)#"
ARGS_IF_RHIDEBUG("Test_Scene1_Ppln1_VS"));
    pplnDesc1.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100, "main", R"#(
#version 450 core
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

layout (binding=0, std140) uniform un_ConstBuf
{
    mat4	mvp;
    mat4	projection;
    mat4	view;
    vec4	color;
    vec4	color1;
    vec4	color2;
    vec4	color3;

} ub;

layout (binding=1) uniform sampler2D un_ColorTexture;

layout(location=0) in  vec2	v_Texcoord;

layout(location=0) out vec4	out_Color;

void main() {
    out_Color = texture(un_ColorTexture, v_Texcoord) * ub.color;
}
)#"
ARGS_IF_RHIDEBUG("Test_Scene1_Ppln1_PS"));

    FGraphicsPipelineDesc pplnDesc2;
    pplnDesc2.AddShader(EShaderType::Vertex, EShaderLangFormat::VKSL_100, "main", R"#(
#version 450 core
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout (binding=0, std140) uniform un_ConstBuf
{
    mat4 mvp;
    mat4 projection;
    mat4 view;
    vec4 color;
    vec4 color1;
    vec4 color2;
    vec4 color3;

} ub;

layout(location=0) in  vec3	at_Position;
layout(location=1) in  vec2	at_Texcoord;

layout(location=0) out vec2	v_Texcoord;

void main() {
    gl_Position	= vec4( at_Position, 1.0 ) * ub.mvp;
    v_Texcoord	= at_Texcoord;
}
)#"
ARGS_IF_RHIDEBUG("Test_Scene1_Ppln2_VS"));
    pplnDesc2.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100, "main", R"#(
#version 450 core
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

layout (binding=0, std140) uniform un_ConstBuf
{
    mat4 mvp;
    mat4 projection;
    mat4 view;
    vec4 color;
    vec4 color1;
    vec4 color2;
    vec4 color3;

} ub;

layout (binding=1) uniform sampler2D un_ColorTexture;

layout(location=0) in  vec2	v_Texcoord;

void main() {
    if ( texture(un_ColorTexture, v_Texcoord).a * ub.color.a < 0.1f )
        discard;
}
)#"
ARGS_IF_RHIDEBUG("Test_Scene1_Ppln2_PS"));

    struct FVertex1 {
        float3 Position;
        short2 TexCoord;
    };

    const uint2 viewSize{ 1024, 1024 };

    const FDeviceProperties& properties = fg.DeviceProperties();

    const size_t cbufSize = 256_b;
    const size_t cbufOffset = Max(cbufSize, properties.MinUniformBufferOffsetAlignment);
    const size_t cbufAlignedSize = Meta::RoundToNext(cbufSize, cbufOffset);

    TAutoResource<FBufferID> constBuf1{ fg, fg.CreateBuffer(FBufferDesc{ cbufAlignedSize      , EBufferUsage::Uniform | EBufferUsage::TransferDst }, Default ARGS_IF_RHIDEBUG("ConstBuf1")) };
    TAutoResource<FBufferID> constBuf2{ fg, fg.CreateBuffer(FBufferDesc{ cbufAlignedSize * 2  , EBufferUsage::Uniform | EBufferUsage::TransferDst }, Default ARGS_IF_RHIDEBUG("ConstBuf2")) };
    TAutoResource<FBufferID> constBuf3{ fg, fg.CreateBuffer(FBufferDesc{ cbufAlignedSize      , EBufferUsage::Uniform | EBufferUsage::TransferDst }, Default ARGS_IF_RHIDEBUG("ConstBuf3")) };
    LOG_CHECK(WindowTest, constBuf1 && constBuf2 && constBuf3);

    TAutoResource<FBufferID> vertexBuf1{ fg, fg.CreateBuffer(FBufferDesc{ sizeof(FVertex1) * 3*1000, EBufferUsage::Vertex | EBufferUsage::TransferDst }, Default ARGS_IF_RHIDEBUG("VertexBuf1")) };
    TAutoResource<FBufferID> vertexBuf2{ fg, fg.CreateBuffer(FBufferDesc{ sizeof(FVertex1) * 3*2000, EBufferUsage::Vertex | EBufferUsage::TransferDst }, Default ARGS_IF_RHIDEBUG("VertexBuf2")) };
    LOG_CHECK(WindowTest, vertexBuf1 && vertexBuf2);

    TAutoResource<FImageID> texture1{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension({ 512, 512 })
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Sampled | EImageUsage::TransferDst),
        Default ARGS_IF_RHIDEBUG("Texture1")) };
    LOG_CHECK(WindowTest, !!texture1);
    TAutoResource<FImageID> texture2{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension({ 256, 512 })
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Sampled | EImageUsage::TransferDst),
        Default ARGS_IF_RHIDEBUG("Texture2")) };
    LOG_CHECK(WindowTest, !!texture2);
    TAutoResource<FSamplerID> sampler1{ fg, fg.CreateSampler(Default ARGS_IF_RHIDEBUG("Sampler1")) };

    const FVertexInputState vertexInput = FVertexInputState{}
        .Bind(""_vertexbuffer, sizeof(FVertex1))
        .Add("at_Position"_vertex, &FVertex1::Position)
        .Add("at_Texcoord"_vertex, &FVertex1::TexCoord, true);


    TAutoResource<FGPipelineID> pipeline1{ fg, fg.CreatePipeline(pplnDesc1 ARGS_IF_RHIDEBUG("Test_Scene1_Pipeline1")) };
    LOG_CHECK(WindowTest, !!pipeline1);
    TAutoResource<FGPipelineID> pipeline2{ fg, fg.CreatePipeline(pplnDesc2 ARGS_IF_RHIDEBUG("Test_Scene1_Pipeline2")) };
    LOG_CHECK(WindowTest, !!pipeline2);

    PPipelineResources resources = NEW_REF(RHIPipeline, FPipelineResources);
    LOG_CHECK(WindowTest, fg.InitPipelineResources(resources.get(), pipeline1, FDescriptorSetID{ "0" }));


    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("Test_Scene1")
        .SetDebugFlags(EDebugFlags::Default)) };
    LOG_CHECK(WindowTest, !!cmd);

    TAutoResource<FImageID> colorTarget{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(viewSize)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::ColorAttachment | EImageUsage::TransferSrc),
        Default ARGS_IF_RHIDEBUG("ColorTarget")) };
    LOG_CHECK(WindowTest, colorTarget.Valid());

    TAutoResource<FImageID> depthTarget{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension(viewSize)
        .SetFormat(EPixelFormat::Depth32f)
        .SetUsage(EImageUsage::DepthStencilAttachment),
        Default ARGS_IF_RHIDEBUG("DepthTarget")) };
    LOG_CHECK(WindowTest, depthTarget.Valid());


    FLogicalPassID depthPass = cmd->CreateRenderPass(FRenderPassDesc{ viewSize }
        .AddTarget(ERenderTargetID::Depth, depthTarget, FDepthStencilValue{Zero}, EAttachmentStoreOp::Store)
        .SetDepthTestEnabled(true).SetDepthWriteEnabled(true));
    LOG_CHECK(WindowTest, !!depthPass);
    FLogicalPassID opaquePass = cmd->CreateRenderPass(FRenderPassDesc{ viewSize }
        .AddTarget(ERenderTargetID::Color0, colorTarget, EAttachmentLoadOp::Load, EAttachmentStoreOp::Store)
        .AddTarget(ERenderTargetID::Depth, depthTarget, EAttachmentLoadOp::Load, EAttachmentStoreOp::Store)
        .SetDepthTestEnabled(true).SetDepthWriteEnabled(false));
    LOG_CHECK(WindowTest, !!opaquePass);
    FLogicalPassID transparentPass = cmd->CreateRenderPass(FRenderPassDesc{ viewSize }
        .AddTarget(ERenderTargetID::Color0, colorTarget, EAttachmentLoadOp::Keep, EAttachmentStoreOp::Store)
        .AddTarget(ERenderTargetID::Depth, depthTarget, EAttachmentLoadOp::Keep, EAttachmentStoreOp::Keep)
        .AddColorBuffer(ERenderTargetID::Color0, EBlendFactor::SrcAlpha, EBlendFactor::OneMinusSrcAlpha, EBlendOp::Add)
        .SetDepthTestEnabled(true).SetDepthWriteEnabled(false));
    LOG_CHECK(WindowTest, !!transparentPass);
    LOG_CHECK(WindowTest, depthPass != opaquePass);
    LOG_CHECK(WindowTest, opaquePass != transparentPass);
    LOG_CHECK(WindowTest, transparentPass != depthPass);


    {
        // Depth Pass
        PFrameTask tSubmitDepth{};
        {
            (*resources)
                .BindTexture("un_ColorTexture"_uniform, texture1, sampler1)
                .BindBuffer("un_ConstBuf"_uniform, constBuf2, 0_b, cbufSize);

            cmd->Task(depthPass, FDrawVertices{}
                .SetName("DrawDepth")
                .SetTopology(EPrimitiveTopology::TriangleList)
                .Draw(3*1000)
                .SetVertexInput(vertexInput)
                .AddVertexBuffer(""_vertexbuffer, vertexBuf1, 0_b)
                .SetPipeline(pipeline2)
                .AddResources("0"_descriptorset, resources));

            PFrameTask tUpdateBuf0 = cmd->Task(FUpdateBuffer{ constBuf2, 0_b, FRawData(256_b).MakeView() }
                .SetName("UpdateBuf0"));

            tSubmitDepth = cmd->Task(FSubmitRenderPass{ depthPass }
                .SetName("DepthOnlyPass")
                .DependsOn(tUpdateBuf0));
        }

        // Opaque Pass
        PFrameTask tSubmitOpaque{};
        {
            (*resources)
                .BindTexture("un_ColorTexture"_uniform, texture2, sampler1)
                .BindBuffer("un_ConstBuf"_uniform, constBuf2, cbufOffset, cbufSize);

            cmd->Task(opaquePass, FDrawVertices{}
                .SetName("Draw1_Opaque")
                .SetTopology(EPrimitiveTopology::TriangleList)
                .Draw(3*1000)
                .SetVertexInput(vertexInput)
                .AddVertexBuffer(""_vertexbuffer, vertexBuf1, 0_b)
                .SetPipeline(pipeline1)
                .AddResources("0"_descriptorset, resources));

            (*resources)
                .BindTexture("un_ColorTexture"_uniform, texture1, sampler1)
                .BindBuffer("un_ConstBuf"_uniform, constBuf2, 0_b, cbufSize);

            cmd->Task(opaquePass, FDrawVertices{}
                .SetName("Draw2_Opaque")
                .SetTopology(EPrimitiveTopology::TriangleList)
                .Draw(3*1000)
                .SetVertexInput(vertexInput)
                .AddVertexBuffer(""_vertexbuffer, vertexBuf2, 0_b)
                .SetPipeline(pipeline1)
                .AddResources("0"_descriptorset, resources));

            (*resources)
                .BindTexture("un_ColorTexture"_uniform, texture1, sampler1)
                .BindBuffer("un_ConstBuf"_uniform, constBuf1);

            cmd->Task(opaquePass, FDrawVertices{}
                .SetName("Draw0_Opaque")
                .SetTopology(EPrimitiveTopology::TriangleList)
                .Draw(3*2000)
                .SetVertexInput(vertexInput)
                .AddVertexBuffer(""_vertexbuffer, vertexBuf1, 0_b)
                .SetPipeline(pipeline1)
                .AddResources("0"_descriptorset, resources));

            PFrameTask tUpdateBuf1 = cmd->Task(FUpdateBuffer{ constBuf1, 0_b, FRawData(256_b).MakeView() }
                .SetName("UpdateBuf1"));
            PFrameTask tUpdateBuf2 = cmd->Task(FUpdateBuffer{ constBuf2, 256_b, FRawData(256_b).MakeView() }
                .SetName("UpdateBuf2"));

            tSubmitOpaque = cmd->Task(FSubmitRenderPass{ opaquePass }
                .SetName("OpaquePass")
                .DependsOn(tSubmitDepth, tUpdateBuf1, tUpdateBuf2));
        }

        // Transparent Pass
        PFrameTask tSubmitTransparent{};
        {
            (*resources)
                .BindTexture("un_ColorTexture"_uniform, texture2, sampler1)
                .BindBuffer("un_ConstBuf"_uniform, constBuf3);

            cmd->Task(transparentPass, FDrawVertices{}
                .SetName("Draw1_Transparent")
                .SetTopology(EPrimitiveTopology::TriangleList)
                .Draw(3*1000)
                .SetVertexInput(vertexInput)
                .AddVertexBuffer(""_vertexbuffer, vertexBuf2, 0_b)
                .SetPipeline(pipeline1)
                .AddResources("0"_descriptorset, resources));

            (*resources)
                .BindTexture("un_ColorTexture"_uniform, texture1, sampler1)
                .BindBuffer("un_ConstBuf"_uniform, constBuf2, 0_b, cbufSize);

            cmd->Task(transparentPass, FDrawVertices{}
                .SetName("Draw2_Transparent")
                .SetTopology(EPrimitiveTopology::TriangleList)
                .Draw(3*1000)
                .SetVertexInput(vertexInput)
                .AddVertexBuffer(""_vertexbuffer, vertexBuf1, 0_b)
                .SetPipeline(pipeline1)
                .AddResources("0"_descriptorset, resources));

            (*resources)
                .BindTexture("un_ColorTexture"_uniform, texture1, sampler1)
                .BindBuffer("un_ConstBuf"_uniform, constBuf2, cbufOffset, cbufSize);

            cmd->Task(transparentPass, FDrawVertices{}
                .SetName("Draw0_Transparent")
                .SetTopology(EPrimitiveTopology::TriangleList)
                .Draw(3*2000)
                .SetVertexInput(vertexInput)
                .AddVertexBuffer(""_vertexbuffer, vertexBuf2, 0_b)
                .SetPipeline(pipeline1)
                .AddResources("0"_descriptorset, resources));

            PFrameTask tUpdateBuf3 = cmd->Task(FUpdateBuffer{ constBuf3, 0_b, FRawData(256_b).MakeView() }
                .SetName("UpdateBuf3"));

            tSubmitTransparent = cmd->Task(FSubmitRenderPass{ transparentPass }
                .SetName("TransparentPass")
                .DependsOn(tSubmitOpaque, tUpdateBuf3));
        }
        Unused(tSubmitTransparent);

        // Present
        PFrameTask present = cmd->Task(FPresent{ app.RHI().Swapchain().Get(), colorTarget });
        LOG_CHECK(WindowTest, !!present);
    }

    LOG_CHECK(WindowTest, fg.Execute(cmd));
    LOG_CHECK(WindowTest, fg.WaitIdle());

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
