#include "stdafx.h"

#include "GameTest3.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Maths/ScalarMatrixHelpers.h"

#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core.Graphics/Device/IndexBuffer.h"
#include "Core.Graphics/Device/PrimitiveType.h"
#include "Core.Graphics/Device/VertexBuffer.h"
#include "Core.Graphics/Device/VertexDeclaration.h"
#include "Core.Graphics/Device/VertexTypes.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"

#include "Core.Engine/Effect/Effect.h"
#include "Core.Engine/Effect/EffectDescriptor.h"
#include "Core.Engine/Effect/MaterialEffect.h"

#include "Core.Engine/Input/Camera/KeyboardMouseCameraController.h"

#include "Core.Engine/Material/Material.h"
#include "Core.Engine/Material/MaterialConstNames.h"
#include "Core.Engine/Material/MaterialContext.h"
#include "Core.Engine/Material/MaterialDatabase.h"
#include "Core.Engine/Material/Parameters/MaterialParameterBlock.h"

#include "Core.Engine/Mesh/Model.h"
#include "Core.Engine/Mesh/ModelBone.h"
#include "Core.Engine/Mesh/ModelMesh.h"
#include "Core.Engine/Mesh/ModelMeshSubPart.h"
#include "Core.Engine/Mesh/Loader/ModelLoader.h"

#include "Core.Engine/Render/Layers/RenderLayer.h"
#include "Core.Engine/Render/Layers/RenderLayerClear.h"
#include "Core.Engine/Render/Layers/RenderLayerDrawRect.h"
#include "Core.Engine/Render/Layers/RenderLayerSetRenderTarget.h"
#include "Core.Engine/Render/RenderCommand.h"
#include "Core.Engine/Render/RenderContext.h"
#include "Core.Engine/Render/RenderState.h"
#include "Core.Engine/Render/RenderTree.h"
#include "Core.Engine/Render/Surfaces/RenderSurface.h"
#include "Core.Engine/Render/Surfaces/RenderSurfaceBackBuffer.h"
#include "Core.Engine/Render/Surfaces/RenderSurfaceProxy.h"
#include "Core.Engine/Render/Surfaces/RenderSurfaceRelative.h"

#include "Core.Engine/Service/EffectCompilerService.h"
#include "Core.Engine/Service/RenderSurfaceService.h"
#include "Core.Engine/Service/TextureCacheService.h"

#include "Core.Engine/Camera/Camera.h"
#include "Core.Engine/Scene/Scene.h"
#include "Core.Engine/World/World.h"

#include "Core.Application/ApplicationConsole.h"

namespace Core {
typedef Graphics::Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N vertex_type;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void MountGameDataPath_() {
    FVirtualFileSystem::Instance().MountNativePath(
        L"GameData:\\", 
        FCurrentProcess::Instance().Directory() + L"\\..\\..\\Data\\" );
}
//----------------------------------------------------------------------------
static void SetupPostprocess_(
    Engine::FScene *scene,
    Engine::FAbstractRenderSurface *principal,
    Engine::FAbstractRenderSurface *backBuffer,
    Graphics::IDeviceAPIEncapsulator *device,
    Engine::FEffectCompiler *effectCompiler,
    Engine::FRenderSurfaceManager *renderSurfaceManager ) {
    using namespace Engine;
    using namespace Graphics;

    // Bloom selector

    FEffectDescriptor *const bloomSelectorDescriptor = new FEffectDescriptor();
    bloomSelectorDescriptor->SetName("BloomSelector");
    bloomSelectorDescriptor->SetRenderState(
        new FRenderState(FRenderState::EBlending::Opaque,
                        FRenderState::ECulling::None,
                        FRenderState::EDepthTest::None,
                        FRenderState::EFillMode::Solid ));
    bloomSelectorDescriptor->SetVS(L"GameData:/Shaders/BloomSelector.fx");
    bloomSelectorDescriptor->SetPS(L"GameData:/Shaders/BloomSelector.fx");
    bloomSelectorDescriptor->SetShaderProfile(EShaderProfileType::ShaderModel4_1);
    bloomSelectorDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float4__TexCoord0_Float2::Declaration);

