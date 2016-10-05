#include "stdafx.h"

#include "GameTest.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"

#include "Core.Graphics/Device/PrimitiveType.h"
#include "Core.Graphics/Device/PresentationParameters.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"

#include "Core.Graphics/Device/State/BlendState.h"
#include "Core.Graphics/Device/State/DepthStencilState.h"
#include "Core.Graphics/Device/State/RasterizerState.h"

#include "Core.Graphics/Device/Texture/DepthStencil.h"
#include "Core.Graphics/Device/Texture/RenderTarget.h"

#include "Core.Graphics/Device/IndexBuffer.h"
#include "Core.Graphics/Device/VertexBuffer.h"
#include "Core.Graphics/Device/VertexDeclaration.h"

#include "Core.Graphics/Device/Shader/ShaderEffect.h"
#include "Core.Graphics/Device/Shader/ShaderProgram.h"
#include "Core.Graphics/Device/Shader/ShaderSource.h"

#include "Core.Graphics/Device/Shader/ConstantBuffer.h"
#include "Core.Graphics/Device/Shader/ConstantBufferLayout.h"

#include "Core.Graphics/Device/State/SamplerState.h"
#include "Core.Graphics/Device/Texture/RenderTarget.h"
#include "Core.Graphics/Device/Texture/Texture2D.h"

#include "Core.Engine/Mesh/GenericVertex.h"
#include "Core.Engine/Mesh/GenericVertexOptimizer.h"
#include "Core.Engine/Mesh/GeometricPrimitives.h"
#include "Core.Engine/Mesh/Loader/MeshLoader.h"

#include "Core.Engine/Texture/TextureLoader.h"

#include "Core.Engine/Camera/Camera.h"
#include "Core.Engine/Camera/CameraController.h"
#include "Core.Engine/Camera/CameraModel.h"
#include "Core.Engine/Input/Camera/KeyboardMouseCameraController.h"

