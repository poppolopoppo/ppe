// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "VoxelCubeApp.h"

#include "MeshBuilderService.h"
#include "RHIModule.h"

#include "Mesh/Format/WaveFrontObj.h"
#include "Mesh/GeometricPrimitives.h"
#include "Mesh/GenericMeshHelpers.h"

#include "UI/ImGui.h"
#include "UI/Widgets/FrameRateOverlayWidget.h"

#include "Input/InputService.h"
#include "Window/MainWindow.h"
#include "Window/WindowService.h"

#include "Diagnostic/FeedbackContext.h"
#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "Maths/Threefy.h"

namespace PPE {
LOG_CATEGORY(, VoxelCube)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVoxelCubeApp::FVoxelCubeApp(FModularDomain& domain)
:   parent_type(domain, "Tools/VoxelCube", true) {

    FRHIModule& rhiModule = FRHIModule::Get(domain);
    rhiModule.SetStagingBufferSize(8_MiB);
}
//----------------------------------------------------------------------------
FVoxelCubeApp::~FVoxelCubeApp() = default;
//----------------------------------------------------------------------------
void FVoxelCubeApp::Start() {
    FFeedbackProgressBar marquee("Starting application ..."_view);
    Meta::Unused(marquee);

    parent_type::Start();

    const Application::PMainWindow mainWindow{ Window().MainWindow() };

    _camera = NEW_REF(UserDomain, Application::FCamera);
    _viewport = NEW_REF(UserDomain, Application::FViewportClient, _camera, mainWindow);

    _vertexInput.Bind(Default, sizeof(FVertexData));
    _vertexInput.Add(RHI::EVertexID::Position, RHI::EVertexFormat::Float3, Meta::StandardLayoutOffset(&FVertexData::Position));
    _vertexInput.Add(RHI::EVertexID::Color, RHI::EVertexFormat::UByte4_Norm, Meta::StandardLayoutOffset(&FVertexData::Color));
    _vertexInput.Add(RHI::EVertexID::Normal, RHI::EVertexFormat::Float3/*RHI::EVertexFormat::UByte2_Norm*/, Meta::StandardLayoutOffset(&FVertexData::Normal));

    _cameraInputs = NEW_REF(UserDomain, Application::FInputMapping, "CameraInputs");
    _cameraInputs->MapAll(_freeLookCamera);
    Input().AddInputMapping(_cameraInputs, -1);

    _OnApplicationTick.Emplace([frameRateOverlay(MakeUnique<Application::FFrameRateOverlayWidget>(this))](const IApplicationService&, FTimespan) {
        Unused(frameRateOverlay->Show());
    });

    marquee.Print("Reloading assets content..."_view);

    PPE_LOG_CHECKVOID(VoxelCube, ReloadContent_(*RHI().FrameGraph()));
}
//----------------------------------------------------------------------------
void FVoxelCubeApp::Run() {
    parent_type::Run();

    ApplicationLoop();
}
//----------------------------------------------------------------------------
void FVoxelCubeApp::Shutdown() {
    Input().RemoveInputMapping(_cameraInputs);

    _colorRT.Reset();
    _depthRT.Reset();
    _graphicsPpln.Reset();
    _indexBuffer.Reset();
    _vertexBuffer.Reset();
    _uniformBuffer.Reset();

    _cameraInputs.reset();
    _resources.reset();
    _vertexInput.Clear();

    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
void FVoxelCubeApp::Render(RHI::IFrameGraph& fg, FTimespan dt) {
    parent_type::Render(fg, dt);

    _viewport->Update(dt, _freeLookCamera);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::MenuItem("Reload"))
            ReloadContent_(fg);

        ImGui::MenuItem("Recompute normals", nullptr, &_bRecomputeNormals);

        ImGui::EndMainMenuBar();
    }

    FUniformData uniformData;
    uniformData.View = _viewport->Camera()->View();
    uniformData.Projection = _viewport->Camera()->Projection();
    uniformData.ViewProjection = _viewport->Camera()->ViewProjection();
    uniformData.InvertViewProjection = _viewport->Camera()->InvertViewProjection();

