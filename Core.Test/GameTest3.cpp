#include "stdafx.h"

#include "GameTest3.h"

#include "Core/Logger.h"
#include "Core/ScalarMatrixHelpers.h"
#include "Core/VirtualFileSystem.h"

#include "Core.Graphics/DeviceEncapsulator.h"
#include "Core.Graphics/IndexBuffer.h"
#include "Core.Graphics/PrimitiveType.h"
#include "Core.Graphics/SurfaceFormat.h"
#include "Core.Graphics/VertexBuffer.h"
#include "Core.Graphics/VertexDeclaration.h"
#include "Core.Graphics/VertexTypes.h"

#include "Core.Engine/Effect.h"
#include "Core.Engine/EffectCompilerService.h"
#include "Core.Engine/EffectDescriptor.h"
#include "Core.Engine/Material.h"
#include "Core.Engine/MaterialContext.h"
#include "Core.Engine/MaterialDatabase.h"
#include "Core.Engine/MaterialEffect.h"
#include "Core.Engine/MaterialParameterBlock.h"
#include "Core.Engine/RenderCommand.h"
#include "Core.Engine/RenderContext.h"
#include "Core.Engine/RenderLayer.h"
#include "Core.Engine/RenderLayerClear.h"
#include "Core.Engine/RenderLayerDrawRect.h"
#include "Core.Engine/RenderLayerSetRenderTarget.h"
#include "Core.Engine/RenderState.h"
#include "Core.Engine/RenderSurface.h"
#include "Core.Engine/RenderSurfaceBackBuffer.h"
#include "Core.Engine/RenderSurfaceProxy.h"
#include "Core.Engine/RenderSurfaceRelative.h"
#include "Core.Engine/RenderSurfaceService.h"
#include "Core.Engine/RenderTree.h"
#include "Core.Engine/TextureCacheService.h"

#include "Core.Engine/Camera.h"
#include "Core.Engine/GenericVertexOptimizer.h"
#include "Core.Engine/MeshLoader.h"
#include "Core.Engine/Scene.h"
#include "Core.Engine/World.h"

#include "Core.Application/KeyboardMouseCameraController.h"

namespace Core {
typedef Graphics::Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N vertex_type;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void MountGameDataPath_() {
    auto& const vfs = VirtualFileSystem::Instance();
    vfs.MountNativePath(L"GameData:/", L"D:/Dropbox/code/cpp/DXCPP/Core/Data");
}
//----------------------------------------------------------------------------
static void SetupPostprocess_(
    Engine::Scene *scene,
    Engine::AbstractRenderSurface *principal,
    Engine::AbstractRenderSurface *backBuffer,
    Graphics::IDeviceAPIEncapsulator *device,
    Engine::EffectCompiler *effectCompiler,
    Engine::RenderSurfaceManager *renderSurfaceManager ) {
    using namespace Engine;
    using namespace Graphics;

    // Bloom selector

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

    // Gaussian blur effect

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

    // Bloom gaussian blur material effect

    Material *const bloomBlurHMaterial = new Material("BloomBlurH");
    bloomBlurHMaterial->AddTexture("Input", L"VirtualData:/Surfaces/DownsizeDiv2/RT");
    bloomBlurHMaterial->AddParameter("BlurDuDv", new MaterialParameterBlock<float2>(float2(1, 0)));
    MaterialEffect *const bloomBlurHEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), bloomBlurHMaterial);

    Material *const bloomBlurVMaterial = new Material("BloomBlurV");
    bloomBlurVMaterial->AddTexture("Input", L"VirtualData:/Surfaces/DownsizeDiv4/RT");
    bloomBlurVMaterial->AddParameter("BlurDuDv", new MaterialParameterBlock<float2>(float2(0, 0.5f)));
    MaterialEffect *const bloomBlurVEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), bloomBlurVMaterial);

    // Principal gaussian blur material effect

    Material *const principalBlurHMaterial = new Material("PrincipalBlurH");
    principalBlurHMaterial->AddTexture("Input", L"VirtualData:/Surfaces/Principal/RT");
    principalBlurHMaterial->AddParameter("BlurDuDv", new MaterialParameterBlock<float2>(float2(1, 0)));
    MaterialEffect *const principalBlurHEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), principalBlurHMaterial);

    Material *const principalBlurVMaterial = new Material("PrincipalBlurV");
    principalBlurVMaterial->AddTexture("Input", L"VirtualData:/Surfaces/DownsizeDiv2/RT");
    principalBlurVMaterial->AddParameter("BlurDuDv", new MaterialParameterBlock<float2>(float2(0, 0.5f)));
    MaterialEffect *const principalBlurVEffect = effectCompiler->CreateMaterialEffect(
        gaussianBlur9Descriptor, gaussianBlur9Descriptor->VertexDeclarations().front(), principalBlurVMaterial);

    // Postprocess effect

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

    // Postprocess material effect

    Material *const postprocessMaterial = new Material("Postprocess");
    postprocessMaterial->AddTexture("Principal", L"VirtualData:/Surfaces/Principal/RT");
    postprocessMaterial->AddTexture("Bloom", L"VirtualData:/Surfaces/Bloom/RT");
    postprocessMaterial->AddTexture("PrincipalBlur", L"VirtualData:/Surfaces/DownsizeDiv4/RT");
    MaterialEffect *const postprocessEffect = effectCompiler->CreateMaterialEffect(
        postprocessDescriptor, postprocessDescriptor->VertexDeclarations().front(), postprocessMaterial);

    // Postprocess render surfaces

    const PAbstractRenderSurface downSizeDiv2 = new RenderSurfaceRelative("DownsizeDiv2", float2(0.5f), principal, SurfaceFormat::R11G11B10);
    const PAbstractRenderSurface downSizeDiv4 = new RenderSurfaceRelative("DownsizeDiv4", float2(0.25f), principal, SurfaceFormat::R11G11B10);
    const PAbstractRenderSurface bloom = new RenderSurfaceRelative("Bloom", float2(0.125f), principal, SurfaceFormat::R11G11B10);

    renderSurfaceManager->Register(downSizeDiv2);
    renderSurfaceManager->Register(downSizeDiv4);
    renderSurfaceManager->Register(bloom);

    // Postprocess render tree

    scene->RenderTree()->Add(new RenderLayerSetRenderTarget(downSizeDiv2));
    scene->RenderTree()->Add(new RenderLayerDrawRect(bloomSelectorEffect));
    scene->RenderTree()->Add(new RenderLayerSetRenderTarget(downSizeDiv4));
    scene->RenderTree()->Add(new RenderLayerDrawRect(bloomBlurHEffect));
    scene->RenderTree()->Add(new RenderLayerSetRenderTarget(bloom));
    scene->RenderTree()->Add(new RenderLayerDrawRect(bloomBlurVEffect));

    scene->RenderTree()->Add(new RenderLayerSetRenderTarget(downSizeDiv2));
    scene->RenderTree()->Add(new RenderLayerDrawRect(principalBlurHEffect));
    scene->RenderTree()->Add(new RenderLayerSetRenderTarget(downSizeDiv4));
    scene->RenderTree()->Add(new RenderLayerDrawRect(principalBlurVEffect));

    scene->RenderTree()->Add(new RenderLayerSetRenderTarget(backBuffer));
    scene->RenderTree()->Add(new RenderLayerDrawRect(postprocessEffect));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
