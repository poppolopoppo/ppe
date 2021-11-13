#include "stdafx.h"

#include "WindowTestApp.h"

#include "RHI/FrameGraph.h"
#include "RHI/ImageView.h"
#include "RHI/PipelineDesc.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformNotification.h"
#include "HAL/RHIService.h"
#include "Meta/Utility.h"

namespace PPE {
LOG_CATEGORY(, WindowTest)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern bool Test_Draw1_(FWindowTestApp& app);
extern bool Test_Draw2_(FWindowTestApp& app);
//----------------------------------------------------------------------------
FWindowTestApp::FWindowTestApp(const FModularDomain& domain)
:   parent_type(domain, "WindowTest", true) {

#if 0 //%_NOCOMMIT%
    FLogger::SetGlobalVerbosity(ELoggerVerbosity::None);
#endif
}
//----------------------------------------------------------------------------
FWindowTestApp::~FWindowTestApp() = default;
//----------------------------------------------------------------------------
void FWindowTestApp::Start() {
    parent_type::Start();

    Application::FPlatformNotification::NotifySystray(
        Application::FPlatformNotification::ENotificationIcon::Warning,
        L"HelloWorld!",
        L"WindowTest");

    auto launchTest = [this](FWStringView name, auto&& test) {
        UNUSED(name);
        LOG(WindowTest, Emphasis, L"start framegraph test <{0}> ...", name);
        RHI::IFrameGraph& fg = *RHI().FrameGraph();
        fg.PrepareNewFrame();
        if (Likely(test(*this))) {
            fg.LogFrame();
            LOG(WindowTest, Info, L"frame graph test <{0}> [PASSED]", name);
            return true;
        }
        LOG(WindowTest, Error, L"frame graph test <{0}> [FAILED]", name);
        return false;
    };

#define LAUNCH_TEST_(_Func) launchTest(WSTRINGIZE(_Func), _Func)

    static volatile bool enabled = true;
    if (enabled) {
        LAUNCH_TEST_(&Test_Draw1_);
        LAUNCH_TEST_(&Test_Draw2_);
    }

#undef LAUNCH_TEST_

    ApplicationLoop();
}
//----------------------------------------------------------------------------
void FWindowTestApp::Shutdown() {
    Application::FPlatformNotification::NotifySystray(
        Application::FPlatformNotification::ENotificationIcon::Warning,
        L"Here I go",
        L"WindowTest");

    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
bool Test_Draw1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FGraphicsPipelineDesc desc;
    desc.AddShader(EShaderType::Vertex, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) out vec3  v_Color;

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
    gl_Position = vec4( g_Positions[gl_VertexIndex], 0.0, 1.0 );
    v_Color = g_Colors[gl_VertexIndex];
}
)#"
ARGS_IF_RHIDEBUG("Test_Draw_VS"));
    desc.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) out vec4  out_Color;

layout(location=0) in  vec3  v_Color;