    FMaterial *const bloomSelectorMaterial = new FMaterial("BloomSelector");
    bloomSelectorMaterial->AddTexture("Input", L"VirtualData:/Surfaces/Principal/RT");

    FMaterialEffect *const bloomSelectorEffect = effectCompiler->CreateMaterialEffect(
        bloomSelectorDescriptor, bloomSelectorDescriptor->VertexDeclarations().front(), bloomSelectorMaterial);

    // Gaussian blur effect

    FEffectDescriptor *const gaussianBlur9Descriptor = new FEffectDescriptor();
    gaussianBlur9Descriptor->SetName("GaussianBlur9");
    gaussianBlur9Descriptor->SetRenderState(
        new FRenderState(FRenderState::EBlending::Opaque,
                        FRenderState::ECulling::None,
                        FRenderState::EDepthTest::None,
                        FRenderState::EFillMode::Solid ));
    gaussianBlur9Descriptor->SetVS(L"GameData:/Shaders/GaussianBlur9.fx");
    gaussianBlur9Descriptor->SetPS(L"GameData:/Shaders/GaussianBlur9.fx");
    gaussianBlur9Descriptor->SetShaderProfile(EShaderProfileType::ShaderModel4_1);
    gaussianBlur9Descriptor->AddVertexDeclaration(Vertex::FPosition0_Float4__TexCoord0_Float2::Declaration);

    // Bloom gaussian blur material effect

