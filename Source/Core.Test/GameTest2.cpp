#include "stdafx.h"

#include "GameTest2.h"

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
#include "Core.Engine/Material/Material.h"
#include "Core.Engine/Material/MaterialContext.h"
#include "Core.Engine/Material/MaterialDatabase.h"
#include "Core.Engine/Material/Parameters/MaterialParameterBlock.h"
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
#include "Core.Engine/Input/Camera/KeyboardMouseCameraController.h"
#include "Core.Engine/Mesh/GenericVertexOptimizer.h"
#include "Core.Engine/Mesh/Loader/MeshLoader.h"
#include "Core.Engine/Mesh/Loader/ModelLoader.h"
#include "Core.Engine/Mesh/Model.h"
#include "Core.Engine/Mesh/ModelMesh.h"
#include "Core.Engine/Mesh/ModelMeshSubPart.h"
#include "Core.Engine/Scene/Scene.h"
#include "Core.Engine/World/World.h"

#include "Core.Application/ApplicationConsole.h"

namespace Core {
typedef Graphics::Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N vertex_type;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGameTest2::FGameTest2(const wchar_t *appname)
:   parent_type(
    appname,
    Graphics::EDeviceAPI::DirectX11,
    Graphics::FPresentationParameters(
        1280, 720,
        Graphics::FSurfaceFormat::R8G8B8A8_SRGB,
        Graphics::FSurfaceFormat::D24S8,
        false,
        true,
        0,
        Graphics::EPresentInterval::Default ),
    10, 10) {
#ifndef FINAL_RELEASE
    // creates a command window to show stdout messages
    Application::FApplicationConsole::RedirectIOToConsole();
#endif
}
//----------------------------------------------------------------------------
FGameTest2::~FGameTest2() {}
//----------------------------------------------------------------------------
void FGameTest2::Start() {
    using namespace Engine;

    Assert(!_context);
    _context = new FRenderContext(parent_type::Services(), 512/* mo */<< 20);

    parent_type::Start();
}
//----------------------------------------------------------------------------
void FGameTest2::Shutdown() {
    using namespace Engine;

    parent_type::Shutdown();

    Assert(_context);
    RemoveRef_AssertReachZero(_context);
}
//----------------------------------------------------------------------------
void FGameTest2::Initialize(const FTimeline& time) {
    parent_type::Initialize(time);

    FVirtualFileSystem::Instance().MountNativePath(L"GameData:\\", FCurrentProcess::Instance().Directory() + L"\\..\\..\\Data\\");

    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = FDeviceEncapsulator().Device();
    const ViewportF& viewport = FDeviceEncapsulator().Parameters().Viewport();

    FEffectCompiler *const effectCompiler = _context->EffectCompilerService()->EffectCompiler();
    FRenderSurfaceManager *const renderSurfaceManager = _context->RenderSurfaceService()->Manager();
    FTextureCache *const textureCache = _context->TextureCacheService()->TextureCache();

    // Basic mesh rendering effect

    FEffectDescriptor *const basicEffectDescriptor = new FEffectDescriptor();
    basicEffectDescriptor->SetName("BasicEffect");
    basicEffectDescriptor->SetRenderState(
        new FRenderState(FRenderState::EBlending::Opaque,
                        FRenderState::ECulling::CounterClockwise,
                        FRenderState::EDepthTest::Default ));
    basicEffectDescriptor->SetVS(L"GameData:/Shaders/Basic.fx");
    basicEffectDescriptor->SetPS(L"GameData:/Shaders/Basic.fx");
    basicEffectDescriptor->SetShaderProfile(EShaderProfileType::ShaderModel4_1);
    basicEffectDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Declaration);

    // Bloom selector effect

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

    // Separable gaussian blur 9x9 effect

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

    // Principal RT blur (gaussian9)

    FMaterial *const principalBlurHMaterial = new FMaterial("GaussianBlur9H");
    principalBlurHMaterial->AddTexture("Input", L"VirtualData:/Surfaces/Principal/RT");
    principalBlurHMaterial->AddParameter("BlurDuDv", new TMaterialParameterBlock<float2>(float2(1, 0)));

    FMaterial *const principalBlurVMaterial = new FMaterial("GaussianBlur9V");
    principalBlurVMaterial->AddTexture("Input", L"VirtualData:/Surfaces/DownsizeDiv2/RT");
    principalBlurVMaterial->AddParameter("BlurDuDv", new TMaterialParameterBlock<float2>(float2(0, 0.5f)));

