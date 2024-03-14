// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "ShaderToyApp.h"

#include "ApplicationModule.h"
#include "RHIModule.h"
#include "TextureService.h"

#include "RHI/EnumToString.h"

#include "Texture/TextureSource.h"

#include "HAL/PlatformDialog.h"

#include "UI/Imgui.h"
#include "External/imgui/Public/imgui-internal.h"

#include "UI/Widgets/ConsoleWidget.h"
#include "UI/Widgets/FileDialogWidget.h"
#include "UI/Widgets/LogViewerWidget.h"
#include "UI/Widgets/MemoryUsageWidget.h"
#include "UI/Widgets/FrameRateOverlayWidget.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformDialog.h"
#include "HAL/PlatformMisc.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "Maths/Threefy.h"
#include "Memory/UniquePtr.h"
#include "VirtualFileSystem.h"

namespace PPE {
LOG_CATEGORY(, ShaderToy)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static CONSTEXPR const char GShaderToyApp_BufferLetters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static CONSTEXPR size_t GShaderToyApp_DebugNameCapacity = 128;
//----------------------------------------------------------------------------
} //!namepsace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FShaderToyApp::FShaderToyApp(FModularDomain& domain)
:   parent_type(domain, "Tools/ShaderToy", true) {

    FRHIModule& rhiModule = FRHIModule::Get(domain);
    rhiModule.SetStagingBufferSize(8_MiB);
}
//----------------------------------------------------------------------------
FShaderToyApp::~FShaderToyApp() = default;
//----------------------------------------------------------------------------
void FShaderToyApp::Start() {
    parent_type::Start();

    /*auto lang = TextEditor::LanguageDefinition::GLSL();

    _editor.create();
    _editor->SetLanguageDefinition(lang);*/

    PPE_LOG_CHECKVOID(ShaderToy, CreateDummySource_(*RHI().FrameGraph(), {64, 64}, RHI::EPixelFormat::RGBA8_UNorm));

    _vertexShader.AddShader(RHI::EShaderLangFormat::VKSL_100, "main", R"#(
        #pragma shader_stage(vertex)
        #extension GL_ARB_separate_shader_objects : enable
        #extension GL_ARB_shading_language_420pack : enable

        layout(location=0) out struct {
            vec2 vTexCoord;
        }   Out;

        void main() {
            Out.vTexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
            gl_Position = vec4(Out.vTexCoord * vec2(2, -2) + vec2(-1, 1), 0.0, 1.0 );
        }
        )#" ARGS_IF_RHIDEBUG("ShaderToy/VertexShader"));

    PPE_LOG_CHECKVOID(ShaderToy, _main.Construct(*this, L"Saved:/ShaderToy/main.glsl"));

    //STACKLOCAL_WTEXTWRITER(fragmentSource, GShaderToyApp_DebugNameCapacity);
    //forrange(i, 0, lengthof(_buffers)) {
    //    fragmentSource.Reset();
    //    Format(fragmentSource, L"Saved:/ShaderToy/Buffer{}.vksl", wchar_t(GShaderToyApp_BufferLetters[i]));

    //    PPE_LOG_CHECKVOID(ShaderToy, _buffers[i].Construct(*fg, fragmentSource.Written()));
    //}

    _shaderTime.ResetToZero();
    _shaderTimeSpeed = 1.0f;

    StartInterafaceWidgets_();

    ApplicationLoop();
}
//----------------------------------------------------------------------------
void FShaderToyApp::Shutdown() {
    _fileDialog.reset();
    _memoryUsage.reset();
    _logViewer.reset();

    _dockspaceBottom = _dockspaceLeft = Zero;

    _main.TearDown(*this);

    _buffers.clear_ReleaseMemory();
    _sources.clear_ReleaseMemory();

    _dummySource = Default;
    _vertexShader = Default;

    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
bool FShaderToyApp::FBuffer::Construct(const FShaderToyApp& app, const FFilename& fragmentSource) {
    FragmentSource = fragmentSource;
    DebugName = fragmentSource.Basename().ToString();

    if (not VFS_FileExists(FragmentSource)) {
        const FStringView defaultSource = MakeStringView(R"#(#version 450 core
layout(location = 0) out vec4 fragColor;

//layout(push_constant) uniform uPushConstant {
layout(set=0, binding=0, std140) uniform uPushConstant {
    float   iTime;                  // Current time in seconds
    float   iTimeDelta;             // Time it takes to render a frame, in seconds
    int     iFrame;                 // Current frame
    float   iFrameRate;             // Number of frames rendered per second
    vec4    iMouse;                 // xy = current pixel coords (if LMB is down). zw = click pixel
    vec4    iResolution;            // Width / Height / WidthOO / HeightOO
    vec4    iChannelResolution[4];  // Width / Height / WidthOO / HeightOO
}   Un;

layout(set=0, binding=1) uniform sampler2D iChannel0;
layout(set=0, binding=2) uniform sampler2D iChannel1;
layout(set=0, binding=3) uniform sampler2D iChannel2;
layout(set=0, binding=4) uniform sampler2D iChannel3;

layout(location = 0) in struct {
    vec2 TexCoord;
}   In;

void main() {
    vec3 col = 0.5 + 0.5*cos(Un.iTime+In.TexCoord.xyx+vec3(0,2,4));

    fragColor = vec4(col, 1.0);
}
)#");
        PPE_LOG_CHECK(ShaderToy, VFS_WriteAll(FragmentSource, MakeRawView(defaultSource), EAccessPolicy::Create_Text));
    }

    const RHI::SFrameGraph fg = app.RHI().FrameGraph();

    UniformBuffer.Reset(*fg, fg->CreateBuffer(RHI::FBufferDesc{
        sizeof(FPushConstantData),
        RHI::EBufferUsage::Uniform | RHI::EBufferUsage::TransferDst },
        Default ARGS_IF_RHIDEBUG(
            INLINE_FORMAT(GShaderToyApp_DebugNameCapacity, "ShaderToy/{}/UniformBuffer", DebugName))) );
    PPE_LOG_CHECK(ShaderToy, UniformBuffer);

    forrange(s, 0, lengthof(Inputs)) {
        RHI::FSamplerDesc desc;
        desc.SetFilter(RHI::ETextureFilter::Linear, RHI::ETextureFilter::Linear, RHI::EMipmapFilter::Linear);
        desc.SetAddressMode(RHI::EAddressMode::Repeat);
        desc.SetLodRange(-1000.f, 1000.f);

        FInput& in = Inputs[s];

        in.Sampler.Reset(*fg, fg->CreateSampler(desc ARGS_IF_RHIDEBUG(
            INLINE_FORMAT(GShaderToyApp_DebugNameCapacity, "ShaderToy/{}/Sampler_{}", DebugName, GShaderToyApp_BufferLetters[s]))) );
        PPE_LOG_CHECK(ShaderToy, in.Sampler.Valid());

        in.Source = app._dummySource;
    }

    PPE_LOG_CHECK(ShaderToy, RecreatePipeline(*fg, app._vertexShader));

    Resources = fg->CreatePipelineResources(*Pipeline, "0"_descriptorset);
    PPE_LOG_CHECK(ShaderToy, Resources.valid());

    return true;
}
//----------------------------------------------------------------------------
void FShaderToyApp::FBuffer::TearDown(const FShaderToyApp& app) {
    Unused(app);

    LastModified = Default;
    Resolution = Default;

    Resources.reset();

    Pipeline.Reset();
    RenderTarget.Reset();
    UniformBuffer.Reset();

    for (FInput& in : Inputs) {
        in.Sampler.Reset();
        in.Source.reset();
    }
}
//----------------------------------------------------------------------------
bool FShaderToyApp::FBuffer::RecreatePipeline(RHI::IFrameGraph& fg, const RHI::FGraphicsPipelineDesc::FShader& vertexShader) {
    FTimestamp lastModified;
    if (not VFS_FileLastModified(&lastModified, FragmentSource) || lastModified == LastModified)
        return Pipeline.Valid();

    LastModified = lastModified;

    RHI::FGraphicsPipelineDesc desc;
    desc.AddShader(RHI::EShaderType::Vertex, vertexShader);
    desc.AddShader(RHI::EShaderType::Fragment, RHI::EShaderLangFormat::VKSL_100, "main", FragmentSource);

    RHI::FGPipelineID newPipeline = fg.CreatePipeline(desc ARGS_IF_RHIDEBUG(
        INLINE_FORMAT(GShaderToyApp_DebugNameCapacity, "ShaderToy/{}/Pipeline", DebugName)));
    if (newPipeline.Valid())
        Pipeline.Reset(fg, std::move(newPipeline));

    return Pipeline.Valid();
}
//----------------------------------------------------------------------------
bool FShaderToyApp::FBuffer::RecreateRenderTarget(RHI::IFrameGraph& fg, const uint2& viewportSize) {
    if (Resolution == viewportSize)
        return true;

    RenderTarget.Reset(fg, fg.CreateImage(RHI::FImageDesc{}
        .SetDimension(viewportSize)
        .SetFormat(RHI::EPixelFormat::RGBA16f)
        .SetUsage(RHI::EImageUsage::ColorAttachment | RHI::EImageUsage::Sampled),
        Default ARGS_IF_RHIDEBUG(INLINE_FORMAT(GShaderToyApp_DebugNameCapacity, "ShaderToy/{}/RenderTarget", DebugName))) );

    PPE_LOG_CHECK(ShaderToy, RenderTarget.Valid());

    Resolution = viewportSize;
    return true;
}
//----------------------------------------------------------------------------
RHI::PFrameTask FShaderToyApp::FBuffer::UpdateUniformBuffer(const RHI::FCommandBufferBatch& cmd, const FPushConstantData& frameData) {
    FPushConstantData pc = frameData;

    pc.iResolution.xy = float2(Resolution);
    pc.iResolution.zw = Rcp(pc.iResolution.xy);

    forrange(i, 0, lengthof(Inputs)) {
        pc.iChannelResolution[i].xy = float2(Inputs[i].Source->Desc.Dimensions.xy);
        pc.iChannelResolution[i].zw = Rcp(pc.iResolution.xy);
    }

    Resources->BindBuffer("uPushConstant"_uniform, UniformBuffer);

    Resources->BindTexture("iChannel0"_uniform, Inputs[0].Source->Image, Inputs[0].Sampler);
    Resources->BindTexture("iChannel1"_uniform, Inputs[1].Source->Image, Inputs[1].Sampler);
    Resources->BindTexture("iChannel2"_uniform, Inputs[2].Source->Image, Inputs[2].Sampler);
    Resources->BindTexture("iChannel3"_uniform, Inputs[3].Source->Image, Inputs[3].Sampler);

    return cmd->Task(RHI::FUpdateBuffer{}
        .SetBuffer(UniformBuffer)
        .AddData(MakePodConstView(pc)) );
}
//----------------------------------------------------------------------------
RHI::PFrameTask FShaderToyApp::FBuffer::RenderFrame(const FShaderToyApp& app, const RHI::FCommandBufferBatch& cmd, const FPushConstantData& frameData) {
    const RHI::SFrameGraph fg = app.RHI().FrameGraph();

    if (not Pipeline.Valid() or app._needRefresh)
        PPE_LOG_CHECK(ShaderToy, RecreatePipeline(*fg, app._vertexShader));

    const RHI::FLogicalPassID renderPass = cmd->CreateRenderPass(RHI::FRenderPassDesc{uint2(Resolution)}
        .AddViewport(Resolution)
        .AddTarget(RHI::ERenderTargetID::Color0, *RenderTarget, FLinearColor::Transparent(), RHI::EAttachmentStoreOp::Store));

    RHI::FSubmitRenderPass submit{ renderPass };
    submit.DependsOn(UpdateUniformBuffer(cmd, frameData));

    for (const FInput& in: Inputs) {
        if (in.Source->OptionalBufferRef)
            submit.DependsOn(in.Source->OptionalBufferRef->LastTask);
    }

    cmd->Task(renderPass, RHI::FDrawVertices{}
        .SetPipeline(Pipeline)
        .AddResources("0"_descriptorset, Resources)
        .SetTopology(RHI::EPrimitiveTopology::TriangleList)
        .SetEnableDepthTest(false)
        .SetCullMode(RHI::ECullMode::None)
        .Draw(3));

    LastTask = cmd->Task(submit);
    return LastTask;
}
//----------------------------------------------------------------------------
bool FShaderToyApp::CreateDummySource_(RHI::IFrameGraph& fg, const uint2& size, RHI::EPixelFormat format) {
    using namespace RHI;

    _dummySource = NEW_REF(UserDomain, FSource);

    _dummySource->DebugName = "Dummy";
    _dummySource->Image.Reset(fg, fg.CreateImage(RHI::FImageDesc{}
        .SetDimension(size)
        .SetFormat(format)
        .SetAllMipmaps()
        .SetUsage(RHI::EImageUsage::Sampled | RHI::EImageUsage::TransferDst),
        Default ARGS_IF_RHIDEBUG("ShaderToy/Source/Dummy")) );
    PPE_LOG_CHECK(ShaderToy, _dummySource->Image.Valid());

    _dummySource->Desc = fg.Description(_dummySource->Image);

    RHI::FCommandBufferBatch cmd{ fg.Begin(RHI::FCommandBufferDesc{}
        .SetName("ShaderToyApp/Source/Dummy")) };
    PPE_LOG_CHECK(ShaderToy, cmd.Valid());

    RHI::PFrameTask tPrevious;
    for (u32 mip = 0; mip < _dummySource->Desc.MaxLevel; mip++) {
        const RHI::PFrameTask tClearMip = cmd->Task(RHI::FClearColorImage{}
            .SetImage(_dummySource->Image)
            .AddRange(FMipmapLevel{mip}, 1, 0_layer, 1)
            .Clear(FLinearColor{Pastelizer(float(mip)/_dummySource->Desc.MaxLevel)})
            .DependsOn(tPrevious));
        PPE_LOG_CHECK(ShaderToy, tClearMip.valid());

        tPrevious = tClearMip;
    }

    return fg.Execute(cmd);
}
//----------------------------------------------------------------------------
auto FShaderToyApp::CreateTextureSource_(RHI::IFrameGraph& fg, const FFilename& texturePath) -> PCSource {
    using namespace RHI;

    if (PSource found =_sources.GetIFP(texturePath))
        return found;

    const ITextureService& textures = Services().Get<ITextureService>();
    const Meta::TOptional<ContentPipeline::FTextureSource> img = textures.ImportTextureSource2D(texturePath);
    if (not img)
        return _dummySource;

    PPE_LOG_CHECK(ShaderToy, img->NumMips() > 0 && img->Width() > 0 && img->Height() > 0);

    EPixelFormat pixelFmt = Default;
    switch (img->Format()) {
    case ContentPipeline::ETextureSourceFormat::RGBA8:
        if (img->Gamma() == EGammaSpace::sRGB)
            pixelFmt = EPixelFormat::sRGB8_A8;
        else
            pixelFmt = EPixelFormat::RGBA8_UNorm;
        break;
    case ContentPipeline::ETextureSourceFormat::BGRA8:
        if (img->Gamma() == EGammaSpace::sRGB)
            pixelFmt = EPixelFormat::sBGR8_A8;
        else
            pixelFmt = EPixelFormat::BGR8_UNorm;
        break;
    case ContentPipeline::ETextureSourceFormat::BGRE8:
        pixelFmt = EPixelFormat::BGR8_UNorm;
        break;
    case ContentPipeline::ETextureSourceFormat::G16:
        pixelFmt = EPixelFormat::R16_UNorm;
        break;
    case ContentPipeline::ETextureSourceFormat::G8:
        pixelFmt = EPixelFormat::R8_UNorm;
        break;
    case ContentPipeline::ETextureSourceFormat::R16f:
        pixelFmt = EPixelFormat::R16f;
        break;
    case ContentPipeline::ETextureSourceFormat::RG16:
        pixelFmt = EPixelFormat::RG16_UNorm;
        break;
    case ContentPipeline::ETextureSourceFormat::RG8:
        pixelFmt = EPixelFormat::RG8_UNorm;
        break;
    case ContentPipeline::ETextureSourceFormat::RA16:
        pixelFmt = EPixelFormat::RG16_UNorm;
        break;
    case ContentPipeline::ETextureSourceFormat::RA8:
        pixelFmt = EPixelFormat::RG8_UNorm;
        break;
    case ContentPipeline::ETextureSourceFormat::RGBA16:
        pixelFmt = EPixelFormat::RGBA16_UNorm;
        break;
    case ContentPipeline::ETextureSourceFormat::RGBA16f:
        pixelFmt = EPixelFormat::RGBA16f;
        break;
    case ContentPipeline::ETextureSourceFormat::RGBA32f:
        pixelFmt = EPixelFormat::RGBA32f;
        break;
    default:
        AssertNotImplemented();
    }

    const RHI::TAutoResource<RHI::FImageID> stagingImg{ fg, fg.CreateImage(RHI::FImageDesc{}
        .SetUsage(RHI::EImageUsage_Transfer)
        .SetDimension(img->Dimensions().xy)
        .SetFormat(pixelFmt)
        .SetAllMipmaps(),
        Default ARGS_IF_RHIDEBUG(*ToString(texturePath))) };
    PPE_LOG_CHECK(ShaderToy, stagingImg->Valid());

    PSource source = NEW_REF(UserDomain, FSource);
    source->DebugName = texturePath.Basename().ToString();
    source->Image.Reset(fg, fg.CreateImage(RHI::FImageDesc{}
        .SetUsage(RHI::EImageUsage::Sampled | RHI::EImageUsage::TransferDst)
        .SetDimension(img->Dimensions().xy)
        .SetFormat(pixelFmt)
        .SetAllMipmaps(),
        Default ARGS_IF_RHIDEBUG(*ToString(texturePath)) ));
    PPE_LOG_CHECK(ShaderToy, source->Image.Valid());

    source->Desc = fg.Description(source->Image);

    RHI::FCommandBufferBatch cmd{ fg.Begin(RHI::FCommandBufferDesc{}
        .SetName("ShaderToyApp/Source/CreateTexture")) };
    PPE_LOG_CHECK(ShaderToy, cmd.Valid());

    const ContentPipeline::FTextureSource::FReaderScope imgReader{*img};

    const RHI::PFrameTask tUpdate = cmd->Task(RHI::FUpdateImage{}
        .SetImage(*stagingImg, int3(0), 0_mipmap)
        .SetData(imgReader.MipData(0), img->Dimensions().xy));
    PPE_LOG_CHECK(ShaderToy, tUpdate.valid());

    const RHI::PFrameTask tMipMaps = cmd->Task(RHI::FGenerateMipmaps{}
        .SetImage(*stagingImg)
        .SetMipmaps(0, source->Desc.MaxLevel - 1)
        .DependsOn(tUpdate) );
    PPE_LOG_CHECK(ShaderToy, tMipMaps.valid());

    RHI::PFrameTask tPrevious = tMipMaps;
    uint3 mipDimensions = source->Desc.Dimensions;
    for (RHI::FMipmapLevel mip{0}; mip < source->Desc.MaxLevel; ) {
        auto copyImage = RHI::FCopyImage{}
            .From(*stagingImg)
            .To(source->Image)
            .DependsOn(tPrevious);

        const RHI::FMipmapLevel lastMip{Min(mip + (RHI::MaxCopyRegions), source->Desc.MaxLevel.Value)};
        for (; mip < lastMip; mip.Value++) {
            copyImage.AddRegion({mip}, int2(0), {mip}, int2(0), mipDimensions.xy);
            mipDimensions = RHI::FPixelFormatInfo::NextMipDimensions(mipDimensions);
        }

        const RHI::PFrameTask tCopy = cmd->Task(copyImage);
        PPE_LOG_CHECK(ShaderToy, tCopy.valid());

        mip.Value = lastMip;
        tPrevious = tCopy;
    }

    PPE_LOG_CHECK(ShaderToy, fg.Execute(cmd));

    _sources.insert_or_assign({ texturePath, source });
    return source;
}
//----------------------------------------------------------------------------
auto FShaderToyApp::CreateTextureSource_(RHI::IFrameGraph& fg, const ContentPipeline::PTexture& texture) -> PCSource {
    using namespace RHI;

    PSource source = NEW_REF(UserDomain, FSource);
    if (const Meta::TOptional<FFilename> sourceFile = texture->Data().SourceFile())
        source->DebugName = sourceFile->Basename().ToString();

    source->Image.Reset(fg, texture->CreateTextureRHI(fg));
    PPE_LOG_CHECK(ShaderToy, source->Image.Valid());

    source->Desc = fg.Description(source->Image);

    _sources.insert_or_assign({ *texture->Data().SourceFile(), source });
    return source;
}
//----------------------------------------------------------------------------
bool FShaderToyApp::RecreateRenderTargets_(RHI::IFrameGraph& fg, const uint2& viewportSize) {
    for(const PBuffer& buf : _buffers) {
        if (not buf->RecreateRenderTarget(fg, viewportSize))
            return false;
    }

    return _main.RecreateRenderTarget(fg, viewportSize);
}
//----------------------------------------------------------------------------
void FShaderToyApp::StartInterafaceWidgets_() {
    _importTexture = MakeUnique<Application::FImportTextureWidget>(Services().Get<ITextureService>());

    _fileDialog = MakeUnique<Application::FFileDialogWidget>();
    _fileDialog->InitialDirectory = L"Data:/Textures";
    _fileDialog->ViewMode = Application::FFileDialogWidget::EView::Details;

    _memoryUsage = MakeUnique<Application::FMemoryUsageWidget>();

#if USE_PPE_LOGGER
    _logViewer = MakeUnique<Application::FLogViewerWidget>();
    _logViewer->RegisterLogger(true);
#endif

    _OnApplicationTick.Emplace([consoleWidget(MakeUnique<Application::FConsoleWidget>())](const IApplicationService&, FTimespan) {
        Unused(consoleWidget->Show());
    });

    _OnApplicationTick.Emplace([frameRateOverlay(MakeUnique<Application::FFrameRateOverlayWidget>(this))](const IApplicationService&, FTimespan) {
        Unused(frameRateOverlay->Show());
    });
}
//----------------------------------------------------------------------------
void FShaderToyApp::Update(FTimespan dt) {
    parent_type::Update(dt);

    _shaderTime.Tick(RealTime(), _shaderTimeSpeed);

    FTimespan elapsedSinceRefresh;
    _needRefresh = _refreshCooldown.Tick_Every(FSeconds(1), elapsedSinceRefresh);

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar;

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        if (_dockspaceLeft == Zero || _dockspaceBottom == Zero) {
            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            // split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
            //   window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
            //                                                              out_id_at_dir is the id of the node in the direction we specified earlier, out_id_at_opposite_dir is in the opposite direction
            _dockspaceLeft = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
            _dockspaceBottom = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.3f, nullptr, &dockspace_id);

            // we now dock our windows into the docking node we made above
            ImGui::DockBuilderDockWindow(_fileDialog->Title.c_str(), _dockspaceBottom);
            ImGui::DockBuilderDockWindow(_memoryUsage->Title.c_str(), _dockspaceBottom);
#if USE_PPE_LOGGER
            ImGui::DockBuilderDockWindow(_logViewer->Title.c_str(), _dockspaceBottom);
#endif

            // main window
            ImGui::DockBuilderDockWindow("ShaderToy", dockspace_id);

            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    if (ImGui::BeginMainMenuBar()) {
        DEFERRED{ ImGui::EndMainMenuBar(); };

        if (ImGui::BeginMenu("Show")) {
            DEFERRED{ ImGui::EndMenu(); };

            if (ImGui::BeginMenu("ImGui")) {
                DEFERRED{ ImGui::EndMenu(); };
                ImGui::MenuItem("Demo Window", nullptr, &bImGuiShowDemoWindow);
                ImGui::MenuItem("User Guide", nullptr, &bImGuiShowUserGuide);
            }

            if (ImGui::BeginMenu("ImPlot")) {
                DEFERRED{ ImGui::EndMenu(); };
                //ImGui::MenuItem("Demo Window", nullptr, &bImPlotShowDemoWindow);
                ImGui::MenuItem("User Guide", nullptr, &bImPlotShowUserGuide);
            }
        }
    }

    ImGui::End();

    Unused(_importTexture->Show());

    Unused(_fileDialog->Show());
    Unused(_memoryUsage->Show());
#if USE_PPE_LOGGER
    Unused(_logViewer->Show());
#endif

    if (bImGuiShowDemoWindow)
        ImGui::ShowDemoWindow();
    if (bImGuiShowUserGuide)
        ImGui::ShowUserGuide();

    //if (bImPlotShowDemoWindow)
    //    ImPlot::ShowDemoWindow();
    if (bImPlotShowUserGuide)
        ImPlot::ShowUserGuide();
}
//----------------------------------------------------------------------------
void FShaderToyApp::FBuffer::RenderUI(FShaderToyApp& app, RHI::IFrameGraph& fg) {
    char ansiText[FileSystem::MaxPathLength];
    FragmentSource.ToCStr(ansiText);

    if (ImGui::Button(ICON_CI_EDIT)) {
        const FWString nativePath = VFS().Unalias(FragmentSource);
        FPlatformMisc::ExternalTextEditor(*nativePath);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_CI_REFRESH)) {
        PPE_LOG_CHECKVOID(ShaderToy, RecreatePipeline(fg, app._vertexShader));
    }
    ImGui::SameLine();

    ImGui::PushFont(ImGui::LargeFont());
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(ansiText);
    ImGui::PopFont();

    forrange(i, 0, lengthof(Inputs)) {
        FInput& in = Inputs[i];

        if (not ImGui::CollapsingHeader(INLINE_FORMAT(64, "Input {}", GShaderToyApp_BufferLetters[i]), ImGuiTreeNodeFlags_DefaultOpen))
            continue;

        ImGui::PushID(&in);
        DEFERRED{ ImGui::PopID(); };

        if (ImGui::ImageButton(
            ICON_CI_BROWSER,
            FImTexturePackedID{ in.Source->Image->Pack().Packed },
            ImVec2(6, 6) * ImGui::GetTextLineHeightWithSpacing() - ImGui::GetStyle().ItemSpacing * 2,
            ImVec2(0, 0), ImVec2(1, 1),
            ImVec4(1, 1, 1, 1),
            ImGui::GetStyleColorVec4(ImGuiCol_Border))) {
            ImGui::OpenPopup("ShaderToy##Buffer##Input##Popup");
            ImGui::SetNextWindowPos(ImGui::GetItemRectMin() + ImVec2(0, ImGui::GetItemRectSize().y));
        }

        if (ImGui::BeginPopup("ShaderToy##Buffer##Input##Popup")) {
            DEFERRED{ ImGui::EndPopup(); };

            ImGui::BeginChild("ShaderToy##Buffer##Input##Popup##Sources", {200,100}, ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_AlwaysVerticalScrollbar);
            for (const auto& [filename, source] : app._sources) {
                ImGui::PushID(source.get());
                DEFERRED{ ImGui::PopID(); };

                const ImVec2 cursorPos = ImGui::GetCursorPos();

                if (ImGui::Selectable("##SourceTexture")) {
                    in.Source = source;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SetCursorPos(cursorPos);

                ImGui::Image(FImTexturePackedID{ source->Image->Pack().Packed }, ImVec2(1, 1) * ImGui::GetTextLineHeight());
                ImGui::SameLine();

                ImGui::TextUnformatted(source->DebugName.c_str());
            }
            ImGui::EndChild();

            if (ImGui::Button(ICON_FK_UNDO)) {
                in.Source = app._dummySource;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();

            ImGui::BeginDisabled(app._fileDialog->SelectedEntries.empty());
            if (ImGui::Button(ICON_CI_ADD)) {
                for (u32 entryIndex : app._fileDialog->SelectedEntries) {
                    app._importTexture->Import(app._fileDialog->VisibleEntries[entryIndex].Name,
                        [&](const ContentPipeline::PTexture& texture) {
                            in.Source = app.CreateTextureSource_(*app.RHI().FrameGraph(), texture);
                        },
                        [&, fname{app._fileDialog->VisibleEntries[entryIndex].Name}](FStringLiteral reason) {
                            FPlatformDialog::Show(
                                StringFormat(L"{}: {}", fname, reason),
                                L"failed to import texture"_view,
                                FPlatformDialog::kOkCancel,
                                FPlatformDialog::Error);
                        });
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        ImGui::BeginGroup();

        const RHI::FImageDesc& desc = in.Source->Desc;

        const RHI::FPixelFormatInfo pixelInfo = EPixelFormat_Infos(desc.Format);

        ImGui::BeginDisabled();

        ImGui::SmallButton(INLINE_FORMAT(16, "{}", desc.View));

        ImGui::SameLine();
        ImGui::SmallButton(INLINE_FORMAT(16, "{}", desc.Format));

        ImGui::SameLine();
        ImGui::SmallButton(INLINE_FORMAT(16, "{} bpp", pixelInfo.BitsPerPixel(RHI::EImageAspect::Color)));

        ImGui::SameLine();
        ImGui::SmallButton(INLINE_FORMAT(16, "{}x{}", pixelInfo.BlockDim.x, pixelInfo.BlockDim.y));

        if (pixelInfo.IsSRGB()) {
            ImGui::SameLine();
            ImGui::SmallButton("sRGB");
        }

        uint3 dimensions = desc.Dimensions;
        ImGui::SetNextItemWidth(120);
        ImGui::InputScalarN("Dimensions", ImGuiDataType_U32, &dimensions, 3);

        u32 numMips = Min(pixelInfo.FullMipCount(desc.Dimensions), *desc.MaxLevel);
        ImGui::SetNextItemWidth(120);
        ImGui::InputScalar("Mips", ImGuiDataType_U32, &numMips);

        u32 numSlices = *desc.ArrayLayers;
        ImGui::SetNextItemWidth(120);
        ImGui::InputScalar("Layers", ImGuiDataType_U32, &numSlices);

        ImGui::EndDisabled();

        ImGui::TextUnformatted(INLINE_FORMAT(32, "Memory size: {:f3}",
            Fmt::SizeInBytes(pixelInfo.SizeInBytes(
                RHI::EImageAspect::Color,
                desc.Dimensions,
                numMips,
                *desc.ArrayLayers))));

        ImGui::EndGroup();
    }
}
//----------------------------------------------------------------------------
void FShaderToyApp::Render(RHI::IFrameGraph& fg, FTimespan dt) {
    parent_type::Render(fg, dt);

    DEFERRED{ ImGui::End(); };
    bool visible = true;
    if (not ImGui::Begin("ShaderToy", &visible, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse))
        return;

    { // options
        ImGui::BeginChild("ShaderToy##Options", ImVec2(150, 0), ImGuiChildFlags_ResizeX);
        DEFERRED{ ImGui::EndChild(); };

        if (ImGui::BeginTabBar("ShaderToy##Options##TabBar")) {
            DEFERRED{ ImGui::EndTabBar(); };

            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 3, 5 });
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 3, 5 });
            DEFERRED{ ImGui::PopStyleVar(3); };

            if (ImGui::BeginTabItem(*_main.DebugName, &_main.bVisible, ImGuiTabItemFlags_NoCloseButton)) {
                ImGui::PushID(&_main);
                DEFERRED{
                    ImGui::PopID();
                    ImGui::EndTabItem();
                };
                _main.RenderUI(*this, fg);
            }

            for (const PBuffer& pBuf : _buffers) {
                if (ImGui::BeginTabItem(*pBuf->DebugName, &pBuf->bVisible)) {
                    ImGui::PushID(pBuf.get());
                    DEFERRED{
                        ImGui::PopID();
                        ImGui::EndTabItem();
                    };
                    pBuf->RenderUI(*this, fg);
                }
            }
        }
    }

    ImGui::SameLine();
    ImGui::BeginGroup();
    DEFERRED{ ImGui::EndGroup(); };

    float2 mouseCursor;
    uint2 viewportSize;

    { // viewport
        ImGui::BeginChild("ShaderToy##Viewport",
            ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), // Leave room for 1 line below us
            ImGuiChildFlags_Border,
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse);
        DEFERRED{ ImGui::EndChild(); };

        viewportSize = RoundToUnsigned(ImGui::GetContentRegionAvail().ToFloat2());
        PPE_LOG_CHECKVOID(ShaderToy, RecreateRenderTargets_(fg, viewportSize));

        ImGui::Image(
            FImTexturePackedID{ _main.RenderTarget->Pack().Packed },
            float2(viewportSize));

        mouseCursor = Clamp(ImGui::GetMousePos().ToFloat2(), ImGui::GetItemRectMin().ToFloat2(), ImGui::GetItemRectMax().ToFloat2()) - ImGui::GetItemRectMin();

        FPushConstantData frameData;
        frameData.iTime = static_cast<float>(*FSeconds(_shaderTime.Total()));
        frameData.iTimeDelta = static_cast<float>(*FSeconds(dt));
        frameData.iFrame = fg.CurrentFrameIndex();
        frameData.iFrameRate = static_cast<float>(*FSeconds(TickRate()));
        frameData.iMouse = float4(-1);

        if (ImGui::IsItemHovered()) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                frameData.iMouse.xy = mouseCursor;
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                frameData.iMouse.zw = mouseCursor;
        }

        RHI::FCommandBufferBatch cmd = fg.Begin(RHI::FCommandBufferDesc{RHI::EQueueType::Graphics}.SetName("ShaderToy"));
        PPE_LOG_CHECKVOID(ShaderToy, !!cmd);

        for (const PBuffer& buf : _buffers)
            Unused(buf->RenderFrame(*this, cmd, frameData));

        Unused(_main.RenderFrame(*this, cmd, frameData));

        PPE_LOG_CHECKVOID(ShaderToy, fg.Execute(cmd));
    }
    { // Bottom
        ImGui::BeginChild("ShaderToy##Bottom", ImVec2(0, ImGui::GetFrameHeightWithSpacing()));
        DEFERRED{ ImGui::EndChild(); };

        if (ImGui::Button(_shaderTimeSpeed > 0 ? ICON_FK_PAUSE : ICON_FK_PLAY))
            _shaderTimeSpeed = (_shaderTimeSpeed == 0 ? 1.f : 0.f);
        ImGui::SameLine();

        if (ImGui::Button(ICON_FK_STOP))
            _shaderTime.ResetToZero();
        ImGui::SameLine();

        //if (ImGui::Button(_shaderTimeSpeed < 0 ? ICON_FK_FAST_BACKWARD : ICON_FK_BACKWARD))
        //    _shaderTimeSpeed = (_shaderTimeSpeed < 0 ? 1.f : (_shaderTimeSpeed == 0 ? 1.f : -0.3f));
        //ImGui::SameLine();

        if (ImGui::Button(_shaderTimeSpeed > 1 ? ICON_FK_FAST_FORWARD : ICON_FK_FORWARD))
            _shaderTimeSpeed = (_shaderTimeSpeed > 1 ? 1.f : (_shaderTimeSpeed == 0 ? 1.f : 3.f));
        ImGui::SameLine();

        ImGui::PushItemWidth(70);
        ImGui::LabelText("##Clock", ICON_FK_CLOCK_O " %07.2fs", FSeconds(_shaderTime.Total()).Value());
        ImGui::SameLine();

        ImGui::Text(ICON_FK_WINDOW_MAXIMIZE " %d x %d", viewportSize.x, viewportSize.y);
        ImGui::SameLine();

        ImGui::Text(ICON_FK_MOUSE_POINTER  " %.1f x %.1f", mouseCursor.x, mouseCursor.y);
        ImGui::PopItemWidth();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
