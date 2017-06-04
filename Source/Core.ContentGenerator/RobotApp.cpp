#include "stdafx.h"

#include "RobotApp.h"

#include "Core/Color/Color.h"
#include "Core/Diagnostic/DialogBox.h"
#include "Core/Maths/Maths.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"

#include "Core.Graphics/Device/ShaderCompilerEncapsulator.h"
#include "Core.Graphics/Device/Geometry/IndexElementSize.h"
#include "Core.Graphics/Device/Geometry/IndexBuffer.h"
#include "Core.Graphics/Device/Geometry/PrimitiveType.h"
#include "Core.Graphics/Device/Geometry/VertexBuffer.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"
#include "Core.Graphics/Device/Geometry/VertexTypes.h"
#include "Core.Graphics/Device/Shader/ConstantBuffer.h"
#include "Core.Graphics/Device/Shader/ConstantBufferLayout.h"
#include "Core.Graphics/Device/Shader/ShaderCompiled.h"
#include "Core.Graphics/Device/Shader/ShaderEffect.h"
#include "Core.Graphics/Device/ShaderCompilerEncapsulator.h"
#include "Core.Graphics/Device/State/BlendState.h"
#include "Core.Graphics/Device/State/DepthStencilState.h"
#include "Core.Graphics/Device/State/RasterizerState.h"
#include "Core.Graphics/Device/State/SamplerState.h"

#include "Core.Lattice/BulkMesh.h"
#include "Core.Lattice/GenericMesh.h"
#include "Core.Lattice/GenericMeshHelpers.h"
#include "Core.Lattice/WaveFrontObj.h"

#include "Core.RTTI/RTTI_Macros-impl.h"
#include "Core.RTTI/RTTI_Namespace-impl.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
#include "Core/Diagnostic/CurrentProcess.h"
#include "Core/IO/FS/ConstNames.h"
#include "Core/Maths/QuaternionHelpers.h"
#include "Core/Maths/ScalarVectorHelpers.h"
#include "Core/Maths/ScalarMatrixHelpers.h"
#include "Core/Maths/Transform.h"
#include "Core/Memory/Compression.h"

#include "Core.Application/ApplicationConsole.h"
#include "Core.Application/Input/GamepadInputHandler.h"

namespace Core {
namespace ContentGenerator {

extern void Test_Format();
extern void Test_Containers();
extern void Test_Lattice();
extern void Test_Network();
extern void Test_Pixmap();
extern void Test_RTTI();
extern void Test_Thread();
extern void Test_XML();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_NAMESPACE_DECL(, RobotApp);
RTTI_NAMESPACE_DEF(, RobotApp);
class FRTTIConstantBindings {
public:
    FRTTIConstantBindings() : _layout(nullptr), _emptyBindings(0) {}
    explicit FRTTIConstantBindings(const Graphics::FConstantBufferLayout* layout)
        : FRTTIConstantBindings() {
        Initialize(layout);
    }

    bool IsReady() const { return (_layout && 0 == _emptyBindings); }
    bool IsValid() const { return (_layout != nullptr); }

    void Initialize(const Graphics::FConstantBufferLayout* layout) {
        Assert(layout);

        _layout = layout;
        _emptyBindings = _layout->size();

        _bindings.clear();
        _bindings.resize(_layout->size(), nullptr);

#ifdef WITH_CORE_ASSERT
        _bindeds.clear();
        _bindeds.resize(_layout->size(), nullptr);
#endif
    }

    size_t Bind(const RTTI::FMetaObject& obj) {
        Assert(_layout);

        const auto fields = _layout->Fields();
        const RTTI::FMetaClass* metaClass = obj.RTTI_MetaClass();

        _emptyBindings = 0;

        forrange(i, 0, _bindings.size()) {
            if (const RTTI::FMetaProperty* prop = metaClass->PropertyIFP(fields[i].Name().MakeView())) {
                Graphics::EValueType type = RTTI::GraphicsValueType(prop->TypeInfo().Id);
                if (type != Graphics::EValueType::Void) {
                    _bindings[i] = prop->RawPtr(&obj);
#ifdef WITH_CORE_ASSERT
                    // check obj lifetime : shouldn't die while binded here
                    _bindeds[i].reset(&obj);
#endif
                    continue;
                }
            }

            _emptyBindings++;
        }
        Assert(_emptyBindings < _bindings.size());

        return _emptyBindings;
    }

    void SetData(Graphics::IDeviceAPIEncapsulator* device, Graphics::FConstantBuffer* buffer) const {
        Assert(buffer);
        Assert(buffer->Layout() == _layout);
        Assert(IsReady());

        buffer->SetData(device, _bindings.MakeView());
    }

private:
    Graphics::PCConstantBufferLayout _layout;
    size_t _emptyBindings;