    FMaterialEffect *const principalBlurHEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), principalBlurHMaterial);
    FMaterialEffect *const principalBlurVEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), principalBlurVMaterial);

    // Kawase blur effect

    FEffectDescriptor *const kawaseBlurDescriptor = new FEffectDescriptor();
    kawaseBlurDescriptor->SetName("KawaseBlur");
    kawaseBlurDescriptor->SetRenderState(
        new FRenderState(FRenderState::EBlending::Opaque,
                        FRenderState::ECulling::None,
                        FRenderState::EDepthTest::None,
                        FRenderState::EFillMode::Solid ));
    kawaseBlurDescriptor->SetVS(L"GameData:/Shaders/KawaseBlur.fx");
    kawaseBlurDescriptor->SetPS(L"GameData:/Shaders/KawaseBlur.fx");
    kawaseBlurDescriptor->SetShaderProfile(EShaderProfileType::ShaderModel4_1);
    kawaseBlurDescriptor->AddVertexDeclaration(Vertex::FPosition0_Float4__TexCoord0_Float2::Declaration);

    FMaterial *bloomBlurMaterials[5];

    // Bloom blur effect (kawase)

    bloomBlurMaterials[0] = new FMaterial("KawaseBlur0");
    bloomBlurMaterials[0]->AddTexture("Input", L"VirtualData:/Surfaces/DownsizeDiv2/RT");
    bloomBlurMaterials[0]->AddParameter("uniPass", new TMaterialParameterBlock<float>(0));

    bloomBlurMaterials[1] = new FMaterial("KawaseBlur1");
    bloomBlurMaterials[1]->AddTexture("Input", L"VirtualData:/Surfaces/Bloom/RT");
    bloomBlurMaterials[1]->AddParameter("uniPass", new TMaterialParameterBlock<float>(1));

    bloomBlurMaterials[2] = new FMaterial("KawaseBlur2");
    bloomBlurMaterials[2]->AddTexture("Input", L"VirtualData:/Surfaces/DownsizeDiv2/RT");
    bloomBlurMaterials[2]->AddParameter("uniPass", new TMaterialParameterBlock<float>(2));

    bloomBlurMaterials[3] = new FMaterial("KawaseBlur3");
    bloomBlurMaterials[3]->AddTexture("Input", L"VirtualData:/Surfaces/Bloom/RT");
    bloomBlurMaterials[3]->AddParameter("uniPass", new TMaterialParameterBlock<float>(3));

    bloomBlurMaterials[4] = new FMaterial("KawaseBlur4");
    bloomBlurMaterials[4]->AddTexture("Input", L"VirtualData:/Surfaces/DownsizeDiv2/RT");
    bloomBlurMaterials[4]->AddParameter("uniPass", new TMaterialParameterBlock<float>(4));

    FMaterialEffect *bloomBlurEffects[5];
    for (size_t i = 0; i < lengthof(bloomBlurEffects); ++i)
        bloomBlurEffects[i] = effectCompiler->CreateMaterialEffect(
            kawaseBlurDescriptor, kawaseBlurDescriptor->VertexDeclarations().front(), bloomBlurMaterials[i]);

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

    FMaterial *const postprocessMaterial = new FMaterial("Postprocess");
    postprocessMaterial->AddTexture("Principal", L"VirtualData:/Surfaces/Principal/RT");
    postprocessMaterial->AddTexture("Bloom", L"VirtualData:/Surfaces/Bloom/RT");
    postprocessMaterial->AddTexture("PrincipalBlur", L"VirtualData:/Surfaces/PrincipalBlur/RT");

    FMaterialEffect *const postprocessEffect = effectCompiler->CreateMaterialEffect(
        postprocessDescriptor, postprocessDescriptor->VertexDeclarations().front(), postprocessMaterial);

    _world = new FWorld("Test world", Services());
    _world->Initialize();

    _cameraController = new FKeyboardMouseCameraController(float3(0.0f, 3.0f, -6.0f), 0.0f, 0.5f*F_PIOver3, &Keyboard(), &Mouse());
    _camera = new PerspectiveCamera(F_PIOver3, 0.01f, 100.0f, viewport);
    _camera->SetController(_cameraController);

    textureCache->SetFallbackTexture2D(L"GameData:/Textures/Tech/error2D.dds");
    textureCache->SetFallbackTextureCube(L"GameData:/Textures/Tech/errorCube.dds");

    const PAbstractRenderSurface backBuffer = new FRenderSurfaceBackBuffer("BackBuffer", FRenderSurfaceBackBuffer::RenderTarget_DepthStencil);
    const PAbstractRenderSurface principal = new FRenderSurfaceRelative("Principal", float2::One(), FSurfaceFormat::R16G16B16A16_F, FSurfaceFormat::D24S8);
    const PAbstractRenderSurface downSizeDiv2 = new FRenderSurfaceRelative("DownsizeDiv2", float2(0.5f), principal, FSurfaceFormat::R11G11B10);
    const PAbstractRenderSurface principalBlur = new FRenderSurfaceRelative("PrincipalBlur", float2(0.5f), principal, FSurfaceFormat::R11G11B10);
    const PAbstractRenderSurface bloom = new FRenderSurfaceRelative("Bloom", float2(0.5f), principal, FSurfaceFormat::R11G11B10);

    renderSurfaceManager->Register(backBuffer);
    renderSurfaceManager->Register(principal);
    renderSurfaceManager->Register(downSizeDiv2);
    renderSurfaceManager->Register(principalBlur);
    renderSurfaceManager->Register(bloom);

    _mainScene = new FScene("Main scene", _camera, _world, _context->MaterialDatabase());
    _mainScene->Initialize(device);
    _mainScene->RenderTree()->Add(new FRenderLayerSetRenderTarget(principal));
    _mainScene->RenderTree()->Add(new FRenderLayerClear(principal, Color::DarkSlateBlue));
    _mainScene->RenderTree()->Add(new FRenderLayer("Objects"));

    _mainScene->RenderTree()->Add(new FRenderLayerSetRenderTarget(downSizeDiv2));
    _mainScene->RenderTree()->Add(new FRenderLayerDrawRect(bloomSelectorEffect));
    _mainScene->RenderTree()->Add(new FRenderLayerSetRenderTarget(bloom));
    _mainScene->RenderTree()->Add(new FRenderLayerDrawRect(bloomBlurEffects[0]));
    _mainScene->RenderTree()->Add(new FRenderLayerSetRenderTarget(downSizeDiv2));
    _mainScene->RenderTree()->Add(new FRenderLayerDrawRect(bloomBlurEffects[1]));
    _mainScene->RenderTree()->Add(new FRenderLayerSetRenderTarget(bloom));
    _mainScene->RenderTree()->Add(new FRenderLayerDrawRect(bloomBlurEffects[2]));
    _mainScene->RenderTree()->Add(new FRenderLayerSetRenderTarget(downSizeDiv2));
    _mainScene->RenderTree()->Add(new FRenderLayerDrawRect(bloomBlurEffects[3]));
    _mainScene->RenderTree()->Add(new FRenderLayerSetRenderTarget(bloom));
    _mainScene->RenderTree()->Add(new FRenderLayerDrawRect(bloomBlurEffects[4]));

    _mainScene->RenderTree()->Add(new FRenderLayerSetRenderTarget(downSizeDiv2));
    _mainScene->RenderTree()->Add(new FRenderLayerDrawRect(principalBlurHEffect));
    _mainScene->RenderTree()->Add(new FRenderLayerSetRenderTarget(principalBlur));
    _mainScene->RenderTree()->Add(new FRenderLayerDrawRect(principalBlurVEffect));

    _mainScene->RenderTree()->Add(new FRenderLayerSetRenderTarget(backBuffer));
    _mainScene->RenderTree()->Add(new FRenderLayerDrawRect(postprocessEffect));

    _mainScene->MaterialDatabase()->BindEffect("Basic", basicEffectDescriptor);
}
//----------------------------------------------------------------------------
void FGameTest2::Destroy() {
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
void FGameTest2::LoadContent() {
    parent_type::LoadContent();

    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = FDeviceEncapsulator().Device();

    PModel desertArena;
    if (!LoadModel(desertArena, L"GameData:/Models/Infinity/DesertArena/DesertArena.obj"))
        AssertNotReached();

    if (!LoadMesh<u32, vertex_type>(device, &_indices[0], &_vertices[0], L"GameData:/Models/dragon_40k.ply"))
        AssertNotReached();

    if (!LoadMesh<u32, vertex_type>(device, &_indices[1], &_vertices[1], L"GameData:/Models/happy_40k.ply"))
        AssertNotReached();

    _transforms[0] = new TMaterialParameterBlock<float4x4>();
    _transforms[0]->SetValue(float4x4::Identity());

    _transforms[1] = new TMaterialParameterBlock<float4x4>();
    _transforms[1]->SetValue(float4x4::Identity());

    _transforms[2] = new TMaterialParameterBlock<float4x4>();
    _transforms[2]->SetValue(float4x4::Identity());

    _materials[0] = new FMaterial("Basic");
    _materials[0]->AddParameter("FWorld", _transforms[0]);
    _materials[0]->AddParameter("InstanceColor", new TMaterialParameterBlock<float4>(ColorRGBAF(Color::Cyan)));
    _materials[0]->AddTexture("Diffuse", L"GameData:/Models/dragon_40k_ao.dds");
    _materials[0]->AddTexture("Bump", L"GameData:/Models/dragon_40k_bump.dds");

    _materials[1] = new FMaterial("Basic");
    _materials[1]->AddParameter("FWorld", _transforms[1]);
    _materials[1]->AddParameter("InstanceColor", new TMaterialParameterBlock<float4>(ColorRGBAF(Color::Salmon)));
    _materials[1]->AddTexture("Diffuse", L"GameData:/Models/dragon_40k_ao.dds");
    _materials[1]->AddTexture("Bump", L"GameData:/Models/dragon_40k_bump.dds");

    _materials[2] = new FMaterial("Basic");
    _materials[2]->AddParameter("FWorld", _transforms[2]);
    _materials[2]->AddParameter("InstanceColor", new TMaterialParameterBlock<float4>(ColorRGBAF(Color::Gold)));
    _materials[2]->AddTexture("Diffuse", L"GameData:/Models/happy_ao.dds");
    _materials[2]->AddTexture("Bump", L"GameData:/Models/happy_bump.dds");

    if (!AcquireRenderCommand(  _commands[0], _mainScene->RenderTree(),
                                "Objects", _materials[0],
                                _indices[0], _vertices[0],
                                EPrimitiveType::TriangleList,
                                0, 0, _indices[0]->IndexCount() / 3))
        AssertNotReached();

    if (!AcquireRenderCommand(  _commands[1], _mainScene->RenderTree(),
                                "Objects", _materials[1],
                                _indices[0], _vertices[0],
                                EPrimitiveType::TriangleList,
                                0, 0, _indices[0]->IndexCount() / 3))
        AssertNotReached();

    if (!AcquireRenderCommand(  _commands[2], _mainScene->RenderTree(),
                                "Objects", _materials[2],
                                _indices[1], _vertices[1],
                                EPrimitiveType::TriangleList,
                                0, 0, _indices[1]->IndexCount() / 3))
        AssertNotReached();
}
//----------------------------------------------------------------------------
void FGameTest2::UnloadContent() {
    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = FDeviceEncapsulator().Device();

    Assert(_commands[2]);
    ReleaseRenderCommand(_commands[2], device);
    Assert(_commands[1]);
    ReleaseRenderCommand(_commands[1], device);
    Assert(_commands[0]);
    ReleaseRenderCommand(_commands[0], device);

    Assert(_materials[2]);
    RemoveRef_AssertReachZero(_materials[2]);
    Assert(_materials[1]);
    RemoveRef_AssertReachZero(_materials[1]);
    Assert(_materials[0]);
    RemoveRef_AssertReachZero(_materials[0]);

    Assert(_transforms[2]);
    RemoveRef_AssertReachZero(_transforms[2]);
    Assert(_transforms[1]);
    RemoveRef_AssertReachZero(_transforms[1]);
    Assert(_transforms[0]);
    RemoveRef_AssertReachZero(_transforms[0]);

    Assert(_indices[1]);
    _indices[1]->Destroy(device);
    RemoveRef_AssertReachZero(_indices[1]);

    Assert(_vertices[1]);
    _vertices[1]->Destroy(device);
    RemoveRef_AssertReachZero(_vertices[1]);

    Assert(_indices[0]);
    _indices[0]->Destroy(device);
    RemoveRef_AssertReachZero(_indices[0]);

    Assert(_vertices[0]);
    _vertices[0]->Destroy(device);
    RemoveRef_AssertReachZero(_vertices[0]);

    parent_type::UnloadContent();
}
//----------------------------------------------------------------------------
void FGameTest2::Update(const FTimeline& time) {
    parent_type::Update(time);

    using namespace Engine;
    using namespace Graphics;
    using namespace Application;

    const Graphics::FDeviceEncapsulator& encapsulator = FDeviceEncapsulator();

    IDeviceAPIEncapsulator *const device = encapsulator.Device();
    IDeviceAPIContext *const context = encapsulator.Context();

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

    const float rad0 = static_cast<float>(Units::Time::Seconds(_world->Time().Total()).Value()*0.3f);
    _transforms[0]->SetValue(Make3DTransformMatrix(float3(-2,0,0), float3(10), Make3DRotationMatrixAroundY(rad0)));
    const float rad1 = static_cast<float>(Units::Time::Seconds(_world->Time().Total()).Value()*-0.5f+F_PIOver3);
    _transforms[1]->SetValue(Make3DTransformMatrix(float3(+2,0,0), float3(12), Make3DRotationMatrixAroundY(rad1)));
    const float rad2 = static_cast<float>(Units::Time::Seconds(_world->Time().Total()).Value()*0.7f+F_PIOver4);
    _transforms[2]->SetValue(Make3DTransformMatrix(float3(0,-1,+2), float3(15), Make3DRotationMatrixAroundY(rad2)));

    FScene *const scenes[] = { _mainScene.get() };
    _context->UpdateAndPrepare(device, time, _world, MakeView(scenes));
}
//----------------------------------------------------------------------------
void FGameTest2::Draw(const FTimeline& time) {
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
void FGameTest2::Present() {
    parent_type::Present();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