    _resources->BindBuffer("uCamera"_uniform, _uniformBuffer);

    using namespace RHI;

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("VoxelCube/MainPass")) };
    PPE_LOG_CHECKVOID(VoxelCube, !!cmd);

    const FRawImageID swapchainImage = cmd->SwapchainImage(RHI().Swapchain());
    PPE_LOG_CHECKVOID(VoxelCube, !!swapchainImage);

    const RHI::FSwapchainDesc& swapchainDesc = fg.Description(RHI().Swapchain());
    PPE_LOG_CHECKVOID(VoxelCube, RecreateRenderTarget_(fg, swapchainDesc.Dimensions));

    const uint2 viewSize(_viewport->ClientRect().Extents());

    FLogicalPassID renderPass = cmd->CreateRenderPass(FRenderPassDesc{ viewSize }
        .AddTarget(ERenderTargetID::Color0, swapchainImage, FLinearColor::Transparent(), EAttachmentStoreOp::Store)
        .AddTarget(ERenderTargetID::Depth, _depthRT, FDepthValue{}, EAttachmentStoreOp::Store)
        .AddViewport(viewSize)
        .AddColorBuffer(ERenderTargetID::Color0, EBlendFactor::SrcAlpha, EBlendFactor::OneMinusSrcAlpha, EBlendOp::Add)
        .SetDepthTestEnabled(true)
        .SetDepthWriteEnabled(true));
    PPE_LOG_CHECKVOID(VoxelCube, !!renderPass);

    PFrameTask tUpdateUniform = cmd->Task(FUpdateBuffer{}
        .SetBuffer(_uniformBuffer)
        .AddData(MakePodConstView(uniformData)) );

    FSubmitRenderPass submit{ renderPass };
    submit.DependsOn(tUpdateUniform);

    cmd->Task(renderPass, FDrawIndexed{}
        .SetPipeline(*_graphicsPpln)
        .SetTopology(EPrimitiveTopology::TriangleList)
        .AddResources("0"_descriptorset, _resources)
        .SetIndexBuffer(_indexBuffer, 0, IndexAttrib<u32>())
        .AddVertexBuffer(Default, _vertexBuffer)
        .SetVertexInput(_vertexInput)
        .SetEnableDepthTest(true)
        .SetCullMode(ECullMode::Back)
        .Draw(_genericMesh.IndexCount()));

    Unused(cmd->Task(submit));

    PPE_LOG_CHECKVOID(VoxelCube, fg.Execute(cmd));
}
//----------------------------------------------------------------------------
bool FVoxelCubeApp::ReloadContent_(RHI::IFrameGraph& fg) {
    PPE_LOG_CHECK(VoxelCube, CreateUniformBuffers_(fg));
    PPE_LOG_CHECK(VoxelCube, CreateGraphicsPipeline_(fg));
    PPE_LOG_CHECK(VoxelCube, CreateMeshBuffers_(fg));
    return true;
}
//----------------------------------------------------------------------------
bool FVoxelCubeApp::CreateMeshBuffers_(RHI::IFrameGraph& fg) {
    _genericMesh.Clear();

    ContentPipeline::FPositions3f positions0 = _genericMesh.Position3f(0);
    ContentPipeline::FColors4f colors0 = _genericMesh.Color4f(0);

#if 0
    ContentPipeline::FTexcoords3f texcoords0 = _genericMesh.Texcoord3f(0);

    Icosahedron(_genericMesh, positions0, texcoords0, float4x4::Identity());

    const TMemoryView<const float3> texcoordData = texcoords0.MakeView();
    const TMemoryView<float4> colorData = colors0.Resize(texcoords0.size());

    forrange(v, 0, texcoordData.size()) {
        const FLinearColor col{ HSL_to_RGB(0.5 + texcoordData[v] * 0.5) };
        colorData[v] = col.LinearToGamma(EGammaSpace::sRGB);
    }

    ComputeNormals(_genericMesh, positions0, _genericMesh.Normal3f(0));

    _genericMesh.CleanAndOptimize();
#else
    IMeshBuilderService& meshBuilder = Services().Get<IMeshBuilderService>();

    ContentPipeline::FMeshBuilderSettings settings;
    settings.EncodeTangentSpaceToQuaternion = false;
    settings.OptimizeIndicesOrder = true;
    settings.OptimizeVerticesOrder = true;
    settings.RecomputeNormals = ContentPipeline::ERecomputeMode::IfMissing;
    settings.RecomputeTangentSpace = ContentPipeline::ERecomputeMode::Remove;
    settings.RemoveUnusedVertices = true;

    ContentPipeline::FMeshBuilderResult result = meshBuilder.ImportGenericMesh(L"Data:/Models/xyzrgb_statuette_50K_voro10.ply", settings);
    PPE_LOG_CHECK(VoxelCube, result.has_value());

    _genericMesh = std::move(result.value());

    if (not _genericMesh.Color4f_IFP(0)) {
        const TMemoryView<float4> colorData = colors0.Resize(positions0.size());
        forrange(v, 0, colorData.size()) {
            colorData[v] = float4(float3(0.5f), 1.f);
        }
    }
#endif

    RAWSTORAGE(UserDomain, u8) indexData;
    PPE_LOG_CHECK(VoxelCube, _genericMesh.ExportIndices(RHI::EIndexFormat::UInt, indexData));

    RAWSTORAGE(UserDomain, u8) vertexData;
    PPE_LOG_CHECK(VoxelCube, _genericMesh.ExportVertices(_vertexInput, Default, vertexData));

    using namespace RHI;

    TAutoResource<FBufferID> indexBuf{ fg, fg.CreateBuffer(FBufferDesc{
        indexData.SizeInBytes(), EBufferUsage::TransferDst | EBufferUsage::Index },
        Default ARGS_IF_RHIDEBUG("ImGui/IndexBuffer") ) };
    PPE_LOG_CHECK(VoxelCube, indexBuf);

    TAutoResource<FBufferID> vertexBuf{ fg, fg.CreateBuffer(FBufferDesc{
        vertexData.SizeInBytes(), EBufferUsage::TransferDst | EBufferUsage::Vertex },
        Default ARGS_IF_RHIDEBUG("ImGui/VertexBuffer") ) };
    PPE_LOG_CHECK(VoxelCube, vertexBuf);

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("VoxelCube/UpdateMesh")) };
    PPE_LOG_CHECK(VoxelCube, !!cmd);

    Unused(cmd->Task(FUpdateBuffer{}
            .SetBuffer(indexBuf)
            .AddData(indexData.data(), indexData.SizeInBytes(), 0)));

    Unused(cmd->Task(FUpdateBuffer{}
        .SetBuffer(vertexBuf)
        .AddData(vertexData.data(), vertexData.SizeInBytes(), 0)));

    PPE_LOG_CHECK(VoxelCube, fg.Execute(cmd));

    _indexBuffer = std::move(indexBuf);
    _vertexBuffer = std::move(vertexBuf);
    return true;
}
//----------------------------------------------------------------------------
bool FVoxelCubeApp::RecreateRenderTarget_(RHI::IFrameGraph& fg, const uint2& viewportSize) {
    fg.RecreateResourceIFN(_colorRT, std::move(RHI::FImageDesc{}
        .SetDimension(viewportSize)
        .SetFormat(RHI::EPixelFormat::RGBA16f)
        .SetUsage(RHI::EImageUsage::ColorAttachment | RHI::EImageUsage::Sampled)),
        Default ARGS_IF_RHIDEBUG("VoxelCube/ColorRT") );
    PPE_LOG_CHECK(VoxelCube, _colorRT.Valid());

    fg.RecreateResourceIFN(_depthRT, std::move(RHI::FImageDesc{}
        .SetDimension(viewportSize)
        .SetFormat(RHI::EPixelFormat::Depth32f)
        .SetUsage(RHI::EImageUsage::DepthStencilAttachment | RHI::EImageUsage::Sampled)),
        Default ARGS_IF_RHIDEBUG("VoxelCube/DepthRT") );
    PPE_LOG_CHECK(VoxelCube, _depthRT.Valid());

    return true;
}
//----------------------------------------------------------------------------
bool FVoxelCubeApp::CreateUniformBuffers_(RHI::IFrameGraph& fg) {
    using namespace RHI;

    RHI::TAutoResource<RHI::FBufferID> buf{ fg, fg.CreateBuffer(FBufferDesc{
        sizeof(FUniformData), EBufferUsage::Uniform | EBufferUsage::TransferDst },
        Default ARGS_IF_RHIDEBUG("VoxelCube/UniformBuffer")) };
    PPE_LOG_CHECK(VoxelCube, buf);

    _uniformBuffer = std::move(buf);
    return true;
}
//----------------------------------------------------------------------------
bool FVoxelCubeApp::CreateGraphicsPipeline_(RHI::IFrameGraph& fg) {
    using namespace RHI;

    FGraphicsPipelineDesc desc;
    desc.AddShader(RHI::EShaderType::Vertex, RHI::EShaderLangFormat::VKSL_100, "main", R"#(
        #version 450 core
        layout(location = 0) in vec3 aPosition;
        layout(location = 1) in vec4 aColor;
        layout(location = 2) in vec3 aNormal;

        layout(set=0, binding=1, std140) uniform uCamera {
            mat4 View;
            mat4 Projection;
            mat4 ViewProjection;
            mat4 InvertViewProjection;
        } camera;

        out gl_PerVertex{
            vec4 gl_Position;
        };

        layout(location = 0) out struct{
            vec4 Color;
            vec3 Normal;
        } Out;

        // http://jcgt.org/published/0003/02/01/paper.pdf
        // A Survey of Efficient Representations for Independent Unit Vectors
        vec2 sign_not_zero(vec2 v)
        {
            return vec2((v.x >= 0.f) ? 1.f : -1.f, (v.y >= 0.f) ? 1.f : -1.f);
        }

        /// -1.f <= v.x, v.y <= 1.f
        vec3 oct_decode_dir(vec2 v) {
            vec3 n = vec3(v.xy, 1.f - abs(v.x) - abs(v.y));
            if (n.z < 0.f) n.xy = (1.f - abs(n.yx)) * sign_not_zero(n.xy);
            return normalize(n);
        }

        void main()
        {
            Out.Color = aColor;
            //Out.Normal = oct_decode_dir(aNormal * 2.f - 1.f);
            Out.Normal = aNormal;
            gl_Position = camera.ViewProjection * vec4(aPosition, 1.0);
        })#" ARGS_IF_RHIDEBUG("VoxelCube/Test_VS"));

    desc.AddShader(RHI::EShaderType::Fragment, RHI::EShaderLangFormat::VKSL_100, "main", R"#(
        #version 450 core
        layout(location = 0) out vec4 out_Color0;

        layout(location = 0) in struct{
            vec4 Color;
            vec3 Normal;
        } In;

        void main()
        {
            float L = dot(normalize(In.Normal), normalize(vec3(1, -3, 2))) * 0.5 + 0.5;
            out_Color0 = L * In.Color;
            //out_Color0 = vec4(vec3(L), 1);
            //out_Color0 = vec4(abs(normalize(In.Normal)), 1);
        })#" ARGS_IF_RHIDEBUG("VoxelCube/Test_PS"));

    RHI::TAutoResource<RHI::FGPipelineID> ppln{ fg, fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("VoxelCube")) };
    PPE_LOG_CHECK(VoxelCube, ppln);

    RHI::PPipelineResources resources = fg.CreatePipelineResources(*ppln, "0"_descriptorset);
    PPE_LOG_CHECK(VoxelCube, resources);

    _graphicsPpln = std::move(ppln);
    _resources = std::move(resources);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