#include "Core/Container/RawStorage.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Maths/ScalarRectangle.h"
#include "Core/Maths/PackedVectors.h"
#include "Core/Maths/RandomGenerator.h"
#include "Core/Maths/ScalarMatrixHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static Graphics::FShaderEffect *CompileEffect_VS_PS_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::IDeviceAPIShaderCompiler *compiler,
    const Graphics::FVertexDeclaration *vertexDeclaration,
    const FFilename& sourceName ) {
    using namespace Graphics;

    const TMemoryView<const TPair<FString, FString>> defines;
    const EShaderCompilerFlags compilerFlags =
#ifdef _DEBUG
        EShaderCompilerFlags::DefaultForDebug;
#else
        EShaderCompilerFlags::Default;
#endif

    PShaderProgram vertexProgram = new FShaderProgram(EShaderProfileType::ShaderModel4_1, EShaderProgramType::Vertex);
    vertexProgram->Freeze();
    CompileShaderProgram(compiler, vertexProgram, "VSMain", compilerFlags, sourceName, vertexDeclaration, defines);

    PShaderProgram pixelProgram = new FShaderProgram(EShaderProfileType::ShaderModel4_1, EShaderProgramType::Pixel);
    pixelProgram->Freeze();
    CompileShaderProgram(compiler, pixelProgram, "PSMain", compilerFlags, sourceName, vertexDeclaration, defines);

    FShaderEffect *const effect = new FShaderEffect(vertexDeclaration);
    effect->SetStageProgram(EShaderProgramType::Vertex, std::move(vertexProgram));
    effect->SetStageProgram(EShaderProgramType::Pixel, std::move(pixelProgram));
    effect->Freeze();
    effect->Create(device);

    return effect;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGameTest::FGameTest(const wchar_t *appname)
:   parent_type(
    appname,
    Graphics::EDeviceAPI::DirectX11,
    Graphics::FPresentationParameters(
        1024, 768,
        Graphics::FSurfaceFormat::R8G8B8A8_SRGB,
        Graphics::FSurfaceFormat::D24S8,
        false,
        true,
        0,
        Graphics::EPresentInterval::Default ),
    10, 10)
,   _clearColor(
        RGB_to_HSV(Color::IndianRed.Data()),
        RGB_to_HSV(Color::LawnGreen.Data())
    )
,   _rotationAngle(-F_PI, F_PI)
{ }
//----------------------------------------------------------------------------
FGameTest::~FGameTest() {}
//----------------------------------------------------------------------------
void FGameTest::Initialize(const FTimeline& time) {
    parent_type::Initialize(time);

    FVirtualFileSystem::Instance().MountNativePath(L"GameData:/", L"D:/Dropbox/code/cpp/DXCPP/Core/Data");

    using namespace Engine;
    using namespace Graphics;

    _clearColor.Start(time, Units::Time::Seconds(3));
    _rotationAngle.Start(time, Units::Time::Minutes(0.5));

    const ViewportF& viewport = FDeviceEncapsulator().Parameters().Viewport();

    _camera = new PerspectiveCamera(F_PIOver3, 0.01f, 100.0f, viewport);
    _cameraController = new Engine::FKeyboardMouseCameraController(float3(0.0f, 3.0f, -6.0f), 0.0f, 0.5f*F_PIOver3, &Keyboard(), &Mouse());
    _camera->SetController(_cameraController);

    _sunLightDirection = Normalize3(float3( 0,1,-3));
}
//----------------------------------------------------------------------------
void FGameTest::Destroy() {
    parent_type::Destroy();

    _camera->SetController(nullptr);
    RemoveRef_AssertReachZero(_cameraController);
    RemoveRef_AssertReachZero(_camera);
}
//----------------------------------------------------------------------------
void FGameTest::LoadContent() {
    parent_type::LoadContent();

    Assert(!_shaderEffect);

    using namespace Engine;
    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = FDeviceEncapsulator().Device();
    IDeviceAPIShaderCompiler *const compiler = FDeviceEncapsulator().Compiler();

    const FVertexDeclaration *vertexDeclaration = Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Declaration;

    _shaderEffect = CompileEffect_VS_PS_(device, compiler, vertexDeclaration, L"GameData:/Shaders/Basic_old.fx");

    FConstantBufferLayout *const layoutPerFrame = new FConstantBufferLayout();
    layoutPerFrame->AddField("View", EConstantFieldType::Float4x4);
    layoutPerFrame->AddField("Projection", EConstantFieldType::Float4x4);
    layoutPerFrame->AddField("EyePosition", EConstantFieldType::Float3);
    layoutPerFrame->AddField("LightDir", EConstantFieldType::Float3);

    _perFrameBuffer = new FConstantBuffer(layoutPerFrame);
    _perFrameBuffer->SetResourceName("PerFrameBuffer");
    _perFrameBuffer->Freeze();
    _perFrameBuffer->Create(device);

    FConstantBufferLayout *const layoutPerObject = new FConstantBufferLayout();
    layoutPerObject->AddField("FWorld", EConstantFieldType::Float4x4);
    layoutPerObject->AddField("WorldInvertT", EConstantFieldType::Float4x4);

    _perObjectBuffer = new FConstantBuffer(layoutPerObject);
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
        FRandomGenerator rnd;
        for (ColorRGBA& color : colors)
            color = ColorRGBAF(rnd.NextFloat01(),rnd.NextFloat01(),rnd.NextFloat01(),1);
    }

    RAWSTORAGE_ALIGNED(Geometry, u8, 16) exportVertices;
    ExportVertices(
        exportVertices,
        Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Declaration,
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
        Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Declaration,
        vertexCount,
        MakeView(indices) );

    const float ACMR1 = VertexAverageCacheMissRate(MakeView(indices));

    LOG(Warning, L"Optimized mesh average cache miss rate from {0}% to {1}%", ACMR0, ACMR1);

    RAWSTORAGE_ALIGNED(Geometry, u16, 16) exportIndices;
    exportIndices.Resize_DiscardData(indexCount);
    for (size_t i = 0; i < indexCount; ++i)
        exportIndices[i] = checked_cast<u16>(indices[i]);

    _vertexBuffer = new FVertexBuffer(   Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Declaration,
                                        vertexCount, EBufferMode::None, EBufferUsage::Default );
    _vertexBuffer->Freeze();
    _vertexBuffer->Create(device, exportVertices.MakeView().Cast<const u8>());

    _indexBuffer = new IndexBuffer(IndexElementSize::SixteenBits, indexCount, EBufferMode::None, EBufferUsage::Default);
    _indexBuffer->Freeze();
    _indexBuffer->Create(device, exportIndices.MakeView().Cast<const u16>());

#else

    TMeshLoader<u32, Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N> mesh;
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

    _diffuseTexture = checked_cast<FTexture2D *>(FTexture2DLoader::Load(device, L"GameData:/Models/dragon_40k_ao.dds", true));
    AssertRelease(_diffuseTexture);

    _bumpTexture = checked_cast<FTexture2D *>(FTexture2DLoader::Load(device, L"GameData:/Models/dragon_40k_bump.dds", false));
    AssertRelease(_bumpTexture);

    _screenDuDvTexture = checked_cast<FTexture2D *>(FTexture2DLoader::Load(device, L"GameData:/Textures/GrassDuDv.dds", false));
    AssertRelease(_screenDuDvTexture);

    const ViewportF& viewport = FDeviceEncapsulator().Parameters().Viewport();

    _renderTarget = new FRenderTarget(size_t(viewport.Width()), size_t(viewport.Height()), FSurfaceFormat::R16G16B16A16_F);
    _renderTarget->SetResourceName("SceneBuffer");
    _renderTarget->Freeze();
    _renderTarget->Create(device);

    ALIGN(16) const Vertex::FPosition0_Half2 fullscreenVertices[] = {
        {half2(FHalfFloat::MinusOne, FHalfFloat::MinusOne)},
        {half2(FHalfFloat::MinusOne, FHalfFloat::One)},
        {half2(FHalfFloat::One,      FHalfFloat::MinusOne)},
        {half2(FHalfFloat::One,      FHalfFloat::One)},
    };

    _fullscreenQuadVertexBuffer = new FVertexBuffer(Vertex::FPosition0_Half2::Declaration, lengthof(fullscreenVertices), EBufferMode::None, EBufferUsage::Default);
    _fullscreenQuadVertexBuffer->Freeze();
    _fullscreenQuadVertexBuffer->Create(device, MakeView(fullscreenVertices));

    _postProcessEffect = CompileEffect_VS_PS_(device, compiler, Vertex::FPosition0_Half2::Declaration, L"GameData:/Shaders/Postprocess.fx");

    FConstantBufferLayout *const layoutPostProcess = new FConstantBufferLayout();
    layoutPostProcess->AddField("SceneTexture_DuDvDimensions", EConstantFieldType::Float4);
    layoutPostProcess->AddField("TimelineInSeconds", EConstantFieldType::Float);

    _postProcessParams = new FConstantBuffer(layoutPostProcess);
    _postProcessParams->SetResourceName("PostProcessParams");
    _postProcessParams->Freeze();
    _postProcessParams->Create(device);
}
//----------------------------------------------------------------------------
void FGameTest::UnloadContent() {
    parent_type::UnloadContent();

    using namespace Graphics;

    IDeviceAPIEncapsulator *const device = FDeviceEncapsulator().Device();

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
void FGameTest::Update(const FTimeline& time) {
    parent_type::Update(time);

    using namespace Graphics;
    using namespace Engine;
    using namespace Application;

    const Graphics::FDeviceEncapsulator& encapsulator = FDeviceEncapsulator();

    IDeviceAPIEncapsulator *const device = encapsulator.Device();
    IDeviceAPIContext *const context = encapsulator.Context();

    if (Keyboard().IsKeyPressed(EKeyboardKey::Control) &&
        Keyboard().IsKeyUp(EKeyboardKey::F8)) {
        LOG(Info, L"[Shaders] recompile effect ...");

        Assert(_shaderEffect);
        _shaderEffect->Destroy(device);
        RemoveRef_AssertReachZero(_shaderEffect);

        _shaderEffect = CompileEffect_VS_PS_(
            device,
            encapsulator.Compiler(),
            Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Declaration,
            L"GameData:/Shaders/Basic_old.fx");

        Assert(_postProcessEffect);
        _postProcessEffect->Destroy(device);
        RemoveRef_AssertReachZero(_postProcessEffect);

        _postProcessEffect = CompileEffect_VS_PS_(device, encapsulator.Compiler(), Vertex::FPosition0_Half2::Declaration, L"GameData:/Shaders/Postprocess.fx");
    }

    if (Keyboard().IsKeyUp(EKeyboardKey::F11)) {
        LOG(Info, L"[Shaders] toggle wireframe {0:A} -> {1:A}", _wireframe, !_wireframe);
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
void FGameTest::Draw(const FTimeline& time) {
    parent_type::Draw(time);

    using namespace Graphics;

    const Graphics::FDeviceEncapsulator& encapsulator = FDeviceEncapsulator();

    IDeviceAPIEncapsulator *const device = encapsulator.Device();
    IDeviceAPIContext *const context = encapsulator.Context();

    //context->SetRenderTarget(_renderTarget, device->BackBufferDepthStencil());
    {
        context->Clear(device->BackBufferRenderTarget(), HSV_to_RGB(_clearColor.Eval(time)));
        //context->Clear(_renderTarget, HSV_to_RGB(_clearColor.Eval(time)));
        context->Clear(device->BackBufferDepthStencil(), EClearOptions::FDepthStencil, 1, 0);

        context->SetBlendState(FBlendState::Opaque);
        context->SetDepthStencilState(FDepthStencilState::Default);
        context->SetRasterizerState(_wireframe ? FRasterizerState::Wireframe : FRasterizerState::CullCounterClockwise);

        context->SetShaderEffect(_shaderEffect);

        context->SetConstantBuffer(EShaderProgramType::Vertex, 0, _perFrameBuffer);
        context->SetConstantBuffer(EShaderProgramType::Pixel, 0, _perFrameBuffer);

        context->SetSamplerState(EShaderProgramType::Pixel, 0, FSamplerState::LinearClamp);
        context->SetTexture(EShaderProgramType::Pixel, 0, _diffuseTexture);

        context->SetSamplerState(EShaderProgramType::Pixel, 1, FSamplerState::LinearClamp);
        context->SetTexture(EShaderProgramType::Pixel, 1, _bumpTexture);

        float4x4 world = Make3DRotationMatrix(Normalize3(float3(0,30,1)), _rotationAngle.Eval(time));
        float4x4 worldIT = InvertTranspose(world);

        const void *constantBufferData[] = {&world, &worldIT};
        _perObjectBuffer->SetData(device, MakeView(constantBufferData));

        context->SetConstantBuffer(EShaderProgramType::Vertex, 1, _perObjectBuffer);
        context->SetConstantBuffer(EShaderProgramType::Pixel, 1, _perObjectBuffer);

        context->SetIndexBuffer(_indexBuffer);
        context->SetVertexBuffer(_vertexBuffer);

        context->DrawIndexedPrimitives(EPrimitiveType::TriangleList, 0, 0, _indexBuffer->IndexCount() / 3);
    }
    /*context->SetRenderTarget(device->BackBufferRenderTarget(), nullptr);
    {
        context->SetBlendState(FBlendState::Opaque);
        context->SetDepthStencilState(FDepthStencilState::None);
        context->SetRasterizerState(FRasterizerState::CullNone);

        context->SetVertexBuffer(_fullscreenQuadVertexBuffer);
        context->SetShaderEffect(_postProcessEffect);

        context->SetConstantBuffer(EShaderProgramType::Pixel, 0, _postProcessParams);

        context->SetTexture(EShaderProgramType::Pixel, 0, _renderTarget);
        context->SetTexture(EShaderProgramType::Pixel, 1, _screenDuDvTexture);

        context->DrawPrimitives(EPrimitiveType::TriangleStrip, 0, 2);
    }*/
}
//----------------------------------------------------------------------------
void FGameTest::Present() {
    parent_type::Present();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