    FMaterial *const bloomBlurHMaterial = new FMaterial("BloomBlurH");
    bloomBlurHMaterial->AddTexture("Input", L"VirtualData:/Surfaces/DownsizeDiv2/RT");
    bloomBlurHMaterial->AddParameter("BlurDuDv", new TMaterialParameterBlock<float2>(float2(1, 0)));
    FMaterialEffect *const bloomBlurHEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), bloomBlurHMaterial);

    FMaterial *const bloomBlurVMaterial = new FMaterial("BloomBlurV");
    bloomBlurVMaterial->AddTexture("Input", L"VirtualData:/Surfaces/DownsizeDiv4/RT");
    bloomBlurVMaterial->AddParameter("BlurDuDv", new TMaterialParameterBlock<float2>(float2(0, 0.5f)));
    FMaterialEffect *const bloomBlurVEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), bloomBlurVMaterial);

    // Principal gaussian blur material effect

    FMaterial *const principalBlurHMaterial = new FMaterial("PrincipalBlurH");
    principalBlurHMaterial->AddTexture("Input", L"VirtualData:/Surfaces/Principal/RT");
    principalBlurHMaterial->AddParameter("BlurDuDv", new TMaterialParameterBlock<float2>(float2(1, 0)));
    FMaterialEffect *const principalBlurHEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), principalBlurHMaterial);

    FMaterial *const principalBlurVMaterial = new FMaterial("PrincipalBlurV");
    principalBlurVMaterial->AddTexture("Input", L"VirtualData:/Surfaces/DownsizeDiv2/RT");
    principalBlurVMaterial->AddParameter("BlurDuDv", new TMaterialParameterBlock<float2>(float2(0, 0.5f)));
    FMaterialEffect *const principalBlurVEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), principalBlurVMaterial);

    // Postprocess effect

    FEffectDescriptor *const postprocessDescriptor = new FEffectDescriptor();
    postprocessDescriptor->SetName("Postprocess");
    postprocessDescriptor->SetRenderState(
        new FRenderState(FRenderState::EBlending::Opaque,
                        FRenderState::ECulling::None,
                        FRenderState::EDepthTest::None,
                        FRenderState::EFillMode::Solid ));
    postprocessDescriptor->SetVS(L"GameData:/Shaders/Postprocess.fx");
    postprocessDescriptor->SetPS(L"GameData:/Shaders/Postprocess.fx");
    postprocessDescriptor->SetShaderProfile(EShaderProfileType::ShaderModel4_1);
    postprocessDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float4__TexCoord0_Float2::Declaration);

    // Postprocess material effect

    FMaterial *const postprocessMaterial = new FMaterial("Postprocess");
    postprocessMaterial->AddTexture("Principal", L"VirtualData:/Surfaces/Principal/RT");
    postprocessMaterial->AddTexture("Bloom", L"VirtualData:/Surfaces/Bloom/RT");
    postprocessMaterial->AddTexture("PrincipalBlur", L"VirtualData:/Surfaces/DownsizeDiv4/RT");
    FMaterialEffect *const postprocessEffect = effectCompiler->CreateMaterialEffect(
        postprocessDescriptor, postprocessDescriptor->VertexDeclarations().front(), postprocessMaterial);

    // Postprocess render surfaces

    const PAbstractRenderSurface downSizeDiv2 = new FRenderSurfaceRelative("DownsizeDiv2", float2(0.5f), principal, FSurfaceFormat::R11G11B10);
    const PAbstractRenderSurface downSizeDiv4 = new FRenderSurfaceRelative("DownsizeDiv4", float2(0.25f), principal, FSurfaceFormat::R11G11B10);
    const PAbstractRenderSurface bloom = new FRenderSurfaceRelative("Bloom", float2(0.125f), principal, FSurfaceFormat::R11G11B10);

    renderSurfaceManager->Register(downSizeDiv2);
    renderSurfaceManager->Register(downSizeDiv4);
    renderSurfaceManager->Register(bloom);

    // Postprocess render tree

    scene->RenderTree()->Add(new FRenderLayerSetRenderTarget(downSizeDiv2));
    scene->RenderTree()->Add(new FRenderLayerDrawRect(bloomSelectorEffect));
    scene->RenderTree()->Add(new FRenderLayerSetRenderTarget(downSizeDiv4));
    scene->RenderTree()->Add(new FRenderLayerDrawRect(bloomBlurHEffect));
    scene->RenderTree()->Add(new FRenderLayerSetRenderTarget(bloom));
    scene->RenderTree()->Add(new FRenderLayerDrawRect(bloomBlurVEffect));

    scene->RenderTree()->Add(new FRenderLayerSetRenderTarget(downSizeDiv2));
    scene->RenderTree()->Add(new FRenderLayerDrawRect(principalBlurHEffect));
    scene->RenderTree()->Add(new FRenderLayerSetRenderTarget(downSizeDiv4));
    scene->RenderTree()->Add(new FRenderLayerDrawRect(principalBlurVEffect));

    scene->RenderTree()->Add(new FRenderLayerSetRenderTarget(backBuffer));
    scene->RenderTree()->Add(new FRenderLayerDrawRect(postprocessEffect));
}
//----------------------------------------------------------------------------
static Engine::FModel *CreatePBRTestModel_() {
    Engine::PModel referenceModel;
    if (!Engine::LoadModel(referenceModel, L"GameData:/Models/Test/FSphere.obj"))
        AssertNotReached();

    const Engine::PModelMesh modelMesh = referenceModel->Meshes().front();
    const Engine::PModelMeshSubPart modelMeshSubPart = modelMesh->SubParts().front();
    const Engine::PCMaterial material = modelMeshSubPart->Material();
    const AABB3f& meshBoundingBox = referenceModel->BoundingBox();

    const size_t rows = 10;
    const size_t columns = 2;
    const size_t depths = 10;

    const float margin = 1.15f;

    const float2 roughnessRange(0.05f, 1.0f);
    const float2 refractiveIndexRange(0.05f, 3.0f);
    const float2 metallicRange(0.0f, 1.0f);
    const float3 positionRange(
        meshBoundingBox.Extents().x()*rows*margin, 
        meshBoundingBox.Extents().y()*columns*margin, 
        meshBoundingBox.Extents().z()*depths*margin );

    const float dr = 1.0f/(rows - 1);
    const float dc = 1.0f/(columns - 1);
    const float dd = 1.0f/(depths - 1);

    VECTOR(Mesh, Engine::PModelMeshSubPart) newModelMeshSubParts;
    newModelMeshSubParts.reserve(rows * columns);

    AABB3f newBoundingBox;

    for (size_t i = 0; i < rows; ++i) {
        const float fx = i*dr;
        const float roughness = TLerp(roughnessRange.x(), roughnessRange.y(), fx);
        const float x = positionRange.x() * fx - positionRange.x() * 0.5f;

        for (size_t j = 0; j < columns; ++j) {
            const float fy = j*dc;
            const float metallic = TLerp(metallicRange.x(), metallicRange.y(), fy);
            const float y = positionRange.y() * fy - positionRange.y() * 0.5f;

            for (size_t k = 0; k < depths; ++k) {
                const float fz = k*dd;
                const float refractiveIndex = TLerp(refractiveIndexRange.x(), refractiveIndexRange.y(), fz);
                const float z = positionRange.z() * fz - positionRange.z() * 0.5f;

                const float3 translation(x, y, z);
                const float4x4 world = MakeTranslationMatrix(translation);
                const AABB3f boundingBox(meshBoundingBox.Min()+translation, meshBoundingBox.Max()+translation);

                Engine::FMaterial *const newMaterial = new Engine::FMaterial(*material);
                newMaterial->SetParameter(Engine::FMaterialConstNames::Metallic(), new Engine::TMaterialParameterBlock<float>(metallic));
                newMaterial->SetParameter(Engine::FMaterialConstNames::Roughness(), new Engine::TMaterialParameterBlock<float>(roughness));
                newMaterial->SetParameter(Engine::FMaterialConstNames::RefractiveIndex(), new Engine::TMaterialParameterBlock<float>(refractiveIndex));
                newMaterial->SetParameter(Engine::FMaterialConstNames::FWorld(), new Engine::TMaterialParameterBlock<float4x4>(world));

                newModelMeshSubParts.push_back(new Engine::FModelMeshSubPart(
                    modelMeshSubPart->Name(),
                    modelMeshSubPart->BoneIndex(),
                    modelMeshSubPart->BaseVertex(),
                    modelMeshSubPart->FirstIndex(),
                    modelMeshSubPart->IndexCount(),
                    boundingBox,
                    newMaterial ));

                newBoundingBox.Add(boundingBox);
            }
        }
    }

    Engine::MeshRawData indices(modelMesh->Indices());
    Engine::MeshRawData vertices(modelMesh->Vertices());

    VECTOR(Mesh, Engine::PModelBone) newModelBones;
    newModelBones.push_back(referenceModel->Bones().front());

    VECTOR(Mesh, Engine::PModelMesh) newModelMeshes;
    newModelMeshes.push_back(new Engine::FModelMesh(
        modelMesh->IndexCount(), 
        modelMesh->VertexCount(),
        modelMesh->PrimitiveType(),
        modelMesh->IndexType(),
        modelMesh->VertexDeclaration(),
        std::move(indices),
        std::move(vertices),
        std::move(newModelMeshSubParts) ));

    return new Engine::FModel(referenceModel->Name(), newBoundingBox, std::move(newModelBones), std::move(newModelMeshes));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGameTest3::FGameTest3(const wchar_t *appname)
:   parent_type(
    appname,
    Graphics::EDeviceAPI::DirectX11,
    Graphics::FPresentationParameters(
        //640, 480,
        1600, 900,
        Graphics::FSurfaceFormat::R8G8B8A8_SRGB,
        Graphics::FSurfaceFormat::D24S8,
        false,
        true,
        0,
        Graphics::EPresentInterval::Default ),
    10, 10) {
#if 1
#ifndef FINAL_RELEASE
    // creates a command window to show stdout messages
    Application::FApplicationConsole::RedirectIOToConsole();
#endif
#endif
}
//----------------------------------------------------------------------------
FGameTest3::~FGameTest3() {}
//----------------------------------------------------------------------------
void FGameTest3::Start() {
    using namespace Engine;

    Assert(!_context);
    _context = new FRenderContext(parent_type::Services(), 512/* mo */<< 20);

    parent_type::Start();
}
//----------------------------------------------------------------------------
void FGameTest3::Shutdown() {
    using namespace Engine;

    parent_type::Shutdown();

    Assert(_context);
    RemoveRef_AssertReachZero(_context);
}
//----------------------------------------------------------------------------
void FGameTest3::Initialize(const FTimeline& time) {
    parent_type::Initialize(time);

    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = FDeviceEncapsulator().Device();
    const ViewportF& viewport = FDeviceEncapsulator().Parameters().Viewport();

    FEffectCompiler *const effectCompiler = _context->EffectCompilerService()->EffectCompiler();
    FRenderSurfaceManager *const renderSurfaceManager = _context->RenderSurfaceService()->Manager();
    FTextureCache *const textureCache = _context->TextureCacheService()->TextureCache();

    MountGameDataPath_();

    textureCache->SetFallbackTexture2D(L"GameData:/Textures/Tech/error2D.dds");
    textureCache->SetFallbackTextureCube(L"GameData:/Textures/Tech/errorCube.dds");

    _world = new FWorld("Test world", Services());
    _world->Initialize();

    _cameraController = new FKeyboardMouseCameraController(float3(0.0f, 3.0f, -6.0f), 0.0f, 0.5f*F_PIOver3, &Keyboard(), &Mouse());
    _camera = new PerspectiveCamera(F_PIOver3, 0.01f, 1000.0f, viewport);
    _camera->SetController(_cameraController);

    // Main scene

    const PAbstractRenderSurface backBuffer = new FRenderSurfaceBackBuffer("BackBuffer", FRenderSurfaceBackBuffer::RenderTarget_DepthStencil);
    const PAbstractRenderSurface principal = new FRenderSurfaceRelative("Principal", float2::One(), FSurfaceFormat::R16G16B16A16_F, FSurfaceFormat::D24S8);

    renderSurfaceManager->Register(backBuffer);
    renderSurfaceManager->Register(principal);

    _mainScene = new FScene("Main scene", _camera, _world, _context->MaterialDatabase());

    _mainScene->RenderTree()->Add(new FRenderLayerSetRenderTarget(principal));
    _mainScene->RenderTree()->Add(new FRenderLayerClear(principal, Color::OliveDrab));
    _mainScene->RenderTree()->Add(new FRenderLayer("Opaque_Objects"));
    _mainScene->RenderTree()->Add(new FRenderLayer("Transparent_Objects"));

    // Postprocess settings

    SetupPostprocess_(_mainScene, principal, backBuffer, device, effectCompiler, renderSurfaceManager);

    _mainScene->Initialize(device);
}
//----------------------------------------------------------------------------
void FGameTest3::Destroy() {
    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = FDeviceEncapsulator().Device();

    _mainScene->Destroy(device);
    RemoveRef_AssertReachZero(_mainScene);

    _context->Clear();

    _camera->SetController(nullptr);
    RemoveRef_AssertReachZero(_cameraController);
    RemoveRef_AssertReachZero(_camera);

    _world->Destroy();
    RemoveRef_AssertReachZero(_world);

    parent_type::Destroy();
}
//----------------------------------------------------------------------------
void FGameTest3::LoadContent() {
    parent_type::LoadContent();

    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = FDeviceEncapsulator().Device();

#if 1
    // Standard model rendering effect

    FEffectDescriptor *const stdEffectDescriptor = new FEffectDescriptor();
    stdEffectDescriptor->SetName("StandardEffect");
    stdEffectDescriptor->SetRenderState(
        new FRenderState(FRenderState::EBlending::AlphaBlend,
                        FRenderState::ECulling::CounterClockwise,
                        FRenderState::EDepthTest::Default ));
    stdEffectDescriptor->SetVS(L"GameData:/Shaders/Standard.fx");
    stdEffectDescriptor->SetPS(L"GameData:/Shaders/Standard.fx");
    stdEffectDescriptor->SetShaderProfile(EShaderProfileType::ShaderModel4_1);
    stdEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Declaration);
    stdEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Declaration);
    stdEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Declaration);
    stdEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Declaration);
    stdEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__TexCoord0_Float2__Normal0_Float3::Declaration);
    stdEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2::Declaration);
    stdEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N::Declaration);
    stdEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__Normal0_UX10Y10Z10W2N::Declaration);
    stdEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__Color0_UByte4N::Declaration);
    stdEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3::Declaration);
    stdEffectDescriptor->AddSubstitution(FMaterialConstNames::BumpMapping(), "WITH_BUMP_MAPPING");
    stdEffectDescriptor->AddSubstitution(FMaterialConstNames::SeparateAlpha(), "WITH_SEPARATE_ALPHA=1");

    _mainScene->MaterialDatabase()->BindEffect("Standard", stdEffectDescriptor);