    VECTORINSITU(Engine, const void*, 8) _bindings;
#ifdef WITH_CORE_ASSERT
    VECTORINSITU(Engine, RTTI::SCMetaObject, 8) _bindeds;
#endif
};
class UConstantData : public RTTI::FMetaObject {
public:
    RTTI_CLASS_HEADER(UConstantData, RTTI::FMetaObject);

    float4x4    uniWorld;
    float4x4    uniInvertTranspose_World;
    float4x4    uniView;
    float4x4    uniProjection;
    float3      uniEyePosition;
    float3      uniSunDirection;
    float3      uniSRGB_uniSunColor;
    float       uniProcessTotalSeconds;
};
RTTI_CLASS_BEGIN(RobotApp, UConstantData, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(uniWorld)
RTTI_PROPERTY_PUBLIC_FIELD(uniInvertTranspose_World)
RTTI_PROPERTY_PUBLIC_FIELD(uniView)
RTTI_PROPERTY_PUBLIC_FIELD(uniProjection)
RTTI_PROPERTY_PUBLIC_FIELD(uniEyePosition)
RTTI_PROPERTY_PUBLIC_FIELD(uniSunDirection)
RTTI_PROPERTY_PUBLIC_FIELD(uniSRGB_uniSunColor)
RTTI_PROPERTY_PUBLIC_FIELD(uniProcessTotalSeconds)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
class FRobotAppImpl
{
public:
    UConstantData ConstantData;
    FRTTIConstantBindings ConstantBindings;

    Graphics::FShaderCompilerEncapsulator ShaderCompiler;

    const Graphics::FVertexDeclaration* VertexDeclaration = nullptr;

    size_t PrimitiveCount = 0;
    Graphics::PIndexBuffer Indices;
    Graphics::PVertexBuffer Vertices;

    FFilename ShaderFilename;
    Graphics::PShaderEffect ShaderEffect;
    Graphics::PConstantBuffer ConstantBuffer;
    Graphics::PCConstantBufferLayout ConstantBufferLayout;

    Graphics::PShaderProgram PixelShader;
    Graphics::PShaderCompiled PixelShaderCompiled;

    Graphics::PShaderProgram VertexShader;
    Graphics::PShaderCompiled VertexShaderCompiled;

    FRobotAppImpl() {
        ShaderCompiler.Create(Graphics::EDeviceAPI::DirectX11);
    }

    ~FRobotAppImpl() {
        Assert(!Indices);
        Assert(!Vertices);
        Assert(!ShaderEffect);
        Assert(!ConstantBuffer);
        Assert(!ConstantBufferLayout);

        ShaderCompiler.Destroy();
    }

    void Update(Graphics::IDeviceAPIEncapsulator* device, float fov, float aspectRatio, float rotx, float roty, float scale)
    {
        const float3 target = float3(0, 0, 3.5f);
        const float3 eye = float3(0, 1, 0);
        const float3 sunDir = Normalize3(float3(1.0f, 0.9f,-0.7f));
        const float3 sunColor = float3(0.9f, 0.6f, 0.3f);

        const FQuaternion mx = MakeAxisQuaternion(float3::X(), roty);
        const FQuaternion my = MakeAxisQuaternion(float3::Y(), rotx);

        ConstantData.uniWorld = Make3DTransformMatrix(target, float3(scale), mx * my);
        ConstantData.uniInvertTranspose_World = InvertTranspose(ConstantData.uniWorld);
        ConstantData.uniEyePosition = eye;
        ConstantData.uniSunDirection = sunDir;
        ConstantData.uniSRGB_uniSunColor = sunColor;
        ConstantData.uniProcessTotalSeconds = (float)*CurrentProcess().ElapsedSeconds();

        ConstantData.uniView = MakeLookAtLHMatrix(eye, target, float3(0, 1, 0));
        ConstantData.uniProjection = MakePerspectiveFovLHMatrix(Radians(fov), aspectRatio, 0.001f, 10.0f);

        ConstantBindings.SetData(device, ConstantBuffer.get());
    }

    void Draw(Graphics::IDeviceAPIContext* context) {
        if (Indices && Vertices && ShaderEffect && ConstantBuffer)
        {
            Assert(ConstantBindings.IsReady());

            GRAPHICS_DIAGNOSTICS_SCOPEEVENT(context->DeviceDiagnostics(), ShaderEffect->ResourceName());

            context->SetShaderEffect(ShaderEffect.get());
            context->SetIndexBuffer(Indices.get());
            context->SetVertexBuffer(Vertices.get());
            context->SetConstantBuffer(Graphics::EShaderProgramType::Vertex, 0, ConstantBuffer.get());
            context->SetConstantBuffer(Graphics::EShaderProgramType::Pixel, 0, ConstantBuffer.get());
            context->DrawIndexedPrimitives(Graphics::EPrimitiveType::TriangleList, 0, 0, PrimitiveCount);
        }
    }

    void SetShader(Graphics::IDeviceAPIEncapsulator* device, const FFilename& filename) {
        ResetShader(device);

        if (VertexDeclaration) {
            ShaderEffect = new Graphics::FShaderEffect(VertexDeclaration);
            ShaderEffect->SetResourceName(filename.BasenameNoExt().MakeView());

            const Graphics::EShaderProfileType shaderProfile = Graphics::EShaderProfileType::ShaderModel5;

            const Graphics::EShaderCompilerFlags compilerFlags =
#if _DEBUG
                Graphics::EShaderCompilerFlags::DefaultForDebug
#else
                Graphics::EShaderCompilerFlags::Default
#endif
                ;

            const TMemoryView<const TPair<FString, FString>> compilerDefines;

            ConstantBufferLayout.reset();

            {
                VertexShaderCompiled = Graphics::CompileShaderSource(&ShaderCompiler, filename, VertexDeclaration,
                    Graphics::EShaderProgramType::Vertex,
                    shaderProfile, compilerFlags, "vmain", compilerDefines);

                VertexShader = new Graphics::FShaderProgram(
                    VertexDeclaration,
                    Graphics::EShaderProgramType::Vertex,
                    shaderProfile, VertexShaderCompiled.get(), false);
                VertexShader->Freeze();
                VertexShader->Create(device);

                Assert(VertexShaderCompiled->Constants().size() == 1);
                ConstantBufferLayout = VertexShaderCompiled->Constants().Vector().front().second;

                ShaderEffect->SetStageProgram(Graphics::EShaderProgramType::Vertex, VertexShader.get());
            }

            {
                PixelShaderCompiled = Graphics::CompileShaderSource(&ShaderCompiler, filename, VertexDeclaration,
                    Graphics::EShaderProgramType::Pixel,
                    shaderProfile, compilerFlags, "pmain", compilerDefines);

                PixelShader = new Graphics::FShaderProgram(
                    VertexDeclaration,
                    Graphics::EShaderProgramType::Pixel,
                    shaderProfile, PixelShaderCompiled.get(), false);
                PixelShader->Freeze();
                PixelShader->Create(device);

                Assert(PixelShaderCompiled->Constants().size() == 1);
                Graphics::PCConstantBufferLayout constantLayout2 = PixelShaderCompiled->Constants().Vector().front().second;
                Assert(constantLayout2->Equals(*ConstantBufferLayout));

                ShaderEffect->SetStageProgram(Graphics::EShaderProgramType::Pixel, PixelShader.get());
            }

            {
                ConstantBuffer = new Graphics::FConstantBuffer(ConstantBufferLayout.get(), true);
                ConstantBuffer->Freeze();
                ConstantBuffer->Create(device);
            }

            ShaderEffect->Freeze();
            ShaderEffect->Create(device);

            ConstantBindings.Initialize(ConstantBufferLayout.get());
            ConstantBindings.Bind(ConstantData);
        }
    }

    bool SetMesh(Graphics::IDeviceAPIEncapsulator* device, const Lattice::FGenericMesh& mesh) {
        ResetMesh(device);

        bool needShaderCompile = false;

        PrimitiveCount = mesh.TriangleCount();
        {
            const Graphics::FVertexDeclaration* prevVDecl = VertexDeclaration;

            VertexDeclaration = nullptr;
            if (mesh.Position3f_IFP(0)) {
                VertexDeclaration = Graphics::Vertex::FPosition0_Float3::Declaration;
                if (mesh.Normal3f_IFP(0)) {
                    VertexDeclaration = Graphics::Vertex::FPosition0_Float3__Normal0_UX10Y10Z10W2N::Declaration;
                    if (mesh.TexCoord2f_IFP(0)) {
                        VertexDeclaration = Graphics::Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Declaration;
                        if (mesh.Color4f_IFP(0)) {
                            VertexDeclaration = Graphics::Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Declaration;
                        }
                    }
                }
            }
            else {
                AssertNotReached();
            }

            if (prevVDecl != VertexDeclaration) {
                ResetShader(device);
                needShaderCompile = true;
            }
        }
        {
            RAWSTORAGE_THREAD_LOCAL(Vertex, u8) vertexData;
            mesh.ExportVertices(VertexDeclaration, vertexData);

            Vertices = new Graphics::FVertexBuffer(VertexDeclaration, mesh.VertexCount(),
                Graphics::EBufferMode::None, Graphics::EBufferUsage::Default, true);
            Vertices->Freeze();
            Vertices->Create(device, vertexData.MakeConstView());
        }
        {
            RAWSTORAGE_THREAD_LOCAL(Index, u32) indexData;
            mesh.ExportIndices(indexData);

            Indices = new Graphics::FIndexBuffer(Graphics::EIndexElementSize::ThirtyTwoBits, mesh.IndexCount(),
                Graphics::EBufferMode::None, Graphics::EBufferUsage::Default, true);
            Indices->Freeze();
            Indices->Create(device, indexData.MakeConstView());
        }

        return needShaderCompile;
    }

    void ResetShader(Graphics::IDeviceAPIEncapsulator* device) {
        if (ShaderEffect) {
            ShaderEffect->Destroy(device);
            ShaderEffect.reset();

            PixelShader->Destroy(device);
            PixelShader.reset();

            VertexShader->Destroy(device);
            VertexShader.reset();
        }
        if (ConstantBuffer) {
            ConstantBuffer->Destroy(device);
            ConstantBuffer.reset();
        }
        ConstantBufferLayout.reset();
        PixelShaderCompiled.reset();
        VertexShaderCompiled.reset();
    }

    void ResetMesh(Graphics::IDeviceAPIEncapsulator* device) {
        PrimitiveCount = 0;

        if (Indices) {
            Indices->Destroy(device);
            Indices.reset();
        }
        if (Vertices) {
            Vertices->Destroy(device);
            Vertices.reset();
        }
    }

    void Destroy(Graphics::IDeviceAPIEncapsulator* device) {
        ResetMesh(device);
        ResetShader(device);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRobotApp::FRobotApp()
: parent_type(
    L"RobotApp", 100, 100,
    Graphics::FPresentationParameters(
        800, 600,
        Graphics::ESurfaceFormatType::B8G8R8A8_SRGB,
        Graphics::ESurfaceFormatType::D24S8,
        false,
        false,
        0,
        Graphics::EPresentInterval::Default ),
    Graphics::EDeviceAPI::DirectX11,
    true ) {

    Application::FApplicationConsole::RedirectIOToConsole();

    /*
    Test_Lattice();
    Test_Format();
    Test_Containers();
    Test_Pixmap();
    Test_RTTI();
    Test_Thread();
    Test_XML();
    Test_Network();
    */
}
//----------------------------------------------------------------------------
FRobotApp::~FRobotApp() {}
//----------------------------------------------------------------------------
void FRobotApp::Start() {
    parent_type::Start();

    Assert(!_pimpl);
    _pimpl.reset(new FRobotAppImpl());

    RTTI_NAMESPACE(RobotApp).Start();

    RenderLoop();

    Assert(_pimpl);
    _pimpl->Destroy(DeviceEncapsulator().Device());
    _pimpl.reset();

    RTTI_NAMESPACE(RobotApp).Shutdown();
}
//----------------------------------------------------------------------------
void FRobotApp::Shutdown() {
    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
void FRobotApp::Draw(const FTimeline& time) {
    parent_type::Draw(time);

    using Application::EGamepadButton;

    GRAPHICS_DIAGNOSTICS_SCOPEEVENT(DeviceEncapsulator().Diagnostics(), L"FRobotApp::Draw");

    const double totalSeconds = FSeconds(time.Total()).Value();

    const auto* gamepadService = Services().Get<Application::IGamepadService>();
    const Application::FGamepadState& gamepad = gamepadService->State().First();

    float3 hsv(float(Frac(totalSeconds*0.1)), 1.0f, 0.5f);

    if (gamepad.IsConnected()) {
        static float3 p(0.5f);

        if (gamepad.RightTrigger().Raw()>0.5f) {
            p.x() += float(gamepad.LeftStickX().Raw()*time.Elapsed().Value()*0.0005f);
            p.y() += float(gamepad.LeftStickY().Raw()*time.Elapsed().Value()*0.0005f);
        }
        else {
            p.x() = hsv.x();
            p.y() = hsv.y();
        }

        p.z() += float(gamepad.RightStickY().Raw()*time.Elapsed().Value()*0.0005f);

        p = Saturate(p);

        hsv = p;
    }

    const float3 rgb = HSV_to_RGB(hsv);
    const FLinearColor clearColor(rgb, 1.0f);

    Graphics::IDeviceAPIEncapsulator* const device = DeviceEncapsulator().Device();
    Graphics::IDeviceAPIContext* const immediate = DeviceEncapsulator().Immediate();

    device->Clear(device->BackBufferDepthStencil(), Graphics::EClearOptions::All, 1, 0);
    device->Clear(device->BackBufferRenderTarget(), clearColor);

    static int GShaderCurrent = -1;
    static const FWStringView GShaders[] = {
        L"Data:/Shaders/Voxel.fx"
    };

    static int GMeshCurrent = -1;
    static const FWStringView GMeshes[] = {
        L"Data:/Models/Sphere.obj",
        L"Data:/Models/monkey.obj",
        L"Data:/Models/dragon_5k.obj",
        L"Data:/Models/Cerberus.obj",
        L"Data:/Models/Happy.obj",
    };

    bool wireframe = false;

    if (gamepad.IsConnected()) {
        bool needShaderCompile = false;
        if (GMeshCurrent < 0 || gamepad.IsButtonUp(EGamepadButton::Start)) {
            GMeshCurrent = (GMeshCurrent + 1) % lengthof(GMeshes);

            const FFilename srcFilename(GMeshes[GMeshCurrent]);

            LOG(Info, L"[RobotApp] Load mesh '{0}'", srcFilename);

            Lattice::FGenericMesh mesh;

            const FFilename bulkFilename = srcFilename.WithReplacedExtension(FFSConstNames::Bkmz());
            if (not Lattice::FBulkMesh::Load(&mesh, bulkFilename)) {
                Lattice::FWaveFrontObj::Load(&mesh, srcFilename);

                mesh.CleanAndOptimize();

                const FAabb3f bounds = Lattice::ComputeBounds(mesh, 0);
                const float4x4 transform =
                    MakeTranslationMatrix(-bounds.Center()) *
                    MakeScalingMatrix(float3(Rcp(Max(bounds.HalfExtents()))));

                Lattice::Transform(mesh, 0, transform);

                Lattice::FBulkMesh::Save(&mesh, bulkFilename);
            }

            needShaderCompile = _pimpl->SetMesh(device, mesh);
        }
        if (needShaderCompile || GShaderCurrent < 0 || gamepad.IsButtonUp(EGamepadButton::Back)) {
            GShaderCurrent = (GShaderCurrent + 1) % lengthof(GShaders);

            const FFilename srcFilename(GShaders[GShaderCurrent]);

            LOG(Info, L"[RobotApp] Load shader '{0}'", srcFilename);

            _pimpl->SetShader(device, srcFilename);
        }
        if (gamepad.IsButtonPressed(EGamepadButton::LS)) {
            wireframe = true;
        }
    }

    const float aspectRatio = DeviceEncapsulator().Parameters().Viewport().AspectRatio();

    float rotx  = 0;
    float roty  = 0;
    float fov   = 80;
    float scale = 1.0f;

    if (gamepad.IsConnected()) {
        if (gamepad.IsButtonPressed(EGamepadButton::RS)) {
            rotx  = Lerp(-F_PI, F_PI, gamepad.LeftStickX().Raw() * 0.5f + 0.5f);
            roty  = Lerp(-F_PI, F_PI, gamepad.LeftStickY().Raw() * 0.5f + 0.5f);
            fov   = Lerp( 40.f, 90.f, gamepad.RightTrigger().Raw());
            scale = Lerp( 1.0f, 2.0f, gamepad.LeftTrigger().Raw());
        }
        else {
            rotx  = Lerp(-F_PI, F_PI, gamepad.LeftStickX().Smoothed() * 0.5f + 0.5f);
            roty  = Lerp(-F_PI, F_PI, gamepad.LeftStickY().Smoothed() * 0.5f + 0.5f);
            fov   = Lerp( 40.f, 90.f, gamepad.RightTrigger().Smoothed());
            scale = Lerp( 1.0f, 2.0f, gamepad.LeftTrigger().Smoothed());
        }

        rotx += F_HalfPi;

        if (gamepad.IsButtonUp(EGamepadButton::LeftThumb)) {
            LOG(Info, L"[RobotApp] Triggering frame capture");
            device->DeviceDiagnostics()->LaunchProfilerAndTriggerCapture();
        }
    }

    immediate->SetBlendState(Graphics::FBlendState::Opaque);
    immediate->SetDepthStencilState(Graphics::FDepthStencilState::Default);
    immediate->SetRasterizerState(wireframe ? Graphics::FRasterizerState::Wireframe : Graphics::FRasterizerState::CullCounterClockwise);

    _pimpl->Update(device, fov, aspectRatio, rotx, roty, scale);
    _pimpl->Draw(immediate);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
