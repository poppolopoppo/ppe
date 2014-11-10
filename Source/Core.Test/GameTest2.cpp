#include "stdafx.h"

#include "GameTest2.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"

#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core.Graphics/Device/Geometry/IndexBuffer.h"
#include "Core.Graphics/Device/Geometry/PrimitiveType.h"
#include "Core.Graphics/Device/Geometry/VertexBuffer.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"
#include "Core.Graphics/Device/Geometry/VertexTypes.h"
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
#include "Core.Engine/Mesh/Geometry/GenericVertexOptimizer.h"
#include "Core.Engine/Mesh/Loader/MeshLoader.h"
#include "Core.Engine/Scene/Scene.h"
#include "Core.Engine/World/World.h"

#include "Core.Application/Input/Camera/KeyboardMouseCameraController.h"

namespace Core {
typedef Graphics::Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N vertex_type;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
GameTest2::GameTest2(const wchar_t *appname)
:   parent_type(
    appname,
    Graphics::DeviceAPI::DirectX11,
    Graphics::PresentationParameters(
        1280, 720,
        Graphics::SurfaceFormat::R8G8B8A8_SRGB,
        Graphics::SurfaceFormat::D24S8,
        false,
        true,
        0,
        Graphics::PresentInterval::Default ),
    10, 10) {}
//----------------------------------------------------------------------------
GameTest2::~GameTest2() {}
//----------------------------------------------------------------------------
void GameTest2::Start() {
    using namespace Engine;

    Assert(!_context);
    _context = new RenderContext(parent_type::Services(), 512/* mo */<< 20);

    parent_type::Start();
}
//----------------------------------------------------------------------------
void GameTest2::Shutdown() {
    using namespace Engine;

    parent_type::Shutdown();

    Assert(_context);
    RemoveRef_AssertReachZero(_context);
}
//----------------------------------------------------------------------------
void GameTest2::Initialize(const Timeline& time) {
    parent_type::Initialize(time);

    VirtualFileSystem::Instance().MountNativePath(L"GameData:/", L"Data/");

    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = DeviceEncapsulator()->Device();
    const ViewportF& viewport = DeviceEncapsulator()->Parameters().Viewport();

    EffectCompiler *const effectCompiler = _context->EffectCompilerService()->EffectCompiler();
    MaterialDatabase *const materialDatabase = _context->MaterialDatabase();
    RenderSurfaceManager *const renderSurfaceManager = _context->RenderSurfaceService()->Manager();
    TextureCache *const textureCache = _context->TextureCacheService()->TextureCache();

    EffectDescriptor *const basicEffectDescriptor = new EffectDescriptor();
    basicEffectDescriptor->SetName("BasicEffect");
    basicEffectDescriptor->SetRenderState(
        new RenderState(RenderState::Blending::Opaque,
                        RenderState::Culling::CounterClockwise,
                        RenderState::DepthTest::Default ));
    basicEffectDescriptor->SetVS(L"GameData:/Shaders/Basic.fx");
    basicEffectDescriptor->SetPS(L"GameData:/Shaders/Basic.fx");
    basicEffectDescriptor->SetShaderProfile(ShaderProfileType::ShaderModel4_1);
    basicEffectDescriptor->AddVertexDeclaration(Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Declaration);

    EffectDescriptor *const bloomSelectorDescriptor = new EffectDescriptor();
    bloomSelectorDescriptor->SetName("BloomSelector");
    bloomSelectorDescriptor->SetRenderState(
        new RenderState(RenderState::Blending::Opaque,
                        RenderState::Culling::None,
                        RenderState::DepthTest::None,
                        RenderState::FillMode::Solid ));
    bloomSelectorDescriptor->SetVS(L"GameData:/Shaders/BloomSelector.fx");
    bloomSelectorDescriptor->SetPS(L"GameData:/Shaders/BloomSelector.fx");
    bloomSelectorDescriptor->SetShaderProfile(ShaderProfileType::ShaderModel4_1);
    bloomSelectorDescriptor->AddVertexDeclaration(Vertex::Position0_Float4__TexCoord0_Float2::Declaration);

    Material *const bloomSelectorMaterial = new Material("BloomSelector");
    bloomSelectorMaterial->AddTexture("Input", L"VirtualData:/Surfaces/Principal/RT");

    MaterialEffect *const bloomSelectorEffect = effectCompiler->CreateMaterialEffect(
        bloomSelectorDescriptor, bloomSelectorDescriptor->VertexDeclarations().front(), bloomSelectorMaterial);

    EffectDescriptor *const gaussianBlur9Descriptor = new EffectDescriptor();
    gaussianBlur9Descriptor->SetName("GaussianBlur9");
    gaussianBlur9Descriptor->SetRenderState(
        new RenderState(RenderState::Blending::Opaque,
                        RenderState::Culling::None,
                        RenderState::DepthTest::None,
                        RenderState::FillMode::Solid ));
    gaussianBlur9Descriptor->SetVS(L"GameData:/Shaders/GaussianBlur9.fx");
    gaussianBlur9Descriptor->SetPS(L"GameData:/Shaders/GaussianBlur9.fx");
    gaussianBlur9Descriptor->SetShaderProfile(ShaderProfileType::ShaderModel4_1);
    gaussianBlur9Descriptor->AddVertexDeclaration(Vertex::Position0_Float4__TexCoord0_Float2::Declaration);

    Material *const bloomBlurHMaterial = new Material("GaussianBlur9H");
    bloomBlurHMaterial->AddTexture("Input", L"VirtualData:/Surfaces/DownsizeDiv2/RT");
    bloomBlurHMaterial->AddParameter("BlurDuDv", new MaterialParameterBlock<float2>(float2(1, 0)));

    Material *const bloomBlurVMaterial = new Material("GaussianBlur9V");
    bloomBlurVMaterial->AddTexture("Input", L"VirtualData:/Surfaces/DownsizeDiv4/RT");
    bloomBlurVMaterial->AddParameter("BlurDuDv", new MaterialParameterBlock<float2>(float2(0, 0.5f)));

    MaterialEffect *const bloomBlurHEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), bloomBlurHMaterial);
    MaterialEffect *const bloomBlurVEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), bloomBlurVMaterial);

    Material *const principalBlurHMaterial = new Material("GaussianBlur9H");
    principalBlurHMaterial->AddTexture("Input", L"VirtualData:/Surfaces/Principal/RT");
    principalBlurHMaterial->AddParameter("BlurDuDv", new MaterialParameterBlock<float2>(float2(1, 0)));

    Material *const principalBlurVMaterial = new Material("GaussianBlur9V");
    principalBlurVMaterial->AddTexture("Input", L"VirtualData:/Surfaces/DownsizeDiv2/RT");
    principalBlurVMaterial->AddParameter("BlurDuDv", new MaterialParameterBlock<float2>(float2(0, 0.5f)));

    MaterialEffect *const principalBlurHEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), principalBlurHMaterial);
    MaterialEffect *const principalBlurVEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), principalBlurVMaterial);

    EffectDescriptor *const postprocessDescriptor = new EffectDescriptor();
    postprocessDescriptor->SetName("Postprocess");
    postprocessDescriptor->SetRenderState(
        new RenderState(RenderState::Blending::Opaque,
                        RenderState::Culling::None,
                        RenderState::DepthTest::None,
                        RenderState::FillMode::Solid ));
    postprocessDescriptor->SetVS(L"GameData:/Shaders/Postprocess.fx");
    postprocessDescriptor->SetPS(L"GameData:/Shaders/Postprocess.fx");
    postprocessDescriptor->SetShaderProfile(ShaderProfileType::ShaderModel4_1);
    postprocessDescriptor->AddVertexDeclaration(Vertex::Position0_Float4__TexCoord0_Float2::Declaration);

    Material *const postprocessMaterial = new Material("Postprocess");
    postprocessMaterial->AddTexture("Principal", L"VirtualData:/Surfaces/Principal/RT");
    postprocessMaterial->AddTexture("Bloom", L"VirtualData:/Surfaces/Bloom/RT");
    postprocessMaterial->AddTexture("PrincipalBlur", L"VirtualData:/Surfaces/DownsizeDiv4/RT");

    MaterialEffect *const postprocessEffect = effectCompiler->CreateMaterialEffect(
        postprocessDescriptor, postprocessDescriptor->VertexDeclarations().front(), postprocessMaterial);

    _world = new World("Test world", Services());
    _world->Initialize();

    _cameraController = new Application::KeyboardMouseCameraController(float3(0.0f, 3.0f, -6.0f), 0.0f, 0.5f*F_PIOver3, &Keyboard(), &Mouse());
    _camera = new PerspectiveCamera(F_PIOver3, 0.01f, 100.0f, viewport);
    _camera->SetController(_cameraController);

    const PAbstractRenderSurface backBuffer = new RenderSurfaceBackBuffer("BackBuffer", RenderSurfaceBackBuffer::RenderTarget_DepthStencil);
    const PAbstractRenderSurface principal = new RenderSurfaceRelative("Principal", float2::One(), SurfaceFormat::R16G16B16A16_F, SurfaceFormat::D24S8);
    const PAbstractRenderSurface downSizeDiv2 = new RenderSurfaceRelative("DownsizeDiv2", float2(0.5f), principal, SurfaceFormat::R11G11B10);
    const PAbstractRenderSurface downSizeDiv4 = new RenderSurfaceRelative("DownsizeDiv4", float2(0.25f), principal, SurfaceFormat::R11G11B10);
    const PAbstractRenderSurface bloom = new RenderSurfaceRelative("Bloom", float2(0.125f), principal, SurfaceFormat::R11G11B10);

    textureCache->SetFallbackTexture2D(L"GameData:/Textures/Tech/error.dds");
    renderSurfaceManager->Register(backBuffer);
    renderSurfaceManager->Register(principal);
    renderSurfaceManager->Register(downSizeDiv2);
    renderSurfaceManager->Register(downSizeDiv4);
    renderSurfaceManager->Register(bloom);

    _mainScene = new Scene("Main scene", _camera, _world, _context->MaterialDatabase());
    _mainScene->Initialize(device);
    _mainScene->RenderTree()->Add(new RenderLayerSetRenderTarget(principal));
    _mainScene->RenderTree()->Add(new RenderLayerClear(principal, Color::DarkSlateBlue));
    _mainScene->RenderTree()->Add(new RenderLayer("Objects"));

    _mainScene->RenderTree()->Add(new RenderLayerSetRenderTarget(downSizeDiv2));
    _mainScene->RenderTree()->Add(new RenderLayerDrawRect(bloomSelectorEffect));
    _mainScene->RenderTree()->Add(new RenderLayerSetRenderTarget(downSizeDiv4));
    _mainScene->RenderTree()->Add(new RenderLayerDrawRect(bloomBlurHEffect));
    _mainScene->RenderTree()->Add(new RenderLayerSetRenderTarget(bloom));
    _mainScene->RenderTree()->Add(new RenderLayerDrawRect(bloomBlurVEffect));

    _mainScene->RenderTree()->Add(new RenderLayerSetRenderTarget(downSizeDiv2));
    _mainScene->RenderTree()->Add(new RenderLayerDrawRect(principalBlurHEffect));
    _mainScene->RenderTree()->Add(new RenderLayerSetRenderTarget(downSizeDiv4));
    _mainScene->RenderTree()->Add(new RenderLayerDrawRect(principalBlurVEffect));

    _mainScene->RenderTree()->Add(new RenderLayerSetRenderTarget(backBuffer));
    _mainScene->RenderTree()->Add(new RenderLayerDrawRect(postprocessEffect));

    _mainScene->MaterialDatabase()->BindEffect("Basic", basicEffectDescriptor);
}
//----------------------------------------------------------------------------
void GameTest2::Destroy() {
    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = DeviceEncapsulator()->Device();

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
void GameTest2::LoadContent() {
    parent_type::LoadContent();

    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = DeviceEncapsulator()->Device();
    IDeviceAPIShaderCompilerEncapsulator *const compiler = DeviceEncapsulator()->Compiler();

    if (!LoadMesh<u32, vertex_type>(device, &_indices[0], &_vertices[0], L"GameData:/Models/dragon_40k.ply"))
        AssertNotReached();

    if (!LoadMesh<u32, vertex_type>(device, &_indices[1], &_vertices[1], L"GameData:/Models/happy_40k.ply"))
        AssertNotReached();

    _transforms[0] = new MaterialParameterBlock<float4x4>();
    _transforms[0]->SetValue(float4x4::Identity());

    _transforms[1] = new MaterialParameterBlock<float4x4>();
    _transforms[1]->SetValue(float4x4::Identity());

    _transforms[2] = new MaterialParameterBlock<float4x4>();
    _transforms[2]->SetValue(float4x4::Identity());

    _materials[0] = new Material("Basic");
    _materials[0]->AddParameter("World", _transforms[0]);
    _materials[0]->AddParameter("InstanceColor", new MaterialParameterBlock<float4>(ColorRGBAF(Color::Cyan)));
    _materials[0]->AddTexture("Diffuse", L"GameData:/Models/dragon_40k_ao.dds");
    _materials[0]->AddTexture("Bump", L"GameData:/Models/dragon_40k_bump.dds");

    _materials[1] = new Material("Basic");
    _materials[1]->AddParameter("World", _transforms[1]);
    _materials[1]->AddParameter("InstanceColor", new MaterialParameterBlock<float4>(ColorRGBAF(Color::Salmon)));
    _materials[1]->AddTexture("Diffuse", L"GameData:/Models/dragon_40k_ao.dds");
    _materials[1]->AddTexture("Bump", L"GameData:/Models/dragon_40k_bump.dds");

    _materials[2] = new Material("Basic");
    _materials[2]->AddParameter("World", _transforms[2]);
    _materials[2]->AddParameter("InstanceColor", new MaterialParameterBlock<float4>(ColorRGBAF(Color::Gold)));
    _materials[2]->AddTexture("Diffuse", L"GameData:/Models/happy_ao.dds");
    _materials[2]->AddTexture("Bump", L"GameData:/Models/happy_bump.dds");

    if (!AcquireRenderCommand(  _commands[0], _mainScene->RenderTree(),
                                "Objects", _materials[0],
                                _indices[0], _vertices[0],
                                PrimitiveType::TriangleList,
                                0, 0, _indices[0]->IndexCount() / 3))
        AssertNotReached();

    if (!AcquireRenderCommand(  _commands[1], _mainScene->RenderTree(),
                                "Objects", _materials[1],
                                _indices[0], _vertices[0],
                                PrimitiveType::TriangleList,
                                0, 0, _indices[0]->IndexCount() / 3))
        AssertNotReached();

    if (!AcquireRenderCommand(  _commands[2], _mainScene->RenderTree(),
                                "Objects", _materials[2],
                                _indices[1], _vertices[1],
                                PrimitiveType::TriangleList,
                                0, 0, _indices[1]->IndexCount() / 3))
        AssertNotReached();
}
//----------------------------------------------------------------------------
void GameTest2::UnloadContent() {
    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = DeviceEncapsulator()->Device();

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
void GameTest2::Update(const Timeline& time) {
    parent_type::Update(time);

    using namespace Engine;
    using namespace Graphics;
    using namespace Application;

    const Graphics::DeviceEncapsulator *encapsulator = DeviceEncapsulator();

    IDeviceAPIEncapsulator *const device = encapsulator->Device();
    IDeviceAPIContextEncapsulator *const context = encapsulator->Context();

    // shader compilation
    if (Keyboard().IsKeyPressed(KeyboardKey::Control) &&
        Keyboard().IsKeyUp(KeyboardKey::F8)) {
        _context->EffectCompilerService()->EffectCompiler()->RegenerateEffects();
    }

    // wireframe
    if (Keyboard().IsKeyUp(KeyboardKey::F11))
        Effect::SwitchAutomaticFillMode();

    // world time speed
    if (Keyboard().IsKeyUp(KeyboardKey::Add))
        _world->SetSpeed(std::max(1.0f, _world->Time().Speed() * 2));
    if (Keyboard().IsKeyUp(KeyboardKey::Substract) && !_world->IsPaused() )
        _world->SetSpeed(_world->Time().Speed() * 0.5f);
    if (Keyboard().IsKeyUp(KeyboardKey::Multiply))
        _world->TogglePause();

    const float rad0 = static_cast<float>(Units::Time::Seconds(_world->Time().Total()).Value()*0.3f);
    _transforms[0]->SetValue(Make3DTransformMatrix(float3(-2,0,0), float3(10), Make3DRotationMatrixAroundY(rad0)));
    const float rad1 = static_cast<float>(Units::Time::Seconds(_world->Time().Total()).Value()*-0.5f+F_PIOver3);
    _transforms[1]->SetValue(Make3DTransformMatrix(float3(+2,0,0), float3(12), Make3DRotationMatrixAroundY(rad1)));
    const float rad2 = static_cast<float>(Units::Time::Seconds(_world->Time().Total()).Value()*0.7f+F_PIOver4);
    _transforms[2]->SetValue(Make3DTransformMatrix(float3(0,-1,+2), float3(15), Make3DRotationMatrixAroundY(rad2)));

    Scene *const scenes[] = { _mainScene.get() };
    _context->UpdateAndPrepare(device, time, _world, MakeView(scenes));
}
//----------------------------------------------------------------------------
void GameTest2::Draw(const Timeline& time) {
    parent_type::Draw(time);

    using namespace Engine;
    using namespace Graphics;

    const Graphics::DeviceEncapsulator *encapsulator = DeviceEncapsulator();

    IDeviceAPIEncapsulator *const device = encapsulator->Device();
    IDeviceAPIContextEncapsulator *const context = encapsulator->Context();

    _context->FrameTick();

    Scene *const scenes[] = { _mainScene.get() };
    _context->Render(context, MakeView(scenes));
}
//----------------------------------------------------------------------------
void GameTest2::Present() {
    parent_type::Present();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