#else
    // AA Wireframe model rendering effect

    FEffectDescriptor *const wireframeEffectDescriptor = new FEffectDescriptor();
    wireframeEffectDescriptor->SetName("WireframeEffect");
    wireframeEffectDescriptor->SetRenderState(
        new FRenderState(FRenderState::EBlending::AlphaBlend,
                        FRenderState::ECulling::None,
                        FRenderState::EDepthTest::Default,
                        FRenderState::EFillMode::Solid ));
    wireframeEffectDescriptor->SetVS(L"GameData:/Shaders/Wireframe.fx");
    wireframeEffectDescriptor->SetGS(L"GameData:/Shaders/Wireframe.fx");
    wireframeEffectDescriptor->SetPS(L"GameData:/Shaders/Wireframe.fx");
    wireframeEffectDescriptor->SetShaderProfile(EShaderProfileType::ShaderModel4_1);
    wireframeEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Declaration);
    wireframeEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Declaration);
    wireframeEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Declaration);
    wireframeEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Declaration);
    wireframeEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__TexCoord0_Float2__Normal0_Float3::Declaration);
    wireframeEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2::Declaration);
    wireframeEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N::Declaration);
    wireframeEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__Normal0_UX10Y10Z10W2N::Declaration);
    wireframeEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__Color0_UByte4N::Declaration);
    wireframeEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3::Declaration);

    _mainScene->MaterialDatabase()->BindEffect("Standard", wireframeEffectDescriptor);