void main() {
    out_Color = vec4(v_Color, 1.0);
}
)#"
ARGS_IF_RHIDEBUG("Test_Draw_PS"));

    const uint2 viewSize{ 800, 600 };

    FImageID image = fg.CreateImage(
        FImageDesc{}.SetDimension(viewSize).SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::ColorAttachment | EImageUsage::TransferSrc),
        Default ARGS_IF_RHIDEBUG("Test_Draw1_RT"));
    LOG_CHECK(WindowTest, image.Valid());

    ON_SCOPE_EXIT([&]() {
        fg.ReleaseResources(image);
    });

    FGPipelineID ppln = fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Test_Draw1"));
    LOG_CHECK(WindowTest, ppln.Valid());

    ON_SCOPE_EXIT([&]() {
        fg.ReleaseResources(ppln);
    });

    bool imageReadbackValidated = false;
    const auto onLoaded = [&imageReadbackValidated](const FImageView& imageData) {
        const auto testPixel = [&imageData](float x, float y, const FRgba32f& color) -> bool {
            FRgba32f texel;
            imageData.Load(&texel, float3(x, y, 0));
            LOG_CHECK(WindowTest, DistanceSq(color, texel) < F_LargeEpsilon);
            return true;
        };

        imageReadbackValidated = true;

        imageReadbackValidated &= testPixel(0.00f, -0.49f, FLinearColor{ 1.0f, 0.0f, 0.0f, 1.0f });
        imageReadbackValidated &= testPixel(0.49f, 0.49f, FLinearColor{ 0.0f, 1.0f, 0.0f, 1.0f });
        imageReadbackValidated &= testPixel(-0.49f, 0.49f, FLinearColor{ 0.0f, 0.0f, 1.0f, 1.0f });

        imageReadbackValidated &= testPixel(0.00f, -0.51f, Zero);
        imageReadbackValidated &= testPixel(0.51f, 0.51f, Zero);
        imageReadbackValidated &= testPixel(-0.51f, 0.51f, Zero);
        imageReadbackValidated &= testPixel(0.00f, 0.51f, Zero);
        imageReadbackValidated &= testPixel(0.51f, -0.51f, Zero);
        imageReadbackValidated &= testPixel(-0.51f, -0.51f, Zero);
    };

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}.SetName("Test_Draw1").SetDebugFlags(EDebugFlags::Default)) };
    LOG_CHECK(WindowTest, !!cmd);

    FLogicalPassID renderPass = cmd->CreateRenderPass(FRenderPassDesc{ viewSize }
        .AddTarget(ERenderTargetID::Color0, *image, FLinearColor::Transparent(), EAttachmentStoreOp::Store)
        .AddViewport(viewSize));
    LOG_CHECK(WindowTest, !!renderPass);

    cmd->Task(renderPass, FDrawVertices{}.Draw(3).SetPipeline(*ppln).SetTopology(EPrimitiveTopology::TriangleList));

    const PFrameTask tDraw = cmd->Task(FSubmitRenderPass{ renderPass });
    const PFrameTask tRead = cmd->Task(FReadImage{}.SetImage(*image, int2{}, viewSize).SetCallback(onLoaded).DependsOn(tDraw));
    UNUSED(tRead);

    LOG_CHECK(WindowTest, fg.Execute(cmd));
    LOG_CHECK(WindowTest, fg.WaitIdle());
    LOG_CHECK(WindowTest, imageReadbackValidated);

    return true;
}

//----------------------------------------------------------------------------
bool Test_Draw2_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FGraphicsPipelineDesc desc;
    desc.AddShader(EShaderType::Vertex, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) out vec3  v_Color;

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
    gl_Position = vec4( g_Positions[gl_VertexIndex], 0.0, 1.0 );
    v_Color = g_Colors[gl_VertexIndex];
}
)#"
ARGS_IF_RHIDEBUG("Test_Draw_VS"));
    desc.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100, "main", R"#(
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) out vec4  out_Color;

layout(location=0) in  vec3  v_Color;

void main() {
    out_Color = vec4(v_Color, 1.0);
}
)#"
ARGS_IF_RHIDEBUG("Test_Draw_PS"));

    FGPipelineID ppln = fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Test_Draw2"));
    LOG_CHECK(WindowTest, ppln.Valid());

    ON_SCOPE_EXIT([&]() {
        fg.ReleaseResources(ppln);
    });

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}.SetName("Test_Draw2").SetDebugFlags(EDebugFlags::Default)) };
    LOG_CHECK(WindowTest, !!cmd);

    FRawImageID image = cmd->SwapchainImage(*app.RHI().Swapchain());
    const uint2 viewSize = fg.Description(image).Dimensions.xy;

    FLogicalPassID renderPass = cmd->CreateRenderPass(FRenderPassDesc{ viewSize }
        .AddTarget(ERenderTargetID::Color0, image, FLinearColor::Transparent(), EAttachmentStoreOp::Store)
        .AddViewport(viewSize));
    LOG_CHECK(WindowTest, !!renderPass);

    cmd->Task(renderPass, FDrawVertices{}.Draw(3).SetPipeline(*ppln).SetTopology(EPrimitiveTopology::TriangleList));

    const PFrameTask tDraw = cmd->Task(FSubmitRenderPass{ renderPass });
    UNUSED(tDraw);

    LOG_CHECK(WindowTest, fg.Execute(cmd));
    LOG_CHECK(WindowTest, fg.WaitIdle());

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