GameTest3::GameTest3(const wchar_t *appname)
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
GameTest3::~GameTest3() {}
//----------------------------------------------------------------------------
void GameTest3::Start() {
    using namespace Engine;

    Assert(!_context);
    _context = new RenderContext(parent_type::Services(), 512/* mo */<< 20);

    parent_type::Start();
}
//----------------------------------------------------------------------------
void GameTest3::Shutdown() {
    using namespace Engine;

    parent_type::Shutdown();

    Assert(_context);
    RemoveRef_AssertReachZero(_context);
}
//----------------------------------------------------------------------------
void GameTest3::Initialize(const Timeline& time) {
    parent_type::Initialize(time);

    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = DeviceEncapsulator()->Device();
    const ViewportF& viewport = DeviceEncapsulator()->Parameters().Viewport();

    EffectCompiler *const effectCompiler = _context->EffectCompilerService()->EffectCompiler();
    MaterialDatabase *const materialDatabase = _context->MaterialDatabase();
    RenderSurfaceManager *const renderSurfaceManager = _context->RenderSurfaceService()->Manager();
    TextureCache *const textureCache = _context->TextureCacheService()->TextureCache();

    MountGameDataPath_();

    textureCache->SetFallbackTexture2D(L"GameData:/Textures/error.dds");

    _world = new World("Test world", Services());
    _world->Initialize();

    _cameraController = new Application::KeyboardMouseCameraController(float3(0.0f, 3.0f, -6.0f), 0.0f, 0.5f*F_PIOver3, &Keyboard(), &Mouse());
    _camera = new PerspectiveCamera(F_PIOver3, 0.01f, 100.0f, viewport);
    _camera->SetController(_cameraController);

    const PAbstractRenderSurface backBuffer = new RenderSurfaceBackBuffer("BackBuffer", RenderSurfaceBackBuffer::RenderTarget_DepthStencil);
    const PAbstractRenderSurface principal = new RenderSurfaceRelative("Principal", float2::One(), SurfaceFormat::R16G16B16A16_F, SurfaceFormat::D24S8);

    renderSurfaceManager->Register(backBuffer);
    renderSurfaceManager->Register(principal);

    _mainScene = new Scene("Main scene", _camera, _world, _context->MaterialDatabase());

    _mainScene->RenderTree()->Add(new RenderLayerSetRenderTarget(principal));
    _mainScene->RenderTree()->Add(new RenderLayerClear(principal, Color::OliveDrab));
    _mainScene->RenderTree()->Add(new RenderLayer("Objects"));

    SetupPostprocess_(_mainScene, principal, backBuffer, device, effectCompiler, renderSurfaceManager);

    _mainScene->Initialize(device);
}
//----------------------------------------------------------------------------
void GameTest3::Destroy() {
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
void GameTest3::LoadContent() {
    parent_type::LoadContent();

    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = DeviceEncapsulator()->Device();
    IDeviceAPIShaderCompilerEncapsulator *const compiler = DeviceEncapsulator()->Compiler();

    // TODO
}
//----------------------------------------------------------------------------
void GameTest3::UnloadContent() {
    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = DeviceEncapsulator()->Device();

    // TODO

    parent_type::UnloadContent();
}
//----------------------------------------------------------------------------
void GameTest3::Update(const Timeline& time) {
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

    Scene *const scenes[] = { _mainScene.get() };
    _context->UpdateAndPrepare(device, time, _world, MakeView(scenes));
}
//----------------------------------------------------------------------------
void GameTest3::Draw(const Timeline& time) {
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
void GameTest3::Present() {
    parent_type::Present();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
