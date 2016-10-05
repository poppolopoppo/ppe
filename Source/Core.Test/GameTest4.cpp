#include "stdafx.h"

#include "GameTest4.h"

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

#include "Core.Engine/Lighting/LightingEnvironment.h"

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
static Engine::FModel *CreatePBRTestModel_() {
    Engine::PModel referenceModel;
    if (!Engine::LoadModel(referenceModel, L"GameData:/Models/Test/FSphere.obj"))
        AssertNotReached();

    const Engine::PModelMesh modelMesh = referenceModel->Meshes().front();
    const Engine::PModelMeshSubPart modelMeshSubPart = modelMesh->SubParts().front();
    const Engine::PCMaterial material = modelMeshSubPart->Material();
    const AABB3f& meshBoundingBox = referenceModel->BoundingBox();

    const size_t rows = 20;
    const size_t columns = 2;
    const size_t depths = 20;

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
static Engine::FMaterialEffect *DeferredShadeEffect_(Engine::FEffectCompiler *effectCompiler) {
    Assert(effectCompiler);

    using namespace Engine;
    using namespace Graphics;

    PCVertexDeclaration const vertexDecl = Vertex::FPosition0_Float3__TexCoord0_Half2::Declaration;

    PEffectDescriptor const descriptor = new FEffectDescriptor();
    descriptor->SetName("DeferredShade");
    descriptor->SetRenderState(
        new FRenderState(FRenderState::EBlending::Opaque,
                        FRenderState::ECulling::None,
                        FRenderState::EDepthTest::None,
                        FRenderState::EFillMode::Solid ));
    descriptor->SetVS(L"GameData:/Shaders/DeferredShade.fx");
    descriptor->SetPS(L"GameData:/Shaders/DeferredShade.fx");
    descriptor->SetShaderProfile(EShaderProfileType::ShaderModel4_1);
    descriptor->AddVertexDeclaration(vertexDecl);

    PMaterial const material = new FMaterial("DeferredShade");
    material->AddTexture("GBuffer0",    L"VirtualData:/Surfaces/GBuffer0/RT");
    material->AddTexture("GBuffer1",    L"VirtualData:/Surfaces/GBuffer1/RT");
    material->AddTexture("GBuffer2",    L"VirtualData:/Surfaces/GBuffer2/RT");
    material->AddTexture("DepthBuffer", L"VirtualData:/Surfaces/DepthBuffer/DS");

    FMaterialEffect *const effect = effectCompiler->CreateMaterialEffect(descriptor, vertexDecl, material);

    return effect;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGameTest4::FGameTest4(const wchar_t *appname)
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
FGameTest4::~FGameTest4() {}
//----------------------------------------------------------------------------
void FGameTest4::Start() {
    using namespace Engine;

    Assert(!_context);
    _context = new FRenderContext(parent_type::Services(), 512/* mo */<< 20);

    parent_type::Start();
}
//----------------------------------------------------------------------------
void FGameTest4::Shutdown() {
    using namespace Engine;

    parent_type::Shutdown();

    Assert(_context);
    RemoveRef_AssertReachZero(_context);
}
//----------------------------------------------------------------------------
void FGameTest4::Initialize(const FTimeline& time) {
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
    _camera = new PerspectiveCamera(F_PIOver3, 1.0f, 1000.0f, viewport);
    _camera->SetController(_cameraController);

    // Main scene

    const PAbstractRenderSurface backBuffer = new FRenderSurfaceBackBuffer("BackBuffer", FRenderSurfaceBackBuffer::FRenderTarget);
    const PAbstractRenderSurface depthBuffer = new FRenderSurfaceBackBuffer("DepthBuffer", FRenderSurfaceBackBuffer::FDepthStencil);

    renderSurfaceManager->Register(backBuffer);
    renderSurfaceManager->Register(depthBuffer);

    // GBuffer Layout :
    // (0) Diffuse RGB / Metallic A
    // (1) Specular RGB / Roughness A
    // (2) Normal RGB

    const PAbstractRenderSurface gbuffer[] = {
        new FRenderSurfaceRelative("GBuffer0", float2::One(), FSurfaceFormat::R10G10B10A2),
        new FRenderSurfaceRelative("GBuffer1", float2::One(), FSurfaceFormat::R8G8B8A8),
        new FRenderSurfaceRelative("GBuffer2", float2::One(), FSurfaceFormat::R10G10B10A2)
    };

    renderSurfaceManager->Register(gbuffer[0]);
    renderSurfaceManager->Register(gbuffer[1]);
    renderSurfaceManager->Register(gbuffer[2]);

    _mainScene = new FScene("Main scene", _camera, _world, _context->MaterialDatabase());

    // gbuffer rendering
    _mainScene->RenderTree()->Add(new FRenderLayerSetRenderTarget(MakeConstView(gbuffer), depthBuffer));
    _mainScene->RenderTree()->Add(new FRenderLayerClear(gbuffer[0],  Color::Black));
    _mainScene->RenderTree()->Add(new FRenderLayerClear(gbuffer[1],  Color::Transparent));
    _mainScene->RenderTree()->Add(new FRenderLayerClear(gbuffer[2],  Color::Blue));
    _mainScene->RenderTree()->Add(new FRenderLayerClear(depthBuffer, Color::Black));
    _mainScene->RenderTree()->Add(new FRenderLayer("Opaque_Objects"));

    // deferred shade
    _mainScene->RenderTree()->Add(new FRenderLayerSetRenderTarget(backBuffer));
    _mainScene->RenderTree()->Add(new FRenderLayerClear(backBuffer, Color::Black));
    _mainScene->RenderTree()->Add(new FRenderLayerDrawRect(DeferredShadeEffect_(effectCompiler)));


    _mainScene->Initialize(device);
}
//----------------------------------------------------------------------------
void FGameTest4::Destroy() {
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
void FGameTest4::LoadContent() {
    parent_type::LoadContent();

    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = FDeviceEncapsulator().Device();

    FEffectDescriptor *const stdEffectDescriptor = new FEffectDescriptor();
    stdEffectDescriptor->SetName("DeferredStandardEffect");
    stdEffectDescriptor->SetRenderState(
        new FRenderState(FRenderState::EBlending::Opaque,
                        FRenderState::ECulling::CounterClockwise,
                        FRenderState::EDepthTest::Default ));
    stdEffectDescriptor->SetVS(L"GameData:/Shaders/DeferredStandard.fx");
    stdEffectDescriptor->SetPS(L"GameData:/Shaders/DeferredStandard.fx");
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
    stdEffectDescriptor->AddSubstitution(FMaterialConstNames::SeparateAlpha(), "WITH_SEPARATE_ALPHA");

    _mainScene->MaterialDatabase()->BindEffect("Standard", stdEffectDescriptor);
    _mainScene->MaterialDatabase()->BindTexture("IrradianceMap", L"GameData:/Textures/CubeMaps/Test/Irradiance.dds");
    _mainScene->MaterialDatabase()->BindTexture("ReflectionMap", L"GameData:/Textures/CubeMaps/Test/Reflection.dds");

    if (!LoadModel(_model, L"GameData:/Models/Sponza/sponza.obj"))
    //if (!LoadModel(_model, L"GameData:/Models/Sponza/sponza_light.obj"))
    //if (!LoadModel(_model, L"GameData:/Models/Infinity/DesertArena/DesertArena.obj"))
    //if (!LoadModel(_model, L"GameData:/Models/Infinity/BrokenTower/BrokenTower.obj"))
    //if (!LoadModel(_model, L"GameData:/Models/Test/Cone.obj"))
        AssertNotReached();
    
    //_model = CreatePBRTestModel_();
    AssertRelease(_model);

    _model->Create(device);

    if (!AcquireModelRenderCommand( _renderCommand, device, 
                                    _mainScene->RenderTree(),
                                    "Opaque_Objects",
                                    _model.get() ))
        AssertNotReached();
}
//----------------------------------------------------------------------------
void FGameTest4::UnloadContent() {
    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = FDeviceEncapsulator().Device();

    ReleaseModelRenderCommand(_renderCommand, device, _model);

    _model->Destroy(device);
    _model.reset(nullptr);

    parent_type::UnloadContent();
}
//----------------------------------------------------------------------------
void FGameTest4::Update(const FTimeline& time) {
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

    // sunlight control
    if (Keyboard().IsKeyPressed(EKeyboardKey::Control)) {
        const float angularSpeed = Units::Time::Seconds(time.Elapsed()).Value() * F_PI * 0.1;
        FDirectionalLight& sun = _world->Lighting()->Sun();
        const float3& sunDirection = sun.Direction();
        if (Keyboard().IsKeyPressed(EKeyboardKey::L))
            sun.SetDirection(MakeAxisQuaternion(float3::Up(), angularSpeed).Transform(sunDirection));
        if (Keyboard().IsKeyPressed(EKeyboardKey::J))
            sun.SetDirection(MakeAxisQuaternion(float3::Up(), -angularSpeed).Transform(sunDirection));
        if (Keyboard().IsKeyPressed(EKeyboardKey::I))
            sun.SetDirection(MakeAxisQuaternion(float3::Right(), angularSpeed).Transform(sunDirection));
        if (Keyboard().IsKeyPressed(EKeyboardKey::K))
            sun.SetDirection(MakeAxisQuaternion(float3::Right(), -angularSpeed).Transform(sunDirection));
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
void FGameTest4::Draw(const FTimeline& time) {
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
void FGameTest4::Present() {
    parent_type::Present();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
