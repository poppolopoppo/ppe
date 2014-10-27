#pragma once

#include "Core/Color/Color.h"
#include "Core/Maths/Transform/ScalarMatrix.h"
#include "Core/Time/Timely.h"

#include "Core.Graphics/Device/DeviceAPIEncapsulator.h"

#include "Core.Application/Application.h"
#include "Core.Application/ApplicationWindow.h"

namespace Core {
namespace Graphics {
FWD_REFPTR(ConstantBuffer);
FWD_REFPTR(ShaderEffect);
FWD_REFPTR(ShaderProgram);
FWD_REFPTR(IndexBuffer);
FWD_REFPTR(VertexBuffer);
FWD_REFPTR(Texture2D);
FWD_REFPTR(RenderTarget);
}

namespace Engine {
FWD_REFPTR(PerspectiveCamera);
}

namespace Application {
FWD_REFPTR(KeyboardMouseCameraController);
}}

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class GameTest : public Core::Application::ApplicationWindow {
public:
    typedef Core::Application::ApplicationWindow parent_type;

    GameTest() : GameTest(L"Core Game Window Test Pipo Bimbo") {}
    explicit GameTest(const wchar_t *appname);
    virtual ~GameTest();

protected:
    virtual void Initialize(const Timeline& time) override;
    virtual void Destroy() override;

    virtual void LoadContent() override;
    virtual void UnloadContent() override;

    virtual void Update(const Timeline& time) override;
    virtual void Draw(const Timeline& time) override;

    virtual void Present() override;

private:
    Timely::PulsarSmoothstep<float4> _clearColor;
    Timely::PulsarSmoothstep<float> _rotationAngle;

    Engine::PPerspectiveCamera _camera;
    Application::PKeyboardMouseCameraController _cameraController;

    Graphics::PShaderEffect _shaderEffect;

    Graphics::PConstantBuffer _perFrameBuffer;
    Graphics::PConstantBuffer _perObjectBuffer;

    Graphics::PIndexBuffer _indexBuffer;
    Graphics::PVertexBuffer _vertexBuffer;

    Graphics::PTexture2D _diffuseTexture;
    Graphics::PTexture2D _bumpTexture;

    Graphics::PRenderTarget _renderTarget;
    Graphics::PVertexBuffer _fullscreenQuadVertexBuffer;

    Graphics::PShaderEffect _postProcessEffect;
    Graphics::PConstantBuffer _postProcessParams;

    Graphics::PTexture2D _screenDuDvTexture;

    float3 _sunLightDirection;
    ColorRGBA _sunLightColor;

    bool _wireframe = false;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