#endif
    _mainScene->MaterialDatabase()->BindTexture("IrradianceMap", L"GameData:/Textures/CubeMaps/Test/Irradiance.dds");
    _mainScene->MaterialDatabase()->BindTexture("ReflectionMap", L"GameData:/Textures/CubeMaps/Test/Reflection.dds");

#if 0
    //if (!LoadModel(_model, L"GameData:/Models/Infinity/DesertArena/DesertArena.obj"))
    //if (!LoadModel(_model, L"GameData:/Models/Infinity/BrokenTower/BrokenTower.obj"))
    //if (!LoadModel(_model, L"GameData:/Models/Infinity/Beach/Beach.obj"))
    //if (!LoadModel(_model, L"GameData:/Models/Infinity/Potion/Potion.obj"))
    //if (!LoadModel(_model, L"GameData:/Models/Test/Cone.obj"))
    //if (!LoadModel(_model, L"GameData:/Models/Test/TeaPot.obj"))
    //if (!LoadModel(_model, L"GameData:/Models/Test/FScene.obj"))
    //if (!LoadModel(_model, L"GameData:/Models/Test/FSphere.obj"))
    //if (!LoadModel(_model, L"GameData:/Models/Test/Bunny.obj"))
    if (!LoadModel(_model, L"GameData:/Models/Sponza/sponza.obj"))
    //if (!LoadModel(_model, L"GameData:/Models/Sponza/sponza_light.obj"))
        AssertNotReached();

