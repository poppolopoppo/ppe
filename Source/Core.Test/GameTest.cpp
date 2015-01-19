#include "stdafx.h"

#include "GameTest.h"

#include "Core.Graphics/Device/DeviceAPIEncapsulator.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"

#include "Core.Graphics/Device/Geometry/PrimitiveType.h"
#include "Core.Graphics/Device/PresentationParameters.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"

#include "Core.Graphics/Device/State/BlendState.h"
#include "Core.Graphics/Device/State/DepthStencilState.h"
#include "Core.Graphics/Device/State/RasterizerState.h"

#include "Core.Graphics/Device/Texture/DepthStencil.h"
#include "Core.Graphics/Device/Texture/RenderTarget.h"

#include "Core.Graphics/Device/Geometry/IndexBuffer.h"
#include "Core.Graphics/Device/Geometry/VertexBuffer.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core.Graphics/Device/Shader/ShaderEffect.h"
#include "Core.Graphics/Device/Shader/ShaderProgram.h"
#include "Core.Graphics/Device/Shader/ShaderSource.h"

#include "Core.Graphics/Device/Shader/ConstantBuffer.h"
#include "Core.Graphics/Device/Shader/ConstantBufferLayout.h"

#include "Core.Graphics/Device/State/SamplerState.h"
#include "Core.Graphics/Device/Texture/RenderTarget.h"
#include "Core.Graphics/Device/Texture/Texture2D.h"

#include "Core.Engine/Mesh/Geometry/GenericVertex.h"
#include "Core.Engine/Mesh/Geometry/GenericVertexOptimizer.h"
#include "Core.Engine/Mesh/Geometry/GeometricPrimitives.h"
#include "Core.Engine/Mesh/Loader/MeshLoader.h"

#include "Core.Engine/Texture/TextureLoader.h"

#include "Core.Engine/Camera/Camera.h"
#include "Core.Engine/Camera/CameraController.h"
#include "Core.Engine/Camera/CameraModel.h"

#include "Core.Application/Input/Camera/KeyboardMouseCameraController.h"

#include "Core/Container/RawStorage.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Maths/Geometry/ScalarRectangle.h"
#include "Core/Maths/Packing/PackedVectors.h"
#include "Core/Maths/RandomGenerator.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static Graphics::ShaderEffect *CompileEffect_VS_PS_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::IDeviceAPIShaderCompilerEncapsulator *compiler,
    const Graphics::VertexDeclaration *vertexDeclaration,
    const Filename& sourceName ) {
    using namespace Graphics;

    const MemoryView<const Pair<String, String>> defines;
    const ShaderCompilerFlags compilerFlags =
#ifdef _DEBUG
        ShaderCompilerFlags::DefaultForDebug;
#else
        ShaderCompilerFlags::Default;
#endif

    PShaderProgram vertexProgram = new ShaderProgram(ShaderProfileType::ShaderModel4_1, ShaderProgramType::Vertex);
    vertexProgram->Freeze();
    CompileShaderProgram(compiler, vertexProgram, "VSMain", compilerFlags, sourceName, vertexDeclaration, defines);

    PShaderProgram pixelProgram = new ShaderProgram(ShaderProfileType::ShaderModel4_1, ShaderProgramType::Pixel);
    pixelProgram->Freeze();
    CompileShaderProgram(compiler, pixelProgram, "PSMain", compilerFlags, sourceName, vertexDeclaration, defines);

    ShaderEffect *const effect = new ShaderEffect(vertexDeclaration);
    effect->SetStageProgram(ShaderProgramType::Vertex, std::move(vertexProgram));
    effect->SetStageProgram(ShaderProgramType::Pixel, std::move(pixelProgram));
    effect->Freeze();
    effect->Create(device);

    return effect;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
GameTest::GameTest(const wchar_t *appname)
:   parent_type(
    appname,
    Graphics::DeviceAPI::DirectX11,
    Graphics::PresentationParameters(
        1024, 768,
        Graphics::SurfaceFormat::R8G8B8A8_SRGB,
        Graphics::SurfaceFormat::D24S8,
        false,
        true,
        0,
        Graphics::PresentInterval::Default ),
    10, 10)
,   _clearColor(
        RGB_to_HSV(Color::IndianRed.Data()),
        RGB_to_HSV(Color::LawnGreen.Data())
    )
,   _rotationAngle(-F_PI, F_PI)
{ }
//----------------------------------------------------------------------------
GameTest::~GameTest() {}
//----------------------------------------------------------------------------
void GameTest::Initialize(const Timeline& time) {
    parent_type::Initialize(time);

    VirtualFileSystem::Instance().MountNativePath(L"GameData:/", L"D:/Dropbox/code/cpp/DXCPP/Core/Data");

    using namespace Engine;
    using namespace Graphics;

    _clearColor.Start(time, Units::Time::Seconds(3));
    _rotationAngle.Start(time, Units::Time::Minutes(0.5));

    const ViewportF& viewport = DeviceEncapsulator()->Parameters().Viewport();

    _camera = new PerspectiveCamera(F_PIOver3, 0.01f, 100.0f, viewport);
    _cameraController = new Application::KeyboardMouseCameraController(float3(0.0f, 3.0f, -6.0f), 0.0f, 0.5f*F_PIOver3, &Keyboard(), &Mouse());
    _camera->SetController(_cameraController);

    _sunLightDirection = Normalize3(float3( 0,1,-3));
}
//----------------------------------------------------------------------------
void GameTest::Destroy() {
    parent_type::Destroy();

    _camera->SetController(nullptr);
    RemoveRef_AssertReachZero(_cameraController);
    RemoveRef_AssertReachZero(_camera);
}
//----------------------------------------------------------------------------
void GameTest::LoadContent() {
    parent_type::LoadContent();

    Assert(!_shaderEffect);

    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = DeviceEncapsulator()->Device();
    IDeviceAPIShaderCompilerEncapsulator *const compiler = DeviceEncapsulator()->Compiler();

    const VertexDeclaration *vertexDeclaration = Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Declaration;

    _shaderEffect = CompileEffect_VS_PS_(device, compiler, vertexDeclaration, L"GameData:/Shaders/Basic_old.fx");

    ConstantBufferLayout *const layoutPerFrame = new ConstantBufferLayout();
    layoutPerFrame->AddField("View", ConstantFieldType::Float4x4);
    layoutPerFrame->AddField("Projection", ConstantFieldType::Float4x4);
    layoutPerFrame->AddField("EyePosition", ConstantFieldType::Float3);
    layoutPerFrame->AddField("LightDir", ConstantFieldType::Float3);

    _perFrameBuffer = new ConstantBuffer(layoutPerFrame);
    _perFrameBuffer->SetResourceName("PerFrameBuffer");
    _perFrameBuffer->Freeze();
    _perFrameBuffer->Create(device);

    ConstantBufferLayout *const layoutPerObject = new ConstantBufferLayout();
    layoutPerObject->AddField("World", ConstantFieldType::Float4x4);
    layoutPerObject->AddField("WorldInvertT", ConstantFieldType::Float4x4);

    _perObjectBuffer = new ConstantBuffer(layoutPerObject);
    _perObjectBuffer->SetResourceName("PerObjectBuffer");
    _perObjectBuffer->Freeze();
    _perObjectBuffer->Create(device);

#if 0
    GeometricPrimitive::Indices indices;
    GeometricPrimitive::Positions positions;

    //GeometricPrimitive::Cube(indices, positions);
    //GeometricPrimitive::Octahedron(indices, positions);
    //GeometricPrimitive::Icosahedron(indices, positions);
    GeometricPrimitive::Geosphere(5, indices, positions);

    /*for (float3& position : positions)
        position *= 3.0f;*/

    const size_t indexCount = indices.size();
    const size_t vertexCount = positions.size();

    GeometricPrimitive::Normals normals;
    GeometricPrimitive::SmoothNormals(normals, indices, positions);

    GeometricPrimitive::Colors colors;
    colors.resize(positions.size());

    {
        RandomGenerator rnd;
        for (ColorRGBA& color : colors)
            color = ColorRGBAF(rnd.NextFloat01(),rnd.NextFloat01(),rnd.NextFloat01(),1);
    }

    RAWSTORAGE_ALIGNED(Geometry, u8, 16) exportVertices;
    ExportVertices(
        exportVertices,
        Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Declaration,
        vertexCount,
        &positions,
        &colors,
        nullptr,
        &normals,
        nullptr,
        nullptr );

    const float ACMR0 = VertexAverageCacheMissRate(MakeView(indices));

    OptimizeIndicesAndVerticesOrder(
        exportVertices,
        Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Declaration,
        vertexCount,
        MakeView(indices) );

    const float ACMR1 = VertexAverageCacheMissRate(MakeView(indices));

    LOG(Warning, L"Optimized mesh average cache miss rate from {0}% to {1}%", ACMR0, ACMR1);

    RAWSTORAGE_ALIGNED(Geometry, u16, 16) exportIndices;
    exportIndices.Resize_DiscardData(indexCount);
    for (size_t i = 0; i < indexCount; ++i)
        exportIndices[i] = checked_cast<u16>(indices[i]);

    _vertexBuffer = new VertexBuffer(   Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Declaration,
                                        vertexCount, BufferMode::None, BufferUsage::Default );
    _vertexBuffer->Freeze();
    _vertexBuffer->Create(device, exportVertices.MakeView().Cast<const u8>());

    _indexBuffer = new IndexBuffer(IndexElementSize::SixteenBits, indexCount, BufferMode::None, BufferUsage::Default);
    _indexBuffer->Freeze();
    _indexBuffer->Create(device, exportIndices.MakeView().Cast<const u16>());

#else

    MeshLoader<u32, Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N> mesh;
    if (!mesh.Read(L"GameData:/Models/dragon_40k.ply"))
        AssertNotReached();

    const float ACMR0 = VertexAverageCacheMissRate(mesh.Indices().MakeView());

    OptimizeIndicesAndVerticesOrder(
        mesh.Vertices(),
        vertexDeclaration,
        mesh.Header().VertexCount,
        mesh.Indices().MakeView() );

    const float ACMR1 = VertexAverageCacheMissRate(mesh.Indices().MakeView());

    LOG(Warning, L"Optimized mesh average cache miss rate from {0}% to {1}%", ACMR0, ACMR1);

    _indexBuffer = mesh.CreateIndexBuffer(device, "test_mesh");
    _vertexBuffer = mesh.CreateVertexBuffer(device, "test_mesh");

    mesh.Clear();

#endif

    _diffuseTexture = checked_cast<Texture2D *>(Texture2DLoader::Load(device, L"GameData:/Models/dragon_40k_ao.dds", true));
    AssertRelease(_diffuseTexture);

    _bumpTexture = checked_cast<Texture2D *>(Texture2DLoader::Load(device, L"GameData:/Models/dragon_40k_bump.dds", false));
    AssertRelease(_bumpTexture);

    _screenDuDvTexture = checked_cast<Texture2D *>(Texture2DLoader::Load(device, L"GameData:/Textures/GrassDuDv.dds", false));
    AssertRelease(_screenDuDvTexture);

    const ViewportF& viewport = DeviceEncapsulator()->Parameters().Viewport();

    _renderTarget = new RenderTarget(size_t(viewport.Width()), size_t(viewport.Height()), SurfaceFormat::R16G16B16A16_F);
    _renderTarget->SetResourceName("SceneBuffer");
    _renderTarget->Freeze();
    _renderTarget->Create(device);

    ALIGN(16) const Vertex::Position0_Half2 fullscreenVertices[] = {
        {half2(HalfFloat::MinusOne, HalfFloat::MinusOne)},
        {half2(HalfFloat::MinusOne, HalfFloat::One)},
        {half2(HalfFloat::One,      HalfFloat::MinusOne)},
        {half2(HalfFloat::One,      HalfFloat::One)},
    };

    _fullscreenQuadVertexBuffer = new VertexBuffer(Vertex::Position0_Half2::Declaration, lengthof(fullscreenVertices), BufferMode::None, BufferUsage::Default);
    _fullscreenQuadVertexBuffer->Freeze();
    _fullscreenQuadVertexBuffer->Create(device, MakeView(fullscreenVertices));

    _postProcessEffect = CompileEffect_VS_PS_(device, compiler, Vertex::Position0_Half2::Declaration, L"GameData:/Shaders/Postprocess.fx");

    ConstantBufferLayout *const layoutPostProcess = new ConstantBufferLayout();
    layoutPostProcess->AddField("SceneTexture_DuDvDimensions", ConstantFieldType::Float4);
    layoutPostProcess->AddField("TimelineInSeconds", ConstantFieldType::Float);

    _postProcessParams = new ConstantBuffer(layoutPostProcess);
    _postProcessParams->SetResourceName("PostProcessParams");
    _postProcessParams->Freeze();
    _postProcessParams->Create(device);
}
//----------------------------------------------------------------------------
void GameTest::UnloadContent() {
    parent_type::UnloadContent();

    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = DeviceEncapsulator()->Device();

    Assert(_screenDuDvTexture);
    _screenDuDvTexture->Destroy(device);
    RemoveRef_AssertReachZero(_screenDuDvTexture);

    Assert(_postProcessParams);
    _postProcessParams->Destroy(device);
    RemoveRef_AssertReachZero(_postProcessParams);

    Assert(_postProcessEffect);
    _postProcessEffect->Destroy(device);
    RemoveRef_AssertReachZero(_postProcessEffect);

    Assert(_fullscreenQuadVertexBuffer);
    _fullscreenQuadVertexBuffer->Destroy(device);
    RemoveRef_AssertReachZero(_fullscreenQuadVertexBuffer);

    Assert(_renderTarget);
    _renderTarget->Destroy(device);
    RemoveRef_AssertReachZero(_renderTarget);

    Assert(_bumpTexture);
    _bumpTexture->Destroy(device);
    RemoveRef_AssertReachZero(_bumpTexture);

    Assert(_diffuseTexture);
    _diffuseTexture->Destroy(device);
    RemoveRef_AssertReachZero(_diffuseTexture);

    Assert(_indexBuffer);
    _indexBuffer->Destroy(device);
    RemoveRef_AssertReachZero(_indexBuffer);

    Assert(_vertexBuffer);
    _vertexBuffer->Destroy(device);
    RemoveRef_AssertReachZero(_vertexBuffer);

    Assert(_perObjectBuffer);
    _perObjectBuffer->Destroy(device);
    RemoveRef_AssertReachZero(_perObjectBuffer);

    Assert(_perFrameBuffer);
    _perFrameBuffer->Destroy(device);
    RemoveRef_AssertReachZero(_perFrameBuffer);

    Assert(_shaderEffect);
    _shaderEffect->Destroy(device);
    RemoveRef_AssertReachZero(_shaderEffect);
}
//----------------------------------------------------------------------------
void GameTest::Update(const Timeline& time) {
    parent_type::Update(time);

    using namespace Graphics;
    using namespace Application;

    const Graphics::DeviceEncapsulator& encapsulator = *DeviceEncapsulator();

    IDeviceAPIEncapsulator *const device = encapsulator.Device();
    IDeviceAPIContextEncapsulator *const context = encapsulator.Context();

    if (Keyboard().IsKeyPressed(KeyboardKey::Control) &&
        Keyboard().IsKeyUp(KeyboardKey::F8)) {
        LOG(Information, L"[Shaders] recompile effect ...");

        Assert(_shaderEffect);
        _shaderEffect->Destroy(device);
        RemoveRef_AssertReachZero(_shaderEffect);

        _shaderEffect = CompileEffect_VS_PS_(
            device,
            encapsulator.Compiler(),
            Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Declaration,
            L"GameData:/Shaders/Basic_old.fx");

        Assert(_postProcessEffect);
        _postProcessEffect->Destroy(device);
        RemoveRef_AssertReachZero(_postProcessEffect);

        _postProcessEffect = CompileEffect_VS_PS_(device, encapsulator.Compiler(), Vertex::Position0_Half2::Declaration, L"GameData:/Shaders/Postprocess.fx");
    }

    if (Keyboard().IsKeyUp(KeyboardKey::F11)) {
        LOG(Information, L"[Shaders] toggle wireframe {0:A} -> {1:A}", _wireframe, !_wireframe);
        _wireframe = !_wireframe;
    }

    _camera->Update(time);

    const float3 eyePosition = _camera->Model().Parameters().Position;
    const float4x4 view = _camera->Model().View();
    const float4x4 projection = _camera->Model().Projection();

    // per frame object
    {
        const void *constantBufferData[] = {&view, &projection, &eyePosition, &_sunLightDirection};
        _perFrameBuffer->SetData(device, MakeView(constantBufferData));
    }
    // postprocess params
    {
        const float4 sceneTexture_DuDvDimensions = _renderTarget->DuDvDimensions();
        const float timelineInSeconds = static_cast<float>(Units::Time::Seconds(time.Total()).Value());

        const void *constantBufferData[] = {&sceneTexture_DuDvDimensions, &timelineInSeconds};
        _postProcessParams->SetData(device, MakeView(constantBufferData));
    }
}
//----------------------------------------------------------------------------
void GameTest::Draw(const Timeline& time) {
    parent_type::Draw(time);

    using namespace Graphics;

    const Graphics::DeviceEncapsulator& encapsulator = *DeviceEncapsulator();

    IDeviceAPIEncapsulator *const device = encapsulator.Device();
    IDeviceAPIContextEncapsulator *const context = encapsulator.Context();

    //context->SetRenderTarget(_renderTarget, device->BackBufferDepthStencil());
    {
        context->Clear(device->BackBufferRenderTarget(), HSV_to_RGB(_clearColor.Eval(time)));
        //context->Clear(_renderTarget, HSV_to_RGB(_clearColor.Eval(time)));
        context->Clear(device->BackBufferDepthStencil(), ClearOptions::DepthStencil, 1, 0);

        context->SetBlendState(BlendState::Opaque);
        context->SetDepthStencilState(DepthStencilState::Default);
        context->SetRasterizerState(_wireframe ? RasterizerState::Wireframe : RasterizerState::CullCounterClockwise);

        context->SetShaderEffect(_shaderEffect);

        context->SetConstantBuffer(ShaderProgramType::Vertex, 0, _perFrameBuffer);
        context->SetConstantBuffer(ShaderProgramType::Pixel, 0, _perFrameBuffer);

        context->SetSamplerState(ShaderProgramType::Pixel, 0, SamplerState::LinearClamp);
        context->SetTexture(ShaderProgramType::Pixel, 0, _diffuseTexture);

        context->SetSamplerState(ShaderProgramType::Pixel, 1, SamplerState::LinearClamp);
        context->SetTexture(ShaderProgramType::Pixel, 1, _bumpTexture);

        float4x4 world = Make3DRotationMatrix(Normalize3(float3(0,30,1)), _rotationAngle.Eval(time));
        float4x4 worldIT = InvertTranspose(world);

        const void *constantBufferData[] = {&world, &worldIT};
        _perObjectBuffer->SetData(device, MakeView(constantBufferData));

        context->SetConstantBuffer(ShaderProgramType::Vertex, 1, _perObjectBuffer);
        context->SetConstantBuffer(ShaderProgramType::Pixel, 1, _perObjectBuffer);

        context->SetIndexBuffer(_indexBuffer);
        context->SetVertexBuffer(_vertexBuffer);

        context->DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, _indexBuffer->IndexCount() / 3);
    }
    /*context->SetRenderTarget(device->BackBufferRenderTarget(), nullptr);
    {
        context->SetBlendState(BlendState::Opaque);
        context->SetDepthStencilState(DepthStencilState::None);
        context->SetRasterizerState(RasterizerState::CullNone);

        context->SetVertexBuffer(_fullscreenQuadVertexBuffer);
        context->SetShaderEffect(_postProcessEffect);

        context->SetConstantBuffer(ShaderProgramType::Pixel, 0, _postProcessParams);

        context->SetTexture(ShaderProgramType::Pixel, 0, _renderTarget);
        context->SetTexture(ShaderProgramType::Pixel, 1, _screenDuDvTexture);

        context->DrawPrimitives(PrimitiveType::TriangleStrip, 0, 2);
    }*/
}
//----------------------------------------------------------------------------
void GameTest::Present() {
    parent_type::Present();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