#else
    _model = CreatePBRTestModel_();
    AssertRelease(_model);

#endif

    _model->Create(device);

    const TPair<Graphics::FBindName, const char *> tagToRenderLayerName[] = {
        {FMaterialConstNames::SeparateAlpha(), "Transparent_Objects"}
    };
    if (!AcquireModelRenderCommand( _renderCommand, device, 
                                    _mainScene->RenderTree(),
                                    MakeConstView(tagToRenderLayerName),
                                    "Opaque_Objects",
                                    _model.get() ))
        AssertNotReached();
}
//----------------------------------------------------------------------------
void FGameTest3::UnloadContent() {
    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = FDeviceEncapsulator().Device();

    ReleaseModelRenderCommand(_renderCommand, device, _model);

    _model->Destroy(device);
    _model.reset(nullptr);

    parent_type::UnloadContent();
}
//----------------------------------------------------------------------------
void FGameTest3::Update(const FTimeline& time) {
    parent_type::Update(time);

    using namespace Engine;
    using namespace Graphics;
    using namespace Application;

    const Graphics::FDeviceEncapsulator& encapsulator = FDeviceEncapsulator();

    IDeviceAPIEncapsulator *const device = encapsulator.Device();
    IDeviceAPIContext *const context = encapsulator.Context();

    // texture reloading
    if (Keyboard().IsKeyPressed(EKeyboardKey::Control) &&
        Keyboard().IsKeyUp(EKeyboardKey::F7)) {
        _context->TextureCacheService()->TextureCache()->ReloadAllTextures();
    }
    // shader compilation
    if (Keyboard().IsKeyPressed(EKeyboardKey::Control) &&
        Keyboard().IsKeyUp(EKeyboardKey::F8)) {
        _context->EffectCompilerService()->EffectCompiler()->RegenerateEffects();
    }

    // wireframe
    if (Keyboard().IsKeyUp(EKeyboardKey::F11))
        FEffect::SwitchAutomaticFillMode();

    // world time speed
    if (Keyboard().IsKeyUp(EKeyboardKey::Add))
        _world->SetSpeed(std::max(1.0f, _world->Speed() * 2));
    if (Keyboard().IsKeyUp(EKeyboardKey::Subtract) && !_world->IsPaused() )
        _world->SetSpeed(_world->Speed() * 0.5f);
    if (Keyboard().IsKeyUp(EKeyboardKey::Multiply))
        _world->TogglePause();

    FScene *const scenes[] = { _mainScene.get() };
    _context->UpdateAndPrepare(device, time, _world, MakeView(scenes));
}
//----------------------------------------------------------------------------
void FGameTest3::Draw(const FTimeline& time) {
    parent_type::Draw(time);

    using namespace Engine;
    using namespace Graphics;

    const Graphics::FDeviceEncapsulator& encapsulator = FDeviceEncapsulator();

    IDeviceAPIEncapsulator *const device = encapsulator.Device();
    IDeviceAPIContext *const context = encapsulator.Context();

    _context->FrameTick();

    FScene *const scenes[] = { _mainScene.get() };
    _context->Render(context, MakeView(scenes));
}
//----------------------------------------------------------------------------
void FGameTest3::Present() {
    parent_type::Present();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
